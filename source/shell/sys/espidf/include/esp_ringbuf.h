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
 * Clean-room re-implementation of ESP-IDF public API "freertos/ringbuf.h".
 *
 * Authored from the ESP-IDF v5.x public API reference only. No ESP-IDF /
 * FreeRTOS source code or data is copied.
 *
 * This initial drop supports RINGBUF_TYPE_BYTEBUF end-to-end. The split /
 * no-split item-oriented modes are accepted by xRingbufferCreate() but
 * currently fall back to byte-buffer behavior; their item framing contract
 * will be honored in a follow-up as tests are added.
 *
 * Because VSF's ESP-IDF port does not include a full FreeRTOS layer, this
 * header exposes a minimal subset of the FreeRTOS value types that the
 * ring buffer API signature references. The guards allow code that does
 * include real FreeRTOS headers to compile side-by-side.
 */

#ifndef __VSF_ESPIDF_ESP_RINGBUF_H__
#define __VSF_ESPIDF_ESP_RINGBUF_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ TYPES =========================================*/

#ifndef __VSF_ESPIDF_FREERTOS_TYPES_DEFINED__
#define __VSF_ESPIDF_FREERTOS_TYPES_DEFINED__
typedef long            BaseType_t;     /*!< FreeRTOS portable signed word.   */
typedef uint32_t        TickType_t;     /*!< FreeRTOS tick counter type.      */
#   ifndef pdTRUE
#       define pdTRUE       ((BaseType_t)1)
#   endif
#   ifndef pdFALSE
#       define pdFALSE      ((BaseType_t)0)
#   endif
#   ifndef pdPASS
#       define pdPASS       pdTRUE
#   endif
#   ifndef pdFAIL
#       define pdFAIL       pdFALSE
#   endif
#   ifndef portMAX_DELAY
#       define portMAX_DELAY    ((TickType_t)0xFFFFFFFFu)
#   endif
#endif      // __VSF_ESPIDF_FREERTOS_TYPES_DEFINED__

typedef enum {
    RINGBUF_TYPE_NOSPLIT    = 0,    /*!< Item-oriented, no split at wrap     */
    RINGBUF_TYPE_ALLOWSPLIT = 1,    /*!< Item-oriented, may wrap             */
    RINGBUF_TYPE_BYTEBUF    = 2,    /*!< Pure byte stream                    */
} RingbufferType_t;

struct __vsf_espidf_ringbuf;
typedef struct __vsf_espidf_ringbuf * RingbufHandle_t;

/*============================ PROTOTYPES ====================================*/

/* Allocate a ring buffer of the requested byte capacity. Returns NULL on
 * allocation failure. The returned handle must be released via
 * vRingbufferDelete(). */
RingbufHandle_t xRingbufferCreate(size_t buffer_size, RingbufferType_t type);

/* Release a previously created ring buffer. Passing NULL is a no-op. */
void vRingbufferDelete(RingbufHandle_t handle);

/* Enqueue `data_size` bytes. Returns pdTRUE on success, pdFALSE if there
 * is not enough free space. The VSF port's timeout semantics are poll-only
 * on the host (no task blocking); any non-zero ticks value is treated as
 * a single in-place retry. */
BaseType_t xRingbufferSend(RingbufHandle_t handle, const void *data,
                            size_t data_size, TickType_t ticks_to_wait);

/* Pull up to `max_size` bytes from the buffer, copying internally so the
 * caller does not need to invoke vRingbufferReturnItem(). Returns NULL if
 * nothing is available. On success sets *item_size to the number of bytes
 * copied. The returned pointer must be released via vPortFree()-compatible
 * means; in this port it is allocated from the VSF heap and must be freed
 * with vRingbufferReturnItem(). */
void * xRingbufferReceive(RingbufHandle_t handle, size_t *item_size,
                            TickType_t ticks_to_wait);

/* Same as xRingbufferReceive() but caps the number of bytes returned. */
void * xRingbufferReceiveUpTo(RingbufHandle_t handle, size_t *item_size,
                                TickType_t ticks_to_wait, size_t wanted_size);

/* Release a buffer previously returned by xRingbufferReceive(). */
void vRingbufferReturnItem(RingbufHandle_t handle, void *item);

/* Bytes currently free for writing. */
size_t xRingbufferGetCurFreeSize(RingbufHandle_t handle);

/* Bytes currently queued for reading. */
size_t xRingbufferGetCurFilledSize(RingbufHandle_t handle);

/* Total buffer capacity in bytes as passed to xRingbufferCreate(). */
size_t xRingbufferGetMaxItemSize(RingbufHandle_t handle);

#ifdef __cplusplus
}
#endif

#endif      // __VSF_ESPIDF_ESP_RINGBUF_H__
