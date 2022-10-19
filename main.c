#include "server.h"

int main(int argc, char **argv)
{
    config_t config;
    logger_t logger;
    server_t server;
    const char *error;

    if (config_create(&config, &error) != STATUS_OK)
    {
        puts(error);
        goto error;
    }

    if (config_load(config, "config.json", &error) != STATUS_OK)
    {
        puts(error);
        goto error;
    }

    if (logger_create_from_config(&logger, config, &error) != STATUS_OK)
    {
        puts(error);
        goto error;
    }

    if (server_create(&server, config, logger, &error) != STATUS_OK)
    {
        puts(error);
        goto error;
    }

    config_unref(config);
    logger_unref(logger);

    server_deploy(server);

    goto done;
error:
    CLEANUP_FUNCTION(config, config_unref(config))
    CLEANUP_FUNCTION(logger, logger_unref(logger))
done:
    return 0;
}