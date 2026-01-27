#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>

/* ANSI color codes */
#define CLR_RESET "\x1b[0m"
#define CLR_RED "\x1b[31m"
#define CLR_GREEN "\x1b[32m"
#define CLR_YELLOW "\x1b[33m"
#define CLR_BLUE "\x1b[34m"
#define CLR_MAGENTA "\x1b[35m"
#define CLR_CYAN "\x1b[36m"

/* global flags */
bool LOG_INFO_SHOW = true;
bool LOG_DEBUG_SHOW = true;
bool LOG_WARN_SHOW = true;
bool LOG_TRACE_SHOW = true;
bool LOG_ERROR_SHOW = true;
bool LOG_UNAUTHORIZED_SHOW = true;

static const char *level_str(LogLevel level)
{
    switch (level)
    {
    case LOG_INFO:
        return "INFO";
    case LOG_DEBUG:
        return "DEBUG";
    case LOG_WARN:
        return "WARN";
    case LOG_TRACE:
        return "TRACE";
    case LOG_ERROR:
        return "ERROR";
    case LOG_UNAUTHORIZED:
        return "UNAUTHORIZED";
    default:
        return "UNK";
    }
}

static const char *level_color(LogLevel level)
{
    switch (level)
    {
    case LOG_INFO:
        return CLR_GREEN;
    case LOG_DEBUG:
        return CLR_CYAN;
    case LOG_WARN:
        return CLR_YELLOW;
    case LOG_TRACE:
        return CLR_BLUE;
    case LOG_ERROR:
        return CLR_RED;
    case LOG_UNAUTHORIZED:
        return CLR_MAGENTA;
    default:
        return CLR_RESET;
    }
}

static bool should_show(LogLevel level)
{
    switch (level)
    {
    case LOG_INFO:
        return LOG_INFO_SHOW;
    case LOG_DEBUG:
        return LOG_DEBUG_SHOW;
    case LOG_WARN:
        return LOG_WARN_SHOW;
    case LOG_TRACE:
        return LOG_TRACE_SHOW;
    case LOG_ERROR:
        return LOG_ERROR_SHOW;
    case LOG_UNAUTHORIZED:
        return LOG_UNAUTHORIZED_SHOW;
    default:
        return false;
    }
}

void log_write(LogLevel level, const char *fmt, ...)
{
    if (!should_show(level))
        return;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    printf("%04d-%02d-%02d %02d:%02d:%02d %s[%s]%s ",
           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
           tm.tm_hour, tm.tm_min, tm.tm_sec,
           level_color(level),
           level_str(level),
           CLR_RESET);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}
