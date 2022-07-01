#ifndef SERVER_H
#define SERVER_H

#include "config.h"
#include "logger.h"

struct server_s
{
    int ref;
    config_t config;
    logger_t logger;
    void *private;
};

typedef struct server_s* server_t;

int server_create(server_t *server, config_t config, logger_t logger, const char ** error);

void server_deploy(server_t server);

void server_ref(server_t server);

void server_unref(server_t server);

#endif