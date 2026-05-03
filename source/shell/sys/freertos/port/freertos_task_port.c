/*****************************************************************************
 *   Copyright(C)2009-2026 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*
 * Port implementation for "task.h" on VSF.
 *
 * FreeRTOS tasks are backed by vsf_thread workers:
 *  - xTaskCreate spawns a vsf_thread with a heap-allocated stack and a
 *    trampoline that invokes the user TaskFunction_t.
 *  - vTaskDelay routes to vsf_thread_delay, mapping 1 tick == 1 ms (see
 *    pdMS_TO_TICKS in FreeRTOS.h).
 *  - taskYIELD is vsf_thread_yield.
 *  - xTaskGetTickCount returns the systimer reading in milliseconds.
 *  - vTaskDelete(NULL) performs a clean self-exit via vsf_thread_exit.
 *
 * Priority handling: the MVP collapses any FreeRTOS priority to
 * VSF_FREERTOS_CFG_DEFAULT_VSF_PRIO. A richer mapping can be introduced
 * without breaking callers.
 */

/*============================ INCLUDES ======================================*/

#include "../vsf_freertos_cfg.h"

#if VSF_USE_FREERTOS == ENABLED && VSF_FREERTOS_CFG_USE_TASK == ENABLED

#include "FreeRTOS.h"
#include "task.h"

#include "../vsf_freertos.h"
#include "kernel/vsf_kernel.h"

#if defined(VSF_USE_HEAP) && VSF_USE_HEAP == ENABLED
#   include "service/heap/vsf_heap.h"
#else
#   error "VSF_FREERTOS_CFG_USE_TASK requires VSF_USE_HEAP"
#endif

#include <string.h>
#include <stddef.h>

/*============================ TYPES =========================================*/

typedef struct __freertos_task_t {
    vsf_thread_t        thread;
#if VSF_KERNEL_CFG_EDA_SUPPORT_SUB_CALL == ENABLED
    vsf_thread_cb_t     thread_cb;
#endif
    TaskFunction_t      entry;
    void *              arg;
    void *              stack;
    uint32_t            stack_bytes;
} __freertos_task_t;

/*============================ IMPLEMENTATION ================================*/

#if VSF_KERNEL_CFG_EDA_SUPPORT_SUB_CALL == ENABLED
static void __freertos_task_wrapper(vsf_thread_cb_t *cb)
{
    __freertos_task_t *ft = (__freertos_task_t *)
        ((uint8_t *)cb - offsetof(__freertos_task_t, thread_cb));
    if (ft->entry != NULL) {
        ft->entry(ft->arg);
    }
    // FreeRTOS contract forbids returning from a task body; if it happens,
    // terminate the vsf_thread cleanly. The __freertos_task_t backing
    // storage is intentionally leaked in the MVP (self-cleanup would race
    // with the scheduler that is still referencing our fields).
    vsf_thread_exit();
}
#else
static void __freertos_task_wrapper(vsf_thread_t *t)
{
    __freertos_task_t *ft = (__freertos_task_t *)
        ((uint8_t *)t - offsetof(__freertos_task_t, thread));
    if (ft->entry != NULL) {
        ft->entry(ft->arg);
    }
    vsf_thread_exit();
}
#endif

void vTaskDelay(const TickType_t xTicksToDelay)
{
    // pdMS_TO_TICKS(ms) == ms in this shim, so the incoming tick is in ms.
    vsf_thread_delay(vsf_systimer_ms_to_tick((uint_fast32_t)xTicksToDelay));
}

void vTaskDelayUntil(TickType_t * const pxPreviousWakeTime,
                     const TickType_t xTimeIncrement)
{
    if (pxPreviousWakeTime == NULL) {
        return;
    }
    TickType_t now  = xTaskGetTickCount();
    TickType_t next = *pxPreviousWakeTime + xTimeIncrement;
    *pxPreviousWakeTime = next;
    // Signed compare handles tick counter wrap.
    int32_t delta = (int32_t)(next - now);
    if (delta > 0) {
        vsf_thread_delay(vsf_systimer_ms_to_tick((uint_fast32_t)delta));
    }
}

TickType_t xTaskGetTickCount(void)
{
    return (TickType_t)vsf_systimer_tick_to_ms(vsf_systimer_get_tick());
}

TickType_t xTaskGetTickCountFromISR(void)
{
    return xTaskGetTickCount();
}

