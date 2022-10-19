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

typedef int (*close_file_fn)(FILE *);

struct device_s
{
    const char *name;
    const char *endpoint;
};
typedef struct device_s *device_t;

struct config_s
{
    int ref;
    char *port;
    char *log_path;
    int log_level;
    device_t devices;
    int ndevices;
    void *json;
};
typedef struct config_s *config_t;

struct logger_s
{
    int ref;
    FILE *file;
    int level;
    close_file_fn close_fn;
    int color;
    void *mutex;
};
typedef struct logger_s *logger_t;

#endif