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

#ifndef __VSF_ESPIDF_INTERNAL_H__
#define __VSF_ESPIDF_INTERNAL_H__

/*============================ INCLUDES ======================================*/

#include "kernel/vsf_kernel.h"
#include "./vsf_espidf_cfg.h"

#if VSF_USE_ESPIDF == ENABLED

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/

typedef struct vsf_espidf_t {
    bool is_inited;
} vsf_espidf_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

// Sub-system lifecycle. Called from user init (once).
extern void vsf_espidf_init(void);

#ifdef __cplusplus
}
#endif

#endif      // VSF_USE_ESPIDF
#endif      // __VSF_ESPIDF_INTERNAL_H__
