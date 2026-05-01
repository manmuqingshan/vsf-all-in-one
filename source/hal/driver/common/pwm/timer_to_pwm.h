/*****************************************************************************
 *   Copyright(C)2009-2022 by VSF Team                                       *
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

#ifndef __HAL_DRIVER_COMMON_PWM_TIMER_TO_PWM_H__
#define __HAL_DRIVER_COMMON_PWM_TIMER_TO_PWM_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"

#if VSF_HAL_USE_PWM == ENABLED && VSF_HAL_USE_TIMER == ENABLED

#include "hal/driver/common/template/vsf_template_hal_driver.h"
#include "hal/driver/common/template/vsf_template_pwm.h"
#include "hal/driver/common/template/vsf_template_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/

/*
 * Default MULTI_CLASS follows VSF_PWM_CFG_MULTI_CLASS.
 * When ENABLED, vsf_hw_pwm_t inherits vsf_pwm_t so it can be accessed through
 * the class dispatcher; when DISABLED, callers must use vsf_hw_pwm_* directly.
 */
#ifndef VSF_HW_PWM_CFG_MULTI_CLASS
#   define VSF_HW_PWM_CFG_MULTI_CLASS           VSF_PWM_CFG_MULTI_CLASS
#endif

/*
 * Capability defaults (Hz). Can be overridden by the hw driver before
 * including timer_to_pwm.inc.
 */
#ifndef VSF_HW_PWM_CFG_CAPABILITY_MIN_FREQ
#   define VSF_HW_PWM_CFG_CAPABILITY_MIN_FREQ   (1ul * 1000)
#endif
#ifndef VSF_HW_PWM_CFG_CAPABILITY_MAX_FREQ
#   define VSF_HW_PWM_CFG_CAPABILITY_MAX_FREQ   (100ul * 1000 * 1000)
#endif

/*============================ TYPES =========================================*/

/*
 * Generic hw PWM instance that maps every call onto a vsf_timer_t.
 *
 * .timer:   pointer to the underlying timer class instance (cast from the
 *           concrete vsf_hw_timer_t* on init-time). Uses vsf_timer_t so the
 *           glue stays chip-agnostic and goes through the dispatcher.
 *           REQUIRES VSF_TIMER_CFG_MULTI_CLASS == ENABLED.
 * .channel: timer channel bound to this PWM instance (0 for most chips).
 * .cfg:     cached user config (for get_configuration).
 * .period:  cached full period in timer ticks, written by pwm_set(), used by
 *           pwm_enable() to re-arm after a stop/enable cycle.
 */
typedef struct vsf_hw_pwm_t {
#if VSF_HW_PWM_CFG_MULTI_CLASS == ENABLED
    vsf_pwm_t           vsf_pwm;
#endif
    vsf_timer_t        *timer;
    uint8_t             channel;
    bool                enabled;
    bool                configured;
    vsf_pwm_cfg_t       cfg;
    uint32_t            period;
    uint32_t            pulse;
} vsf_hw_pwm_t;

/*============================ MACROFIED FUNCTIONS ===========================*/

/*
 * Default per-instance initializer used by timer_to_pwm.inc when the caller
 * does not override VSF_PWM_CFG_IMP_LV0. Assumes "PWMx <-> TIMx" one-to-one
 * mapping and channel 0. Override VSF_PWM_CFG_IMP_LV0 before including
 * timer_to_pwm.inc if a different mapping is needed.
 */
#ifndef VSF_TIMER_TO_PWM_DEFAULT_TIMER
#   define VSF_TIMER_TO_PWM_DEFAULT_TIMER(__IDX) \
        (vsf_timer_t *)&VSF_MCONNECT(vsf_hw_timer, __IDX)
#endif
#ifndef VSF_TIMER_TO_PWM_DEFAULT_CHANNEL
#   define VSF_TIMER_TO_PWM_DEFAULT_CHANNEL(__IDX) 0
#endif

#ifdef __cplusplus
}
#endif

#endif  /* VSF_HAL_USE_PWM && VSF_HAL_USE_TIMER */
#endif  /* __HAL_DRIVER_COMMON_PWM_TIMER_TO_PWM_H__ */
/* EOF */
