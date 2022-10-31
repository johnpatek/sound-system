#ifndef CONFIG_H
#define CONFIG_H
#include "common.h"

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

typedef void (*device_iterator_fn)(device_t, void*);

int config_create(config_t *config, const char ** error);

int config_load(config_t config, const char * const path, const char ** error);

int config_validate(config_t config, const char ** error);

void config_iterate_devices(config_t config, device_iterator_fn device_fn, void * user_data);

void config_ref(config_t config);

void config_unref(config_t config);

#endif