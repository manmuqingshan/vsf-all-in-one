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

/*
 * esp_vfs_port.c -- port layer for esp_vfs.h.
 *
 * See the commentary at the top of include/esp_vfs.h for why the register_*
 * entry points are stubs: VSF's native VFS is reached through vk_fs_mount
 * (component/fs) and is surfaced to user code through the standard POSIX
 * API provided by shell/sys/linux, not through a function-pointer table.
 *
 * This file exists primarily so that translation units written against
 * ESP-IDF still link. Callers that want a concrete mount should use the
 * esp_vfs_fat.h / esp_littlefs.h helpers instead.
 */

/*============================ INCLUDES ======================================*/

#include "../vsf_espidf_cfg.h"

#if VSF_USE_ESPIDF == ENABLED && VSF_ESPIDF_CFG_USE_VFS == ENABLED

#include "esp_vfs.h"

/*============================ IMPLEMENTATION ================================*/

esp_err_t esp_vfs_register(const char *base_path, const esp_vfs_t *vfs, void *ctx)
{
    (void)base_path; (void)vfs; (void)ctx;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t esp_vfs_unregister(const char *base_path)
{
    (void)base_path;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t esp_vfs_register_fd_range(const esp_vfs_t *vfs, void *ctx,
                                    int min_fd, int max_fd)
{
    (void)vfs; (void)ctx; (void)min_fd; (void)max_fd;
    return ESP_ERR_NOT_SUPPORTED;
}

#endif      // VSF_USE_ESPIDF && VSF_ESPIDF_CFG_USE_VFS
