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
#if (VSF_ESPIDF_CFG_USE_TIMER == ENABLED) || (VSF_ESPIDF_CFG_USE_EVENT == ENABLED)
#   include "esp_err.h"
#endif

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/

// Internal singleton state. The ESP-IDF compatibility shim has no
// per-instance notion (esp_* APIs are global functions), so ownership
// of the state belongs to the shim itself -- users only hand in a cfg.
static struct {
    bool        is_inited;
#if VSF_HAL_USE_RNG == ENABLED
    vsf_rng_t  *rng;
#endif
} __vsf_espidf = { 0 };

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

void vsf_espidf_init(const vsf_espidf_cfg_t *cfg)
{
    // Idempotent: a second init is a no-op. cfg == NULL is treated as a
    // zero-initialised cfg so callers with no peripherals to inject can
    // simply call vsf_espidf_init(NULL).
    if (__vsf_espidf.is_inited) {
        return;
    }
#if VSF_HAL_USE_RNG == ENABLED
    __vsf_espidf.rng = (cfg != NULL) ? cfg->rng : NULL;
#endif
    __vsf_espidf.is_inited = true;

    // Per-module init hooks. Only modules with visible init state are
    // chained here; esp_err/esp_log/esp_system/esp_ringbuf are stateless
    // from the sub-system perspective.
#if VSF_ESPIDF_CFG_USE_TIMER == ENABLED
    extern esp_err_t esp_timer_init(void);
    (void)esp_timer_init();
#endif
#if VSF_ESPIDF_CFG_USE_EVENT == ENABLED
    extern esp_err_t esp_event_loop_create_default(void);
    (void)esp_event_loop_create_default();
#endif
    // TODO:
    //   vsf_espidf_nvs_init();
}

#if VSF_HAL_USE_RNG == ENABLED
vsf_rng_t * vsf_espidf_get_rng(void)
{
    return __vsf_espidf.rng;
}
#endif

#endif      // VSF_USE_ESPIDF
