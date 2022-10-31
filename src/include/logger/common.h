#ifndef COMMON_H
#define COMMON_H
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>

#define STATUS_OK 0
#define STATUS_ERROR 1

#define IF_THROW(CONDITION, ERROR) \
    if (CONDITION)                 \
    {                              \
        if (error != NULL)         \
        {                          \
            *error = ERROR;        \
        }                          \
        goto error;                \
    }

#define CLEANUP_FUNCTION(OBJECT, FUNCTION) \
    if (OBJECT != NULL)                    \
    {                                      \
        FUNCTION;                      \
    }

#define CLEANUP(OBJECT) CLEANUP_FUNCTION(OBJECT, free(OBJECT))

enum log_levels
{
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
};

#endif