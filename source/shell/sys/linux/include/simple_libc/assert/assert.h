#ifndef __SIMPLE_LIBC_ASSERT_H__
#define __SIMPLE_LIBC_ASSERT_H__

// for VSF_ASSERT
#include "utilities/compiler/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define assert              VSF_ASSERT

#ifdef __cplusplus
}
#endif

#endif