# CMakeLists head
#
# port/ - bridge layer between FreeRTOS-shaped public API (include/) and VSF
# kernel primitives (vsf_thread, vsf_eda, vsf_callback_timer, vsf_heap).
#
# Every port file is guarded internally by its component's
# VSF_FREERTOS_CFG_USE_* switch, so listing a file here is safe even before
# the component's switch is flipped on.

vsf_add_sources(
    freertos_task_port.c       # -> vsf_thread
    freertos_heap_port.c       # -> vsf_heap (pvPortMalloc / vPortFree / xPortGetFreeHeapSize)
    freertos_queue_port.c      # -> vsf_eda_queue (xQueueSend / xQueueReceive / ...)
)

# vsf_add_sources(
#     freertos_semphr_port.c        # -> vsf_eda_sync / mutex
#     freertos_timers_port.c        # -> vsf_callback_timer
#     freertos_event_groups_port.c  # -> vsf_eda_bmpevt
# )
