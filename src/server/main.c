#include "server.h"
#include <gst/gst.h>

int main(int argc, char **argv)
{
    gst_init(NULL,NULL);
    config_t config;
    logger_t logger;
    server_t server;
    int status;
    const char *error;

    config = NULL;
    logger = NULL;
    server = NULL;

    if(argc < 2)
    {
        puts("main: no config file");
        goto error;    
    }

    if(config_create(&config,&error) != STATUS_OK)
    {
        puts(error);
        puts("main: config_create failed");
        goto error;
    }

    if(config_load(config,argv[1],&error) != STATUS_OK)
    {
        puts(error);
        puts("main: config_load failed");
        goto error;
    }

    goto done;
error:
    CLEANUP_FUNCTION(config, config_unref(config))
    CLEANUP_FUNCTION(logger, logger_unref(logger))
    
done:
    gst_deinit();
    return 0;
}