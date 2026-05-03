# CMakeLists head
#
# port/ - bridge layer between ESP-IDF-shaped public API (include/) and VSF
# internal services (kernel/eda, service/trace, service/fifo, component/mal,
# component/fs, component/tcpip, hal/driver/*, etc.).
#
# Source files are added as each component lands. Every port file is guarded
# internally by its component's VSF_ESPIDF_CFG_USE_* switch, so listing a file
# here is safe even before the component's switch is enabled.

vsf_add_sources(
    esp_err_port.c         # -> pure header + 2 diagnostic helpers
    esp_log_port.c         # -> service/trace (vsf_trace)
    esp_timer_port.c       # -> vsf_callback_timer
    esp_system_port.c      # -> vsf_arch_reset / vsf_heap / PRNG
    esp_ringbuf_port.c     # -> byte ring buffer on vsf_heap
    esp_heap_caps_port.c   # -> vsf_heap (caps collapsed, statistics bridged)
)

# vsf_add_sources(
#     esp_event_port.c       # -> kernel/vsf_eda
#     esp_partition_port.c   # -> component/mal
#     nvs_flash_port.c       # -> partition + littlefs-backed KV
#     esp_netif_port.c       # -> component/tcpip/netdrv + lwip
#     esp_http_client_port.c # -> component/tcpip/protocol/http
#     driver_gpio_port.c     # -> hal/driver/common/template/vsf_template_gpio.h
#     driver_uart_port.c     # -> vsf_template_usart.h
#     driver_i2c_port.c      # -> vsf_template_i2c.h
#     driver_spi_master_port.c # -> vsf_template_spi.h
#     driver_ledc_port.c     # -> vsf_template_pwm.h
#     driver_gptimer_port.c  # -> vsf_template_timer.h
#     driver_adc_port.c      # -> vsf_template_adc.h
# )
