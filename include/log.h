#ifndef LOG_H
#define LOG_H

typedef enum
{
    LOG_INFO,
    LOG_DEBUG,
    LOG_WARN,
    LOG_TRACE,
    LOG_ERROR,
    LOG_UNAUTHORIZED
} LogLevel;

void log_write(LogLevel level, const char *fmt, ...);

#endif
