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

// Clean-room FreeRTOS task.h shim for VSF.
//
// Scope of the initial MVP:
//  - vTaskDelay / vTaskDelayUntil      (cooperative sleep via vsf_thread)
//  - xTaskGetTickCount                 (vsf_systimer_get_ms)
//  - taskYIELD                         (vsf_thread_yield)
//  - xTaskCreate / xTaskCreateStatic   (spawn a vsf_thread worker)
//  - vTaskDelete                       (self-termination / best-effort)
//
// Suspend/Resume/Notify and the full task API surface are deferred; they
// will be layered on top of the same vsf_thread worker model.

#ifndef __VSF_FREERTOS_TASK_H__
#define __VSF_FREERTOS_TASK_H__

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ TYPES =========================================*/

typedef void *                  TaskHandle_t;
typedef void (*TaskFunction_t)(void *);

// Static-create counterpart. Applications declare one of these to reserve
// space for the task control block; the shim repurposes the storage as
// its own internal control block.
typedef struct {
    uint8_t opaque[64];
} StaticTask_t;

/*============================ API ===========================================*/

// Delay the calling task by xTicksToDelay ticks. In this shim, 1 tick equals
// 1 ms (see pdMS_TO_TICKS); the underlying sleep uses vsf_thread_delay.
// MUST be called from a vsf_thread context.
extern void vTaskDelay(const TickType_t xTicksToDelay);

// Absolute-deadline variant. pxPreviousWakeTime is updated on return.
extern void vTaskDelayUntil(TickType_t * const pxPreviousWakeTime,
                            const TickType_t xTimeIncrement);

// Returns the current kernel tick count. Mapped to vsf_systimer milliseconds
// to keep pdMS_TO_TICKS(ms) semantics consistent.
extern TickType_t xTaskGetTickCount(void);
extern TickType_t xTaskGetTickCountFromISR(void);

// Yield the CPU to any equal-priority ready task.
#define taskYIELD()             vTaskYield()
extern void vTaskYield(void);

// Creates a FreeRTOS task backed by a vsf_thread worker. pvParameters is
// passed to pxTaskCode. usStackDepth is interpreted as bytes (see StackType_t
// typedef). uxPriority is mapped to VSF_FREERTOS_CFG_DEFAULT_VSF_PRIO in the
// MVP; board overrides can refine the mapping.
// Returns pdPASS on success, pdFAIL on error.
extern BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,
                              const char * const pcName,
                              const uint32_t usStackDepth,
                              void * const pvParameters,
                              UBaseType_t uxPriority,
                              TaskHandle_t * const pxCreatedTask);

// Deletes a task. xTaskToDelete == NULL means self-delete.
// NOTE: self-delete from inside the task body is the only supported form in
// the MVP; deleting another task is best-effort and may leak the stack.
extern void vTaskDelete(TaskHandle_t xTaskToDelete);

// Returns the handle of the calling task, or NULL if called from outside a
// vsf_thread context.
extern TaskHandle_t xTaskGetCurrentTaskHandle(void);

#ifdef __cplusplus
}
#endif

#endif      // __VSF_FREERTOS_TASK_H__
