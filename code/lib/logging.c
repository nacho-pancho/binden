/**
 * \file logging.c
 * \brief Bare bones logger
 */
#include "logging.h"
#include <stdio.h>

static int log_level = LOG_INFO;
static FILE* log_stream = NULL;

void set_log_level(int l) {
    log_level = l;
}

void set_log_stream(FILE* stream) {
    log_stream = stream;
}

void  error(const char *format, ... ) {
    va_list args;
    va_start(args, format);
    if (log_level <= LOG_ERROR) {
        vfprintf(stderr, format, args);
    }
    va_end(args);
}

void warn(const char *format, ... ) {
    va_list args;
    va_start(args, format);
    if (log_level <= LOG_WARNING) {
        vfprintf(stderr, format, args);
    }
    va_end(args);
}

void info(const char *format, ... ) {
    va_list args;
    if (!log_stream) {
        log_stream = stdout;
    }
    va_start(args, format);
    if (log_level <= LOG_INFO) {
        vfprintf(log_stream, format, args);
    }
    va_end(args);
}

void debug(const char *format, ... ) {
    va_list args;
    if (!log_stream) {
        log_stream = stdout;
    }
    va_start(args, format);
    if (log_level <= LOG_DEBUG) {
        vfprintf(log_stream, format, args);
    }
    va_end(args);
}


char level_warn(void) {
    return log_level <= LOG_WARNING;
}
char level_info(void) {
    return log_level <= LOG_INFO;
}
char level_error(void) {
    return log_level <= LOG_ERROR;
}
char level_debug(void) {
    return log_level <= LOG_DEBUG;
}
