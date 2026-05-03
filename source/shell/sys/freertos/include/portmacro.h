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

// Clean-room FreeRTOS portmacro.h shim for VSF.
//
// Only the primitive types FreeRTOS applications rely on are exposed here.
// Architecture-specific primitives (portYIELD_FROM_ISR, portENTER_CRITICAL,
// etc.) are mapped to VSF kernel operations where a sensible equivalent
// exists; the rest become no-ops so that code written against FreeRTOS
// compiles but the semantics are explicit in this header.

#ifndef __VSF_FREERTOS_PORTMACRO_H__
#define __VSF_FREERTOS_PORTMACRO_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ TYPES =========================================*/

typedef long                    BaseType_t;
typedef unsigned long           UBaseType_t;

// A tick is a systimer tick in the VSF shim. 32-bit matches FreeRTOS with
// configUSE_16_BIT_TICKS == 0.
typedef uint32_t                TickType_t;
#ifndef portMAX_DELAY
#   define portMAX_DELAY        ((TickType_t)0xFFFFFFFFUL)
#endif

// StackType_t is documented as "the type used by the stack". FreeRTOS
// applications pass stack budgets in units of StackType_t. We define it as
// a byte so budgets are expressed in bytes, which matches how the VSF
// thread API actually consumes them.
typedef uint8_t                 StackType_t;

/*============================ MACROS ========================================*/

#ifndef pdFALSE
#   define pdFALSE              ((BaseType_t)0)
#endif
#ifndef pdTRUE
#   define pdTRUE               ((BaseType_t)1)
#endif
#ifndef pdPASS
#   define pdPASS               pdTRUE
#endif
#ifndef pdFAIL
#   define pdFAIL               pdFALSE
#endif

// VSF uses cooperative scheduling inside a priority queue; from the caller
// point of view there is no "critical section" separate from interrupt
// masking. These are no-ops so that code compiles; users that actually need
// IRQ-safe sections should call vsf_protect_*() directly.
#define portENTER_CRITICAL()            ((void)0)
#define portEXIT_CRITICAL()             ((void)0)
#define portENTER_CRITICAL_ISR()        ((void)0)
#define portEXIT_CRITICAL_ISR()         ((void)0)
#define portDISABLE_INTERRUPTS()        ((void)0)
#define portENABLE_INTERRUPTS()         ((void)0)

// Stack budget unit. FreeRTOS defines configMINIMAL_STACK_SIZE in units of
// StackType_t. In this shim one unit == one byte.
#define portSTACK_TYPE                  StackType_t
#define portBASE_TYPE                   BaseType_t

// portTICK_PERIOD_MS is the number of milliseconds per kernel tick. The VSF
// shim maps FreeRTOS ticks 1:1 to milliseconds so that pdMS_TO_TICKS is the
// identity. The underlying sleep still uses systimer ticks internally.
#ifndef portTICK_PERIOD_MS
#   define portTICK_PERIOD_MS           ((TickType_t)1)
#endif
#ifndef portTICK_RATE_MS
#   define portTICK_RATE_MS             portTICK_PERIOD_MS
#endif

// Placeholder: real ISR yield must be deferred to the scheduler on the
// caller side. Documented as a best-effort no-op.
#define portYIELD_FROM_ISR(x)           ((void)(x))

#ifdef __cplusplus
}
#endif

#endif      // __VSF_FREERTOS_PORTMACRO_H__
