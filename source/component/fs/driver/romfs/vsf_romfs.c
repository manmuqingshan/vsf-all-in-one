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

#include "../../vsf_fs_cfg.h"

#if VSF_USE_FS == ENABLED && VSF_FS_USE_ROMFS == ENABLED

#define __VSF_FS_CLASS_INHERIT__
#define __VSF_ROMFS_CLASS_IMPLEMENT
#include "../../vsf_fs.h"

/*============================ MACROS ========================================*/

#if VSF_ROMFS_CFG_DIRECT_ACCESS != ENABLED
#   error currently only VSF_ROMFS_CFG_DIRECT_ACCESS mode is supported
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef enum vk_romfs_file_attr_t {
    ROMFS_FILEATTR_HRD      = 0,
    ROMFS_FILEATTR_DIR      = 1,
    ROMFS_FILEATTR_REG      = 2,
    ROMFS_FILEATTR_LNK      = 3,
    ROMFS_FILEATTR_BLK      = 4,
    ROMFS_FILEATTR_CHR      = 5,
    ROMFS_FILEATTR_SCK      = 6,
    ROMFS_FILEATTR_FIF      = 7,
    ROMFS_FILEATTR_MSK      = 0x07,
    ROMFS_FILEATTR_EXEC_MSK = 8,
} vk_romfs_file_attr_t;

/*============================ PROTOTYPES ====================================*/

dcl_vsf_peda_methods(static, __vk_romfs_mount)
dcl_vsf_peda_methods(static, __vk_romfs_lookup)
dcl_vsf_peda_methods(static, __vk_romfs_read)
dcl_vsf_peda_methods(static, __vk_romfs_setpos)
static void * __vk_romfs_direct_access(vk_file_t *file);

/*============================ GLOBAL VARIABLES ==============================*/

#if     __IS_COMPILER_GCC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

const vk_fs_op_t vk_romfs_op = {
    .fn_mount       = (vsf_peda_evthandler_t)vsf_peda_func(__vk_romfs_mount),
    .fn_unmount     = (vsf_peda_evthandler_t)vsf_peda_func(vk_fsop_succeed),
#if VSF_FS_CFG_USE_CACHE == ENABLED
    .fn_sync        = (vsf_peda_evthandler_t)vsf_peda_func(vk_fsop_succeed),
#endif
    .fn_rename      = (vsf_peda_evthandler_t)vsf_peda_func(vk_fsop_not_support),
    .fop            = {
        .fn_read    = (vsf_peda_evthandler_t)vsf_peda_func(__vk_romfs_read),
        .fn_write   = (vsf_peda_evthandler_t)vsf_peda_func(vk_fsop_not_support),
        .fn_close   = (vsf_peda_evthandler_t)vsf_peda_func(vk_fsop_succeed),
        .fn_setsize = (vsf_peda_evthandler_t)vsf_peda_func(vk_fsop_not_support),
        .fn_setpos  = (vsf_peda_evthandler_t)vsf_peda_func(__vk_romfs_setpos),
        .fn_direct_access = __vk_romfs_direct_access,
    },
    .dop            = {
        .fn_lookup  = (vsf_peda_evthandler_t)vsf_peda_func(__vk_romfs_lookup),
        .fn_create  = (vsf_peda_evthandler_t)vsf_peda_func(vk_fsop_not_support),
        .fn_unlink  = (vsf_peda_evthandler_t)vsf_peda_func(vk_fsop_not_support),
        .fn_chmod   = (vsf_peda_evthandler_t)vsf_peda_func(vk_fsop_not_support),
    },
};

#if     __IS_COMPILER_GCC__
#   pragma GCC diagnostic pop
#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

__vsf_component_peda_ifs_entry(__vk_romfs_mount, vk_fs_mount)
{
    vsf_peda_begin();
    vk_vfs_file_t *dir = (vk_vfs_file_t *)&vsf_this;
    vk_romfs_info_t *fsinfo = dir->subfs.data;
    VSF_FS_ASSERT((fsinfo != NULL) && (fsinfo->root.header != NULL));

    // basic check for romfs
    if (    (fsinfo->root.header->nextfh != be32_to_cpu(0x2d726f6d))
        ||  (fsinfo->root.header->spec != be32_to_cpu(0x3166732d))) {
        vsf_eda_return(VSF_ERR_FAIL);
        return;
    }

    fsinfo->root.attr = VSF_FILE_ATTR_DIRECTORY;
    dir->subfs.root = &fsinfo->root.use_as__vk_file_t;
    vsf_eda_return(VSF_ERR_NONE);
    vsf_peda_end();
}

