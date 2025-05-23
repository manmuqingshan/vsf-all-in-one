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

#ifndef __VSF_TINY_GUI_V_PORT_H__
#define __VSF_TINY_GUI_V_PORT_H__

/*============================ INCLUDES ======================================*/

#include "../../vsf_tgui_cfg.h"

#if     VSF_USE_TINY_GUI == ENABLED                                             \
    &&  VSF_TGUI_CFG_RENDERING_TEMPLATE_SEL == VSF_TGUI_V_TEMPLATE_SIMPLE_VIEW

#include "./vsf_tgui_sv_color.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/

#ifndef VSF_TGUI_HOR_MAX
#   define VSF_TGUI_HOR_MAX                         800
#endif

#ifndef VSF_TGUI_VER_MAX
#   define VSF_TGUI_VER_MAX                         600
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern
void vsf_tgui_sv_port_draw_rect(vsf_tgui_t* gui_ptr,
                        vsf_tgui_location_t* location_ptr,
                        vsf_tgui_size_t* size_ptr,
                        vsf_tgui_v_color_t tColor);

extern
void vsf_tgui_sv_port_draw_char(vsf_tgui_t* gui_ptr,
                        vsf_tgui_location_t* location_ptr,
                        vsf_tgui_location_t* font_location_ptr,
                        vsf_tgui_size_t* size_ptr,
                        const uint8_t chFontIndex,
                        uint32_t char_u32,
                        vsf_tgui_v_color_t tCharColor);

extern
void vsf_tgui_sv_port_draw_root_tile(vsf_tgui_t* gui_ptr,
                        vsf_tgui_location_t* location_ptr,
                        vsf_tgui_location_t* tile_ptrLocation,
                        vsf_tgui_size_t* size_ptr,
                        const vsf_tgui_tile_t* tile_ptr,
                        uint_fast8_t trans_rate,
                        vsf_tgui_v_color_t color,
                        vsf_tgui_v_color_t bg_color);

#ifdef __cplusplus
}
#endif

#endif
#endif
/* EOF */
