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

// Clean-room FreeRTOS umbrella header shim for VSF.
//
// FreeRTOS applications include "FreeRTOS.h" before any other FreeRTOS
// header. We expose the primitive types and configuration macros here so
// that task.h / queue.h / semphr.h / timers.h / event_groups.h can be
// included in any order.

#ifndef __VSF_FREERTOS_H__
#define __VSF_FREERTOS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "portmacro.h"
#include "portable.h"

/*============================ CONFIG ========================================*/

// Minimal stack budget. FreeRTOS calls that auto-allocate a stack use
// multiples of this. In this shim StackType_t == uint8_t, so the unit is
// byte. Users may override in project config.
#ifndef configMINIMAL_STACK_SIZE
#   define configMINIMAL_STACK_SIZE     ((uint16_t)1024)
#endif

// Number of FreeRTOS task priorities available. Mapping to VSF priorities
// is flat by default (see freertos_task_port.c).
#ifndef configMAX_PRIORITIES
#   define configMAX_PRIORITIES         (25)
#endif

// TickType defaults mirror portmacro.h.
#ifndef configTICK_RATE_HZ
#   define configTICK_RATE_HZ           (1000)
#endif

#ifndef configUSE_16_BIT_TICKS
#   define configUSE_16_BIT_TICKS       0
#endif

// Tick->ms conversion. In this shim 1 tick == 1 ms so pdMS_TO_TICKS is the
// identity; the underlying sleep still uses systimer ticks via vsf_thread.
#define pdMS_TO_TICKS(xTimeInMs)        ((TickType_t)(xTimeInMs))
#define pdTICKS_TO_MS(xTicks)           ((uint32_t)(xTicks))

/*============================ ASSERT ========================================*/

#ifndef configASSERT
#   define configASSERT(x)              ((void)(x))
#endif

#ifdef __cplusplus
}
#endif

#endif      // __VSF_FREERTOS_H__
