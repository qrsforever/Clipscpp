/***************************************************************************
 *  Log.h - Log header for clipsc++
 *
 *  Created: 2018-06-06 19:04:43
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __Log_H__
#define __Log_H__

#include <stdio.h>
#include <stdint.h>

enum LogLevel {
    LOG_LEVEL_ERROR   = 0x01,
    LOG_LEVEL_WARNING = 0x02,
    LOG_LEVEL_DEBUG  = 0x03,
    LOG_LEVEL_INFO = 0x04,
    LOG_LEVEL_TRACE = 0x05
};

#ifdef NDEBUG

#define LOGE(args...) ((void)0)
#define LOGW(args...) ((void)0)
#define LOGD(args...) ((void)0)
#define LOGI(args...) ((void)0)

#else /* NDEBUG */

extern int g_logLevel;
#define LOGE(args...) \
do { \
    if (g_logLevel >= LOG_LEVEL_ERROR) \
        logVerbose(__FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_ERROR, args); \
} while(0)

#define LOGW(args...) \
do { \
    if (g_logLevel >= LOG_LEVEL_WARNING) \
        logVerbose(__FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_WARNING, args); \
} while(0)

#define LOGD(args...) \
do { \
    if (g_logLevel >= LOG_LEVEL_DEBUG) \
        logVerbose(__FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_DEBUG, args); \
} while(0)

#define LOGI(args...) \
do { \
    if (g_logLevel >= LOG_LEVEL_INFO) \
        logVerbose(__FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_INFO, args); \
} while(0)

#define LOGT(args...) \
do { \
    if (g_logLevel >= LOG_LEVEL_TRACE) \
        logVerbose(__FILE__, __LINE__, __FUNCTION__, LOG_LEVEL_TRACE, args); \
} while(0)

#endif /* NDEBUG */

#ifdef __cplusplus
extern "C" {
#endif

void logVerbose(const char *file, int line, const char *function, int level, const char *fmt, ...);
void setLogLevel(int level);
int init_clips_logger(void *env);
int finalize_clips_logger(void *env);

#ifdef __cplusplus
}
#endif

#endif /* __Log_H__ */
