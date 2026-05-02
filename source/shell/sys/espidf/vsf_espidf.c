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

#include "./vsf_espidf_cfg.h"

#if VSF_USE_ESPIDF == ENABLED

#include "./vsf_espidf.h"

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/

static vsf_espidf_t __vsf_espidf = { 0 };

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

void vsf_espidf_init(void)
{
    if (__vsf_espidf.is_inited) {
        return;
    }
    __vsf_espidf.is_inited = true;

    // TODO: per-module init hooks will be chained here as modules land:
    //   vsf_espidf_log_init();
    //   vsf_espidf_event_init();
    //   vsf_espidf_timer_init();
    //   vsf_espidf_nvs_init();
    //   ...
}

#endif      // VSF_USE_ESPIDF
