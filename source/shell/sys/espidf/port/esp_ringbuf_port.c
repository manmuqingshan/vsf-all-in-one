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
 * Port implementation for "esp_ringbuf.h" on VSF.
 *
 * Initial drop: fully supports RINGBUF_TYPE_BYTEBUF. For NOSPLIT /
 * ALLOWSPLIT the ring buffer is created successfully but is treated as
 * a byte buffer; item framing (length prefix on each message) is a
 * planned follow-up.
 *
 * Concurrency: the VSF ESP-IDF sub-system currently has no SMP target
 * and on the Windows host the primary use case is single-threaded tests.
 * Access is guarded by vsf_protect(int)/vsf_unprotect to support calls
 * from timer callbacks. Blocking on empty/full is NOT modeled: the
 * ticks_to_wait parameter is accepted for API compatibility but waits
 * are polled immediately.
 */

/*============================ INCLUDES ======================================*/

#include "../vsf_espidf_cfg.h"

#if VSF_USE_ESPIDF == ENABLED && VSF_ESPIDF_CFG_USE_RINGBUF == ENABLED

#include "esp_ringbuf.h"

#include "../vsf_espidf.h"
#include "kernel/vsf_kernel.h"
#include "service/trace/vsf_trace.h"
#if defined(VSF_USE_HEAP) && VSF_USE_HEAP == ENABLED
#   include "service/heap/vsf_heap.h"
#else
#   error "VSF_ESPIDF_CFG_USE_RINGBUF requires VSF_USE_HEAP"
#endif

#include <string.h>

/*============================ TYPES =========================================*/

struct __vsf_espidf_ringbuf {
    uint8_t *           buf;
    size_t              capacity;
    size_t              head;   /*!< write cursor                             */
    size_t              tail;   /*!< read cursor                              */
    size_t              count;  /*!< bytes currently stored                   */
    RingbufferType_t    type;
};

/* Per-receive envelope: we allocate a contiguous copy for the caller and
 * include a small header so vRingbufferReturnItem() can free it safely. */
typedef struct {
    uint32_t            magic;
    size_t              size;
    /* payload[] follows */
} __vsf_espidf_rx_item_t;

#define __VSF_ESPIDF_RX_MAGIC       0x72627831u     // 'rbx1'

/*============================ IMPLEMENTATION ================================*/

RingbufHandle_t xRingbufferCreate(size_t buffer_size, RingbufferType_t type)
{
    if (buffer_size == 0) {
        return NULL;
    }
    RingbufHandle_t rb = (RingbufHandle_t)vsf_heap_malloc(sizeof(*rb));
    if (rb == NULL) {
        return NULL;
    }
    rb->buf = (uint8_t *)vsf_heap_malloc(buffer_size);
    if (rb->buf == NULL) {
        vsf_heap_free(rb);
        return NULL;
    }
    rb->capacity = buffer_size;
    rb->head = rb->tail = rb->count = 0;
    rb->type = type;
    return rb;
}

void vRingbufferDelete(RingbufHandle_t handle)
{
    if (handle == NULL) {
        return;
    }
    if (handle->buf != NULL) {
        vsf_heap_free(handle->buf);
    }
    vsf_heap_free(handle);
}

BaseType_t xRingbufferSend(RingbufHandle_t handle, const void *data,
                            size_t data_size, TickType_t ticks_to_wait)
{
    (void)ticks_to_wait;        // poll-only on host port

    if ((handle == NULL) || (data == NULL) || (data_size == 0)) {
        return pdFALSE;
    }

    vsf_protect_t orig = vsf_protect_int();
    if ((handle->capacity - handle->count) < data_size) {
        vsf_unprotect_int(orig);
        return pdFALSE;
    }
    const uint8_t *src = (const uint8_t *)data;
    size_t first = handle->capacity - handle->head;
    if (first >= data_size) {
        memcpy(handle->buf + handle->head, src, data_size);
        handle->head += data_size;
        if (handle->head == handle->capacity) {
            handle->head = 0;
        }
    } else {
        memcpy(handle->buf + handle->head, src, first);
        memcpy(handle->buf, src + first, data_size - first);
        handle->head = data_size - first;
    }
    handle->count += data_size;
    vsf_unprotect_int(orig);
    return pdTRUE;
}

