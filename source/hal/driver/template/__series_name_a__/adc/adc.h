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

#ifndef __HAL_DRIVER_${DEVICE}_ADC_H__
#define __HAL_DRIVER_${DEVICE}_ADC_H__

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"

#if VSF_HAL_USE_ADC == ENABLED

/*\note Incudes CAN ONLY be put here. */

/*\note ADC can be a driver to a actual device, or to a IPCore driver/emulated driver.
 *      If current driver is for a device, instance structure is not open to user.
 *      Otherwise instance structure should be open to user,
 *          in this case include the code between "IPCore" and "IPCore end" comments.
 */

// IPCore
#if     defined(__VSF_HAL_${ADC_IP}_ADC_CLASS_IMPLEMENT)
#   define __VSF_CLASS_IMPLEMENT__
#elif   defined(__VSF_HAL_${ADC_IP}_ADC_CLASS_INHERIT__)
#   define __VSF_CLASS_INHERIT__
#endif

#include "utilities/ooc_class.h"
// IPCore end

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/

/*\note VSF_${ADC_IP}_ADC_CFG_MULTI_CLASS should be implemented for IP drives and open to user,
 *          while VSF_HW_ADC_CFG_MULTI_CLASS should be in adc.c.
 */

// IPCore
#ifndef VSF_${ADC_IP}_ADC_CFG_MULTI_CLASS
#   define VSF_${ADC_IP}_ADC_CFG_MULTI_CLASS    VSF_ADC_CFG_MULTI_CLASS
#endif
// IPCore end

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

// IPCore
vsf_class(vsf_${adc_ip}_adc_t) {
#if VSF_${ADC_IP}_CFG_MULTI_CLASS == ENABLED
    public_member(
        vsf_adc_t               vsf_adc;
    )
#endif

/*\note You can add more memmber in vsf_${adc_ip}_adc_t instance.
 *      For members accessable from child, put in protected_member.
 *      Else, put in private_member.
 */

    protected_member(
        vsf_${adc_ip}_adc_reg_t *reg;
        vsf_adc_isr_t           isr;
    )
};
// IPCore end

/*============================ INCLUDES ======================================*/

// IPCore
/*\note Extern APIs for ip core diriver.
 *      There is no requirement about how APIs of IPCore drivers should be implemented.
 *      Just consider the simplicity for drivers of actual device.
 */
// IPCore end

#ifdef __cplusplus
}
#endif

#endif /* VSF_HAL_USE_ADC */
#endif
/* EOF */