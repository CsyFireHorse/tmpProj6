/*
 * @Author: ChenShengyong
 * @Date: 2025-02-10 13:54:59
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2026-03-03 15:57:52
 */
#ifndef _ZAL_VERSION_H_
#define _ZAL_VERSION_H_

#define ZAL_VERSION_MAJOR 1
#define ZAL_VERSION_MINOR 3
#define ZAL_VERSION_PATCH 137

#define ZAL_AUTHOR "ChenShengyong@zlg.com"
#define ZAL_CODE_DATE "2026-10-30"

#define ZAL_COMMIT_ID ""

#define ZAL_DESCRIPTION "tune to app/include/qcmd;"

#define STRINGIFY(x) #x
#define STRINGIFY_EXPAND(x) STRINGIFY(x)

#define ZAL_VERSION_STR                 \
    STRINGIFY_EXPAND(ZAL_VERSION_MAJOR) \
    "." STRINGIFY_EXPAND(ZAL_VERSION_MINOR) "." STRINGIFY_EXPAND(ZAL_VERSION_PATCH)

// "1.2.3"

#endif