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

#include "component/vsf_component_cfg.h"

#if VSF_USE_LITTLEFS == ENABLED

// to access protected member in mal->drv
#define __VSF_MAL_CLASS_INHERIT__
#include "component/fs/vsf_fs.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

#ifdef LFS_CONFIG
// Software CRC implementation with small lookup table
uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size) {
    static const uint32_t rtable[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };

    const uint8_t *data = buffer;

    for (size_t i = 0; i < size; i++) {
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
    }

    return crc;
}
#endif

static int __vsf_lfs_mal_read(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
    VSF_FS_ASSERT(vsf_eda_is_stack_owner(vsf_eda_get_cur()));
    VSF_FS_ASSERT(c != NULL);
    VSF_FS_ASSERT(c->context != NULL);

    vk_mal_t *mal = (vk_mal_t *)c->context;
    vk_mal_read(mal, block * c->block_size + off, size, (uint8_t *)buffer);
    int ret = (int)vsf_eda_get_return_value();
    ret = (ret == size) ? 0 : -1;
    return ret;
}

static int __vsf_lfs_mal_prog(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size)
{
    VSF_FS_ASSERT(vsf_eda_is_stack_owner(vsf_eda_get_cur()));
    VSF_FS_ASSERT(c != NULL);
    VSF_FS_ASSERT(c->context != NULL);

    vk_mal_t *mal = (vk_mal_t *)c->context;
    vk_mal_write(mal, block * c->block_size + off, size, (uint8_t *)buffer);
    int ret = (int)vsf_eda_get_return_value();
    ret = (ret == size) ? 0 : -1;
    return ret;
}

static int __vsf_lfs_mal_erase(const struct lfs_config *c, lfs_block_t block)
{
    VSF_FS_ASSERT(vsf_eda_is_stack_owner(vsf_eda_get_cur()));
    VSF_FS_ASSERT(c != NULL);
    VSF_FS_ASSERT(c->context != NULL);

    vk_mal_t *mal = (vk_mal_t *)c->context;
    if (mal->drv->erase != NULL) {
        vk_mal_erase(mal, block * c->block_size, c->block_size);
        return (int)vsf_eda_get_return_value();
    }
    return 0;
}

static int __vsf_lfs_mal_sync(const struct lfs_config *c)
{
    VSF_FS_ASSERT(vsf_eda_is_stack_owner(vsf_eda_get_cur()));
    VSF_FS_ASSERT(c != NULL);
    VSF_FS_ASSERT(c->context != NULL);

    return 0;
}

int vsf_lfs_bind_mal(struct lfs_config *c, vk_mal_t *mal)
{
    c->context = mal;
    c->read = __vsf_lfs_mal_read;
    c->prog = __vsf_lfs_mal_prog;
    c->erase = __vsf_lfs_mal_erase;
    c->sync = __vsf_lfs_mal_sync;
    c->block_cycles = 500;

    lfs_size_t mal_blksz_erase = vk_mal_blksz(mal, 0, 0, VSF_MAL_OP_ERASE);
    lfs_size_t mal_blksz_write = vk_mal_blksz(mal, 0, 0, VSF_MAL_OP_WRITE);
    lfs_size_t mal_blksz_read = vk_mal_blksz(mal, 0, 0, VSF_MAL_OP_READ);
    lfs_size_t mal_blksz_rw = vsf_max(mal_blksz_read, mal_blksz_write);

    c->read_size = mal_blksz_read;
    c->prog_size = mal_blksz_write;
    c->cache_size = mal_blksz_rw;
#ifndef LFS_READONLY
    c->block_size = mal_blksz_erase;
    c->block_count = mal->size / mal_blksz_erase;
#else
    c->block_size = mal_blksz_rw;
    c->block_count = mal->size / mal_blksz_rw;
#endif

    c->lookahead_size = vsf_min(c->block_count, 32);
    // lookahead_size MUST be multiple of 8
    c->lookahead_size = (c->lookahead_size + 7) & ~7;
    return 0;
}

#endif      // VSF_USE_LITTLEFS
