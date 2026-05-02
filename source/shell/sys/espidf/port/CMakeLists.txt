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
)

# vsf_add_sources(
#     esp_log_port.c         # -> service/trace (vsf_trace)
#     esp_event_port.c       # -> kernel/vsf_eda
#     esp_timer_port.c       # -> kernel/vsf_timq + vsf_callback_timer
#     esp_system_port.c      # -> reset / reset_reason / esp_random
#     esp_ringbuf_port.c     # -> service/fifo, service/pbuf
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
