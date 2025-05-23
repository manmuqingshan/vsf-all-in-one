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

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"

#undef VSF_DEVICE_HEADER

#if     defined(__SF32LB520U36__)
#   define  VSF_DEVICE_HEADER       "./SF32LB520U36/device.h"
#elif   defined(__SF32LB523UB6__)
#   define  VSF_DEVICE_HEADER       "./SF32LB523UB6/device.h"
#elif   defined(__SF32LB525UC6__)
#   define  VSF_DEVICE_HEADER       "./SF32LB525UC6/device.h"
#elif   defined(__SF32LB527UD6__))
#   define  VSF_DEVICE_HEADER       "./SF32LB527UD6/device.h"
#else
#   error No supported device found.
#endif

/* include specified device driver header file */
#include VSF_DEVICE_HEADER

#ifdef __VSF_HEADER_ONLY_SHOW_ARCH_INFO__
#   ifndef __CPU_ARM__
#       define __CPU_ARM__
#   endif
#else

#ifndef __HAL_DEVICE_SIFLI_SF32LB52X_H__
#define __HAL_DEVICE_SIFLI_SF32LB52X_H__

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __HAL_DEVICE_SIFLI_SF32LB52X_H__
#endif      // __VSF_HEADER_ONLY_SHOW_ARCH_INFO__
/* EOF */