__vsf_component_peda_ifs_entry(__vk_romfs_lookup, vk_file_lookup)
{
    vsf_peda_begin();
    vk_romfs_file_t *dir = (vk_romfs_file_t *)&vsf_this;
    vk_romfs_info_t *fsinfo = (vk_romfs_info_t *)dir->fsinfo;
    vk_romfs_header_t *header = (vk_romfs_header_t *)&dir->header[1];
    uint32_t nextfh = be32_to_cpu(header->nextfh);

    const char *name = vsf_local.name;
    uint_fast32_t idx = dir->pos;
    bool found = false;

    while (true) {
        if (    (name && vk_file_is_match((char *)name, (char *)header->name))
            ||  (!name && !idx)) {
            found = true;
            break;
        }
        idx--;
        nextfh &= ~(ROMFS_FILEATTR_MSK | ROMFS_FILEATTR_EXEC_MSK);
        if (0 == nextfh) {
            break;
        }
        header = (vk_romfs_header_t*)((uint8_t *)fsinfo->root.header + nextfh);
        nextfh = be32_to_cpu(header->nextfh);
    }

    if (NULL == name) {
        if (found) {
            dir->pos++;
        } else {
            dir->pos = 0;
        }
    }
    if (found) {
        vk_romfs_file_t *new_file = (vk_romfs_file_t *)vk_file_alloc(sizeof(vk_romfs_file_t));
        if (NULL == new_file) {
            *vsf_local.result = NULL;
            vsf_eda_return(VSF_ERR_NOT_ENOUGH_RESOURCES);
        } else {
            new_file->header = header;
            new_file->name = (char *)header->name;
            new_file->attr = VSF_FILE_ATTR_READ;

            uint32_t romfs_attr = nextfh & ROMFS_FILEATTR_MSK;
            while (ROMFS_FILEATTR_HRD == romfs_attr) {
                header = (vk_romfs_header_t *)((uint8_t *)fsinfo->root.header + be32_to_cpu(header->spec));
                nextfh = be32_to_cpu(header->nextfh);
                romfs_attr = nextfh & ROMFS_FILEATTR_MSK;
            }

            switch (romfs_attr) {
            case ROMFS_FILEATTR_DIR:
                new_file->attr |= VSF_FILE_ATTR_DIRECTORY;
                break;
            case ROMFS_FILEATTR_LNK:
                new_file->attr |= VSF_FILE_ATTR_LNK;
                break;
            }
            if (romfs_attr != ROMFS_FILEATTR_DIR) {
                new_file->size = be32_to_cpu(header->size);
                if (nextfh & ROMFS_FILEATTR_EXEC_MSK) {
                    new_file->attr |= VSF_FILE_ATTR_EXECUTE;
                }
            }
            new_file->fsop = &vk_romfs_op;

            *vsf_local.result = &new_file->use_as__vk_file_t;
            vsf_eda_return(VSF_ERR_NONE);
        }
    } else {
        *vsf_local.result = NULL;
        vsf_eda_return(VSF_ERR_NOT_AVAILABLE);
    }
    vsf_peda_end();
}

__vsf_component_peda_ifs_entry(__vk_romfs_read, vk_file_read)
{
    vsf_peda_begin();
    vk_romfs_file_t *file = (vk_romfs_file_t *)&vsf_this;
    vk_romfs_info_t *fsinfo = (vk_romfs_info_t *)file->fsinfo;
    vk_romfs_header_t *header = file->header;
    uint32_t nextfh = be32_to_cpu(header->nextfh);
    uint32_t romfs_attr = nextfh & ROMFS_FILEATTR_MSK;

    uint_fast32_t size = vsf_local.size;
    uint8_t *buff = vsf_local.buff;

    if (ROMFS_FILEATTR_HRD == romfs_attr) {
        header = (vk_romfs_header_t *)((uint8_t *)fsinfo->root.header + be32_to_cpu(header->spec));
    }
    int_fast32_t rsize = file->size - file->pos;
    rsize = vsf_min(size, rsize);
    memcpy(buff, (uint8_t *)&header[1] + file->pos, rsize);
    vsf_eda_return(rsize);
    vsf_peda_end();
}

__vsf_component_peda_ifs_entry(__vk_romfs_setpos, vk_file_setpos)
{
    vsf_peda_begin();
    vk_romfs_file_t *file = (vk_romfs_file_t *)&vsf_this;

    VSF_FS_ASSERT(file != NULL);
    *vsf_local.result = file->pos = vsf_local.offset;
    vsf_eda_return(VSF_ERR_NONE);

    vsf_peda_end();
}

static void * __vk_romfs_direct_access(vk_file_t *file)
{
    vk_romfs_file_t *romfs_file = (vk_romfs_file_t *)file;
    return &romfs_file->header[1];
}

#endif