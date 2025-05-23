#include "../../vsf_linux_cfg.h"

#if VSF_USE_LINUX == ENABLED && VSF_LINUX_USE_BUSYBOX == ENABLED

#if VSF_LINUX_CFG_RELATIVE_PATH == ENABLED
#   include "../../include/unistd.h"
#   include "../../include/fcntl.h"
#else
#   include <unistd.h>
#   include <fcntl.h>
#endif
#if VSF_LINUX_CFG_RELATIVE_PATH == ENABLED && VSF_LINUX_USE_SIMPLE_STDIO == ENABLED
#   include "../../include/simple_libc/stdio.h"
#else
#   include <stdio.h>
#endif

extern void vsh_set_path(char **path);
extern int vsh_main(int argc, char *argv[]);
extern int cd_main(int argc, char *argv[]);
extern int ls_main(int argc, char *argv[]);
extern int pwd_main(int argc, char *argv[]);
extern int cat_main(int argc, char *argv[]);
extern int echo_main(int argc, char *argv[]);
extern int mkdir_main(int argc, char *argv[]);
extern int clear_main(int argc, char *argv[]);
extern int time_main(int argc, char *argv[]);
extern int sleep_main(int argc, char *argv[]);
#if VSF_LINUX_CFG_SUPPORT_SIG == ENABLED
extern int kill_main(int argc, char *argv[]);
#endif
#if VSF_LINUX_LIBC_USE_ENVIRON == ENABLED
extern int export_main(int argc, char *argv[]);
#endif
#if     (VSF_LINUX_CFG_HEAP_SIZE > 0)                                           \
    ||  (   (VSF_HEAP_CFG_STATISTICS == ENABLED)                                \
        &&  (   (VSF_USE_ARCH_HEAP != ENABLED)                                  \
            ||  (VSF_ARCH_HEAP_HAS_STATISTICS == ENABLED)))
extern int free_main(int argc, char *argv[]);
#endif
#if     VSF_LINUX_USE_SOCKET == ENABLED && VSF_LINUX_SOCKET_USE_INET == ENABLED
#   if VSF_LINUX_SOCKET_USE_NETLINK == ENABLED
extern int __vsf_linux_ifconfig_main(int argc, char **argv);
#   endif
#   if VSF_LINUX_SOCKET_USE_ROUTE == ENABLED
extern int __vsf_linux_ip_main(int argc, char **argv);
extern int __vsf_linux_route_main(int argc, char **argv);
#   endif
#endif

extern int vsf_linux_init_main(int argc, char *argv[]);

VSF_CAL_WEAK(vsf_linux_init_main)
int vsf_linux_init_main(int argc, char *argv[])
{
    // run init scripts first
#ifdef VSF_LINUX_CFG_INIT_SCRIPTS
    static const char * __init_scripts[] = {
        VSF_LINUX_CFG_INIT_SCRIPTS
    };
    for (int i = 0; i < dimof(__init_scripts); i++) {
        printf("execute init script: %s\r\n", __init_scripts[i]);
        system(__init_scripts[i]);
    }
#endif
#ifdef VSF_LINUX_CFG_INIT_SCRIPT_FILE
    extern int vsh_run_scripts(const char *scripts);
    vsh_run_scripts(VSF_LINUX_CFG_INIT_SCRIPT_FILE);
#endif

    return vsh_main(argc, argv);
}

int busybox_bind(char *path, vsf_linux_main_entry_t entry)
{
    return vsf_linux_fs_bind_executable(path, entry);
}

#if __IS_COMPILER_GCC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wcast-align"
#elif   __IS_COMPILER_LLVM__ || __IS_COMPILER_ARM_COMPILER_6__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wcast-align"
#endif

int busybox_install(void)
{
    if (    mkdirs(VSF_LINUX_CFG_BIN_PATH, 0)
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/init", vsf_linux_init_main) < 0) {
        return -1;
    }

#if VSF_LINUX_USE_BUSYBOX == ENABLED
    if (    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/sh", vsh_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/ls", ls_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/cd", cd_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/pwd", pwd_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/cat", cat_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/echo", echo_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/mkdir", mkdir_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/clear", clear_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/time", time_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/sleep", sleep_main) < 0
#if VSF_LINUX_CFG_SUPPORT_SIG == ENABLED
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/kill", kill_main) < 0
#endif
#   if VSF_LINUX_LIBC_USE_ENVIRON == ENABLED
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/export", export_main) < 0
#   endif
#   if (VSF_LINUX_CFG_HEAP_SIZE > 0)                                            \
    ||  (   (VSF_HEAP_CFG_STATISTICS == ENABLED)                                \
        &&  (   (VSF_USE_ARCH_HEAP != ENABLED)                                  \
            ||  (VSF_ARCH_HEAP_HAS_STATISTICS == ENABLED)))
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/free", free_main) < 0
#   endif
#if     VSF_LINUX_USE_SOCKET == ENABLED && VSF_LINUX_SOCKET_USE_INET == ENABLED
#   if VSF_LINUX_SOCKET_USE_NETLINK == ENABLED
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/ifconfig", __vsf_linux_ifconfig_main) < 0
#   endif
#   if VSF_LINUX_SOCKET_USE_ROUTE == ENABLED
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/ip", __vsf_linux_ip_main) < 0
        ||  busybox_bind(VSF_LINUX_CFG_BIN_PATH "/route", __vsf_linux_route_main) < 0
#   endif
#endif
        ) {
        return -1;
    }
#endif

#ifndef VSF_LINUX_CFG_PATH
#   define VSF_LINUX_CFG_PATH               VSF_LINUX_CFG_BIN_PATH
#endif
    // GCC: -Wcast-align
    vsh_set_path((char **)VSF_LINUX_CFG_PATH);
    return 0;
}

#if __IS_COMPILER_GCC__
#   pragma GCC diagnostic pop
#elif   __IS_COMPILER_LLVM__ || __IS_COMPILER_ARM_COMPILER_6__
#   pragma clang diagnostic pop
#endif

#endif

