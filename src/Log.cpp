/***************************************************************************
 *  Log.cpp - Log impl for clipsc++
 *
 *  Created: 2018-06-06 19:05:10
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Log.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <string>

extern "C" {
#include "clips.h"
}

#define LOG_INFO_NAME   "info"
#define LOG_DEBUG_NAME  "debug"
#define LOG_WARN_NAME   "warn"
#define LOG_ERROR_NAME  "error"

int g_logLevel = LOG_LEVEL_TRACE;

static std::string g_buffer;

static const char* textLevel[] = {"Assert", "Error!", "Warning", "Debug", "Info", "Trace"};

extern "C"
void logVerbose(const char *file, int line, const char *function, int level, const char *fmt, ...)
{
    static char buffer[2048] = { 0 };
    const char* pFile = strrchr(file, '/');
    if (pFile)
        pFile = pFile + 1;
    else
        pFile = file;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 2047, fmt, args);
    printf("%s:%d | %s | %s | %s",  pFile, line, function, textLevel[level], buffer);
    va_end(args);
}

extern "C"
void setLogLevel(int level)
{
    g_logLevel = level;
}

static int s_log_router_query(void *env, const char *logicalName)
{
    if (strcmp(logicalName, LOG_INFO_NAME) == 0) return TRUE;
    if (strcmp(logicalName, LOG_DEBUG_NAME) == 0) return TRUE;
    if (strcmp(logicalName, LOG_WARN_NAME) == 0) return TRUE;
    if (strcmp(logicalName, LOG_ERROR_NAME) == 0) return TRUE;
    if (strcmp(logicalName, WTRACE) == 0) return TRUE;
    if (strcmp(logicalName, STDOUT) == 0) return TRUE;
    if (strcmp(logicalName, WWARNING) == 0) return TRUE;
    if (strcmp(logicalName, WERROR) == 0) return TRUE;
    if (strcmp(logicalName, WDISPLAY) == 0) return TRUE;
    return FALSE;
}

static int s_log_router_print(void *env, const char *logicalName, const char *str)
{
    if (strcmp(str, "\n") != 0) {
        g_buffer += str;
        return TRUE;
    }
    g_buffer += "\n";
    if (strcmp(logicalName, LOG_INFO_NAME) == 0
        || strcmp(logicalName, WDISPLAY) == 0) {
        LOGI("Clips: %s", g_buffer.c_str());
    } else if (strcmp(logicalName, LOG_DEBUG_NAME) == 0
        || strcmp(logicalName, STDOUT) == 0) {
        LOGD("Clips: %s", g_buffer.c_str());
    } else if (strcmp(logicalName, LOG_WARN_NAME) == 0
        || strcmp(logicalName, WWARNING) == 0) {
        LOGW("Clips: %s", g_buffer.c_str());
    } else if (strcmp(logicalName, LOG_ERROR_NAME) == 0
        || strcmp(logicalName, WERROR) == 0) {
        LOGE("Clips: %s", g_buffer.c_str());
    } else if (strcmp(logicalName, WTRACE) == 0) {
        LOGT("Clips: %s", g_buffer.c_str());
    }
    g_buffer.clear();
    return TRUE;
}

static int s_log_router_exit(void *env, int exitCode)
{
    return TRUE;
}

int init_clips_logger(void *env)
{
    /* int EnvAddRouterWithContext(
     *     environment,routerName,priority,queryFunction,
     *     printFunction, getcFunction,ungetcFunction,
     *     exitFunction,context); */
    int ret = EnvAddRouterWithContext(
        env,
        "clipsc++",
        30,
        s_log_router_query,
        s_log_router_print,
        0, 0,
        s_log_router_exit,
        0);

    /* if (ret == TRUE) {
     *     ret = EnvActivateRouter(env, (char*)logger->get_router_name().c_str());
     *     LOGD("EnvActivateRouter: %d\n", ret);
     * }  */
    return ret;
}

int finalize_clips_logger(void *env)
{
    return EnvDeleteRouter(env, "clipsc++");
}
