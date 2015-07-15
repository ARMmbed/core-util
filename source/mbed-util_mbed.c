// Copyright (C) 2015 ARM Limited. All rights reserved.

#ifdef TARGET_LIKE_MBED // only include this code for mbed targets

#include <stdlib.h>
#include <stdarg.h>
#include "device.h"
#include "mbed/mbed_interface.h"
#include "mbed-util/mbed-util.h"
#if DEVICE_STDIO_MESSAGES
#include <stdio.h>
#endif

void mbed_util_runtime_error_internal(const char *file, int line, const char* format, ...) {
#if DEVICE_STDIO_MESSAGES
    fprintf(stderr, "Runtime error in file %s, line %d: ", file, line);
    va_list arg;
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
#endif
    exit(1);
}

void mbed_util_assert_internal(const char *expr, const char *file, int line, const char* msg)
{
#if DEVICE_STDIO_MESSAGES
    fprintf(stderr, "assertation failed: %s, file: %s, line %d", expr, file, line);
    if (msg)
        fprintf(stderr, " (%s)", msg);
    fprintf(stderr, "\r\n");
#endif
    mbed_die();
}

#endif // #ifdef TARGET_LIKE_MBED

