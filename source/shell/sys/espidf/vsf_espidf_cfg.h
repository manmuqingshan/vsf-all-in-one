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

/*============================ INCLUDES ======================================*/

//! \note do not move this pre-processor statement to other places
#include "../../../vsf_cfg.h"

#ifndef __VSF_ESPIDF_CFG_H__
#define __VSF_ESPIDF_CFG_H__

#include "shell/vsf_shell_wrapper.h"

/*============================ MACROS ========================================*/

// Master switch. User enables this in vsf_usr_cfg.h / board cfg.
#ifndef VSF_USE_ESPIDF
#   define VSF_USE_ESPIDF                       DISABLED
#endif

#ifndef VSF_ESPIDF_ASSERT
#   define VSF_ESPIDF_ASSERT                    VSF_ASSERT
#endif

#define VSF_ESPIDF_WRAPPER(__api)               VSF_SHELL_WRAPPER(vsf_espidf, __api)

// ---------------------------------------------------------------------------
// ESP-IDF API baseline version this sub-system targets.
// Align with a specific IDF release so that header shapes are deterministic.
// Default baseline: v5.x LTS. Adjust when a new baseline is chosen.
// ---------------------------------------------------------------------------
#ifndef VSF_ESPIDF_CFG_VERSION_MAJOR
#   define VSF_ESPIDF_CFG_VERSION_MAJOR         5
#endif
#ifndef VSF_ESPIDF_CFG_VERSION_MINOR
#   define VSF_ESPIDF_CFG_VERSION_MINOR         1
#endif
#ifndef VSF_ESPIDF_CFG_VERSION_PATCH
#   define VSF_ESPIDF_CFG_VERSION_PATCH         0
#endif

// ---------------------------------------------------------------------------
// Per-component enable switches. All OFF by default; opt-in as each module
// is implemented.
// ---------------------------------------------------------------------------

// Core infrastructure
#ifndef VSF_ESPIDF_CFG_USE_LOG
#   define VSF_ESPIDF_CFG_USE_LOG               DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_ERR
#   define VSF_ESPIDF_CFG_USE_ERR               ENABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_EVENT
#   define VSF_ESPIDF_CFG_USE_EVENT             DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_TIMER
#   define VSF_ESPIDF_CFG_USE_TIMER             DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_SYSTEM
#   define VSF_ESPIDF_CFG_USE_SYSTEM            DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_RINGBUF
#   define VSF_ESPIDF_CFG_USE_RINGBUF           DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_HEAP_CAPS
#   define VSF_ESPIDF_CFG_USE_HEAP_CAPS         DISABLED
#endif

// Storage
#ifndef VSF_ESPIDF_CFG_USE_PARTITION
#   define VSF_ESPIDF_CFG_USE_PARTITION         DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_NVS
#   define VSF_ESPIDF_CFG_USE_NVS               DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_VFS
#   define VSF_ESPIDF_CFG_USE_VFS               DISABLED
#endif

// Network
#ifndef VSF_ESPIDF_CFG_USE_NETIF
#   define VSF_ESPIDF_CFG_USE_NETIF             DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_HTTP_CLIENT
#   define VSF_ESPIDF_CFG_USE_HTTP_CLIENT       DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_HTTP_SERVER
#   define VSF_ESPIDF_CFG_USE_HTTP_SERVER       DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_WIFI
#   define VSF_ESPIDF_CFG_USE_WIFI              DISABLED
#endif

// Peripheral drivers (bridged to vsf_template_*)
#ifndef VSF_ESPIDF_CFG_USE_DRIVER_GPIO
#   define VSF_ESPIDF_CFG_USE_DRIVER_GPIO       DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_DRIVER_UART
#   define VSF_ESPIDF_CFG_USE_DRIVER_UART       DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_DRIVER_I2C
#   define VSF_ESPIDF_CFG_USE_DRIVER_I2C        DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_DRIVER_SPI_MASTER
#   define VSF_ESPIDF_CFG_USE_DRIVER_SPI_MASTER DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_DRIVER_LEDC
#   define VSF_ESPIDF_CFG_USE_DRIVER_LEDC       DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_DRIVER_GPTIMER
#   define VSF_ESPIDF_CFG_USE_DRIVER_GPTIMER    DISABLED
#endif
#ifndef VSF_ESPIDF_CFG_USE_DRIVER_ADC
#   define VSF_ESPIDF_CFG_USE_DRIVER_ADC        DISABLED
#endif

#endif      // __VSF_ESPIDF_CFG_H__