static void * __vsf_espidf_rb_receive_core(RingbufHandle_t handle,
                                            size_t *item_size, size_t cap)
{
    if ((handle == NULL) || (item_size == NULL) || (cap == 0)) {
        return NULL;
    }
    vsf_protect_t orig = vsf_protect_int();
    size_t avail = handle->count;
    if (avail == 0) {
        vsf_unprotect_int(orig);
        return NULL;
    }
    size_t take = (avail < cap) ? avail : cap;

    __vsf_espidf_rx_item_t *item =
        (__vsf_espidf_rx_item_t *)vsf_heap_malloc(sizeof(*item) + take);
    if (item == NULL) {
        vsf_unprotect_int(orig);
        return NULL;
    }
    item->magic = __VSF_ESPIDF_RX_MAGIC;
    item->size  = take;

    uint8_t *dst = (uint8_t *)(item + 1);
    size_t first = handle->capacity - handle->tail;
    if (first >= take) {
        memcpy(dst, handle->buf + handle->tail, take);
        handle->tail += take;
        if (handle->tail == handle->capacity) {
            handle->tail = 0;
        }
    } else {
        memcpy(dst, handle->buf + handle->tail, first);
        memcpy(dst + first, handle->buf, take - first);
        handle->tail = take - first;
    }
    handle->count -= take;
    vsf_unprotect_int(orig);

    *item_size = take;
    return dst;
}

void * xRingbufferReceive(RingbufHandle_t handle, size_t *item_size,
                            TickType_t ticks_to_wait)
{
    (void)ticks_to_wait;
    if (handle == NULL) {
        return NULL;
    }
    return __vsf_espidf_rb_receive_core(handle, item_size, handle->capacity);
}

void * xRingbufferReceiveUpTo(RingbufHandle_t handle, size_t *item_size,
                                TickType_t ticks_to_wait, size_t wanted_size)
{
    (void)ticks_to_wait;
    return __vsf_espidf_rb_receive_core(handle, item_size, wanted_size);
}

void vRingbufferReturnItem(RingbufHandle_t handle, void *item)
{
    (void)handle;
    if (item == NULL) {
        return;
    }
    __vsf_espidf_rx_item_t *hdr =
        (__vsf_espidf_rx_item_t *)((uint8_t *)item - sizeof(*hdr));
    if (hdr->magic != __VSF_ESPIDF_RX_MAGIC) {
        // Do not free foreign pointers. Log once through trace so caller
        // can notice the misuse in development.
        vsf_trace_error("vRingbufferReturnItem: bad magic on %p"
                        VSF_TRACE_CFG_LINEEND, item);
        return;
    }
    hdr->magic = 0;     // poison
    vsf_heap_free(hdr);
}

size_t xRingbufferGetCurFreeSize(RingbufHandle_t handle)
{
    if (handle == NULL) {
        return 0;
    }
    vsf_protect_t orig = vsf_protect_int();
    size_t free = handle->capacity - handle->count;
    vsf_unprotect_int(orig);
    return free;
}

size_t xRingbufferGetCurFilledSize(RingbufHandle_t handle)
{
    if (handle == NULL) {
        return 0;
    }
    vsf_protect_t orig = vsf_protect_int();
    size_t used = handle->count;
    vsf_unprotect_int(orig);
    return used;
}

size_t xRingbufferGetMaxItemSize(RingbufHandle_t handle)
{
    return (handle != NULL) ? handle->capacity : 0;
}

#endif      // VSF_USE_ESPIDF && VSF_ESPIDF_CFG_USE_RINGBUF