void vTaskYield(void)
{
    vsf_thread_yield();
}

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,
                       const char * const pcName,
                       const uint32_t usStackDepth,
                       void * const pvParameters,
                       UBaseType_t uxPriority,
                       TaskHandle_t * const pxCreatedTask)
{
    (void)pcName;
    (void)uxPriority;

    if (pxTaskCode == NULL) {
        if (pxCreatedTask != NULL) { *pxCreatedTask = NULL; }
        return pdFAIL;
    }

    uint32_t stack_bytes = usStackDepth;
    if (stack_bytes < VSF_FREERTOS_CFG_MIN_STACK_BYTES) {
        stack_bytes = VSF_FREERTOS_CFG_MIN_STACK_BYTES;
    }
    // vsf_thread_start requires stack_size >= PAGE_SIZE + GUARDIAN_SIZE
    // and aligned to PAGE_SIZE. Grow / align to satisfy both.
    {
        uint32_t page = (uint32_t)VSF_KERNEL_CFG_THREAD_STACK_PAGE_SIZE;
        uint32_t guard = (uint32_t)VSF_KERNEL_CFG_THREAD_STACK_GUARDIAN_SIZE;
        uint32_t min_bytes = page + guard;
        if (stack_bytes < min_bytes) {
            stack_bytes = min_bytes;
        }
        // round up to page boundary
        stack_bytes = (stack_bytes + (page - 1u)) & ~(page - 1u);
    }
    // 8-byte align as required by vsf_thread (stack is uint64_t[]-backed).
    stack_bytes = (stack_bytes + 7u) & ~7u;

    __freertos_task_t *ft =
        (__freertos_task_t *)vsf_heap_malloc(sizeof(*ft));
    if (ft == NULL) {
        if (pxCreatedTask != NULL) { *pxCreatedTask = NULL; }
        return pdFAIL;
    }
    void *stack = vsf_heap_malloc_aligned(stack_bytes, 8);
    if (stack == NULL) {
        vsf_heap_free(ft);
        if (pxCreatedTask != NULL) { *pxCreatedTask = NULL; }
        return pdFAIL;
    }
    memset(ft, 0, sizeof(*ft));
    ft->entry       = pxTaskCode;
    ft->arg         = pvParameters;
    ft->stack       = stack;
    ft->stack_bytes = stack_bytes;

#if VSF_KERNEL_CFG_EDA_SUPPORT_SUB_CALL == ENABLED
    ft->thread_cb.entry      = (vsf_thread_entry_t *)__freertos_task_wrapper;
    ft->thread_cb.stack      = stack;
    ft->thread_cb.stack_size = stack_bytes;
    vsf_err_t err = vsf_thread_start(&ft->thread, &ft->thread_cb,
                                     VSF_FREERTOS_CFG_DEFAULT_VSF_PRIO);
#else
    ft->thread.entry      = (vsf_thread_entry_t *)__freertos_task_wrapper;
    ft->thread.stack      = stack;
    ft->thread.stack_size = stack_bytes;
    vsf_err_t err = vsf_thread_start(&ft->thread,
                                     VSF_FREERTOS_CFG_DEFAULT_VSF_PRIO);
#endif
    if (err != VSF_ERR_NONE) {
        vsf_heap_free(stack);
        vsf_heap_free(ft);
        if (pxCreatedTask != NULL) { *pxCreatedTask = NULL; }
        return pdFAIL;
    }

    if (pxCreatedTask != NULL) { *pxCreatedTask = (TaskHandle_t)ft; }
    return pdPASS;
}

void vTaskDelete(TaskHandle_t xTaskToDelete)
{
    if (xTaskToDelete == NULL) {
        // Self-delete: terminate the vsf_thread cleanly. Backing storage
        // is leaked in MVP; caller-owned cleanup would race the scheduler.
        vsf_thread_exit();
        // unreachable
    }
    // Deleting another task is not safely supported in the MVP.
    (void)xTaskToDelete;
}

TaskHandle_t xTaskGetCurrentTaskHandle(void)
{
    // The opaque handle we expose to FreeRTOS callers is the vsf_thread_t *
    // of the calling task. Callers typically only use it for NULL checks or
    // to round-trip through other FreeRTOS APIs.
    return (TaskHandle_t)vsf_thread_get_cur();
}

#endif      // VSF_USE_FREERTOS && VSF_FREERTOS_CFG_USE_TASK
