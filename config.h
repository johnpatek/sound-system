#ifndef CONFIG_H
#define CONFIG_H
#include "common.h"

int config_create(config_t *config, const char ** error);

int config_load(config_t config, const char * const path, const char ** error);

int config_validate(config_t config, const char ** error);

void config_ref(config_t config);

void config_unref(config_t config);

#endif