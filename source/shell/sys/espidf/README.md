# vsf_espidf — ESP-IDF compatibility sub-system for VSF

A sibling of `shell/sys/linux/` (vsf_linux). Provides an ESP-IDF-shaped
public API on top of VSF kernel / service / component / HAL layers so that
applications written against ESP-IDF can be compiled and run on VSF-hosted
targets.

## Scope

This sub-system does **not** embed any ESP-IDF source code. All public
headers under `include/` are clean-room re-implementations of the
ESP-IDF public API. Implementations live under `port/` and are thin
bridges onto existing VSF facilities.

## Layout

```
espidf/
├── CMakeLists.txt             # sub-system build entry, gated by VSF_USE_ESPIDF
├── vsf_espidf.h               # internal sub-system header
├── vsf_espidf_cfg.h           # feature switches + IDF baseline version
├── vsf_espidf.c               # sub-system init + per-module init chain
├── include/                   # clean-room ESP-IDF public headers
│   ├── README.md
│   ├── driver/                # driver/*.h
│   ├── freertos/              # FreeRTOS.h etc. (forwarders)
│   └── hal/                   # hal/*_types.h
└── port/                      # ESP-IDF-API -> VSF-internal glue
    └── CMakeLists.txt
```

## Backend mapping (implementation guide)

| ESP-IDF API | VSF backend |
|---|---|
| `esp_log.h` (ESP_LOGx) | `service/trace` + `vsf_trace` |
| `esp_err.h` / `esp_check.h` | header-only |
| `esp_event.h` | `kernel/vsf_eda` |
| `esp_timer.h` | `kernel/vsf_timq` + `vsf_callback_timer` |
| `esp_system.h` (restart/reset_reason/random) | VSF reset + entropy |
| xRingbuffer (`esp_ringbuf.h`) | `service/fifo`, `service/pbuf`, `service/simple_stream` |
| `esp_heap_caps.h` | `service/heap` + `service/pool` (caps collapsed) |
| `esp_partition.h` | `component/mal` (`vsf_mal`) |
| `nvs_flash.h` / `nvs.h` | KV layered on partition or littlefs; to be written |
| `esp_vfs.h` + POSIX I/O | `component/fs` + `shell/sys/linux` VFS |
| FatFs / LittleFS / RomFS | `component/fs/driver/{fatfs,littlefs,romfs}` |
| lwIP + BSD sockets | `component/3rd-party/lwip` + `component/tcpip/socket` |
| mbedtls | `component/3rd-party/mbedtls` |
| `esp_http_client.h` / `esp_http_server.h` | `component/tcpip/protocol/http` |
| cJSON / esp_json | `service/json` |
| pthread | `shell/sys/linux` pthread |
| FreeRTOS FreeRTOS.h, queue, event_groups | VSF FreeRTOS compat layer |
| `esp_netif.h` | `component/tcpip/netdrv` + lwIP glue |
| `driver/gpio|uart|i2c|spi_master|ledc|gptimer|adc` | `hal/driver/common/template/vsf_template_*` |
| `esp_wifi.h` | external Wi-Fi module semantic shim (stage C) |
| `esp_bt.h` | external HCI via VSF BTHCI (stage C) |
| `mqtt_client.h` | upstream Paho Embedded C (not ESP-MQTT) |

## Enablement

Master switch `VSF_USE_ESPIDF` plus per-component `VSF_ESPIDF_CFG_USE_*`
switches in `vsf_espidf_cfg.h`. All disabled by default; opt in per module
as it becomes implemented.

## Roadmap

Stage 1 (pure-software core, no HW dependency):
`esp_err` → `esp_log` → `esp_event` → `esp_timer` → `esp_system` →
`esp_ringbuf` → `esp_heap_caps`.

Stage 2 (storage & fs): `esp_partition` → `nvs_flash` → `esp_vfs`.

Stage 3 (network): `esp_netif` → `esp_http_client` / `esp_http_server` →
(optional) MQTT via upstream Paho.

Stage 4 (peripheral drivers): `driver/gpio` → `driver/uart` →
`driver/i2c` → `driver/spi_master` → `driver/ledc` → `driver/gptimer` →
`driver/adc`.

Stage 5 (radio, optional): external Wi-Fi module `esp_wifi` semantic shim.

## Validation

Each stage validated by compiling and running matching
`esp-idf/examples/` trees built against this sub-system's headers
(no ESP-IDF source in the build).
