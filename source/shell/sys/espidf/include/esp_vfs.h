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
 * esp_vfs.h -- ESP-IDF VFS compatibility header (clean-room).
 *
 * Scope
 * -----
 * The ESP-IDF VFS layer has two faces:
 *
 *   (1) POSIX filesystem API surface (open/close/read/write/lseek/stat/
 *       opendir/readdir/mkdir/unlink/rename/...). In ESP-IDF these are
 *       implemented inside newlib + esp_vfs_*; in this sub-system they are
 *       provided natively by VSF's linux shell (shell/sys/linux) which
 *       already wraps component/fs (vk_file_* / vk_fs_mount) as synchronous
 *       POSIX calls. Users therefore include the usual <stdio.h> /
 *       <unistd.h> / <fcntl.h> / <sys/stat.h> / <dirent.h> / <sys/mount.h>
 *       headers and get matching behaviour for free. This header forwards
 *       to them so that source files written with #include "esp_vfs.h"
 *       compile unchanged.
 *
 *   (2) Runtime VFS driver registration (esp_vfs_register /
 *       esp_vfs_register_fd_range). ESP-IDF lets callers splice a custom
 *       driver under a path prefix such as "/dev/foo". VSF's native
 *       equivalents are vk_fs_mount + devfs nodes, whose semantics are
 *       not 1:1 compatible with the esp_vfs_t function-pointer table
 *       (async peda sub-calls vs. bare C callbacks). We deliberately
 *       publish the shape of esp_vfs_t and esp_vfs_register() so that
 *       ESP-IDF code compiles, but the register path returns
 *       ESP_ERR_NOT_SUPPORTED. Concrete filesystems (FAT/LittleFS) bypass
 *       this and mount directly via the matching esp_vfs_fat.h /
 *       esp_littlefs.h entry points, which are fully functional on top
 *       of component/fs.
 */

#ifndef __VSF_ESPIDF_ESP_VFS_H__
#define __VSF_ESPIDF_ESP_VFS_H__

/*============================ INCLUDES ======================================*/

#include "esp_err.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// POSIX surface (open/read/write/lseek/stat/opendir/readdir/mkdir/unlink
// /rename/...). VSF linux shell exposes the whole set already.
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/

// Matches ESP-IDF's CONFIG_VFS_SUPPORT_IO constants so existing code that
// checks them compiles. Path length cap is informational: VSF places no
// hard limit of its own beyond the linux shell FD cap.
#ifndef ESP_VFS_PATH_MAX
#   define ESP_VFS_PATH_MAX         15
#endif

#define ESP_VFS_FLAG_DEFAULT        0
#define ESP_VFS_FLAG_CONTEXT_PTR    (1 << 0)
#define ESP_VFS_FLAG_READONLY_FS    (1 << 1)

/*============================ TYPES =========================================*/

// ESP-IDF's esp_vfs_t is a large table of POSIX-shaped function pointers.
// The fields used by nearly all callers are preserved here so that struct
// initialisers written against ESP-IDF compile; unused tail fields are
// deliberately omitted to keep this shim small. Register_* paths do not
// invoke these function pointers (see file header).
struct esp_vfs_t {
    int flags;
    union {
        ssize_t     (*write)   (int fd, const void *data, size_t size);
        ssize_t     (*write_p) (void *ctx, int fd, const void *data, size_t size);
    };
    union {
        off_t       (*lseek)   (int fd, off_t size, int mode);
        off_t       (*lseek_p) (void *ctx, int fd, off_t size, int mode);
    };
    union {
        ssize_t     (*read)    (int fd, void *dst, size_t size);
        ssize_t     (*read_p)  (void *ctx, int fd, void *dst, size_t size);
    };
    union {
        int         (*open)    (const char *path, int flags, int mode);
        int         (*open_p)  (void *ctx, const char *path, int flags, int mode);
    };
    union {
        int         (*close)   (int fd);
        int         (*close_p) (void *ctx, int fd);
    };
    union {
        int         (*fstat)   (int fd, struct stat *st);
        int         (*fstat_p) (void *ctx, int fd, struct stat *st);
    };
    union {
        int         (*stat)    (const char *path, struct stat *st);
        int         (*stat_p)  (void *ctx, const char *path, struct stat *st);
    };
    union {
        int         (*unlink)  (const char *path);
        int         (*unlink_p)(void *ctx, const char *path);
    };
    union {
        int         (*rename)  (const char *src, const char *dst);
        int         (*rename_p)(void *ctx, const char *src, const char *dst);
    };
};

typedef struct esp_vfs_t esp_vfs_t;

/*============================ PROTOTYPES ====================================*/

// Register / unregister a custom VFS driver under a path prefix.
//
// Status in this sub-system: NOT IMPLEMENTED. The VSF native VFS is reached
// through vk_fs_mount / devfs, not through a function-pointer table.
// These entry points exist so that code written against ESP-IDF links;
// callers receive ESP_ERR_NOT_SUPPORTED and are expected to migrate to
// vk_fs_mount or the higher-level helpers in esp_vfs_fat.h / esp_littlefs.h.
extern esp_err_t esp_vfs_register(const char *base_path,
                                  const esp_vfs_t *vfs,
                                  void *ctx);
extern esp_err_t esp_vfs_unregister(const char *base_path);
extern esp_err_t esp_vfs_register_fd_range(const esp_vfs_t *vfs, void *ctx,
                                           int min_fd, int max_fd);

#ifdef __cplusplus
}
#endif

#endif      // __VSF_ESPIDF_ESP_VFS_H__
