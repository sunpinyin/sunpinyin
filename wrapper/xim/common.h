/**
 * @file see ``LICENSE`` for further details
 * @author Mike
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

#ifdef __cplusplus
#define __EXPORT_API extern "C"
#else
#define __EXPORT_API
#endif

#ifdef LOG_ENABLED

#define __PRINT(label, ...)                             \
    do {                                                \
        fprintf(stderr, "[%s] %s at %s: ", label,       \
                __FILE__, __FUNCTION__);                \
        fprintf(stderr, __VA_ARGS__);                   \
        fprintf(stderr, "\n");                          \
    } while (0)                                         \

#define LOG(...) __PRINT("log",  __VA_ARGS__)
#define DEBUG(...) __PRINT("debug",  __VA_ARGS__)
#else
#define LOG(...) 
#define DEBUG(...)
#endif

#define XIM_VERSION "0.0.3"
#define XIM_PROGRAM_NAME "SunPinyin-XIM"
#define XIM_WEBSITE "http://mike.struct.cn/sunpinyin-xim"
#define XIM_COMMENTS "a XIM front-end for SunPinyin."


#endif /* _COMMON_H_ */
