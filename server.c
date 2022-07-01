#include "server.h"
#include <glib-unix.h>
#include <gst/rtsp-server/rtsp-server.h>

struct server_private_s
{
    GstRTSPServer *rtsp_server;

    GMainLoop *loop;
};

typedef struct server_private_s *server_private_t;

static int server_private_create(server_private_t *server_private, config_t config, const char **error);

static void server_private_destroy(server_private_t server_private);

static void server_destroy(server_t server);

static void server_signal(void *user_data);

int server_create(server_t *server, config_t config, logger_t logger, const char **error)
{
    int status;
    const char *error_message;

    status = 0;
    *server = NULL;

    if (server == NULL)
    {
        error_message = "server_create: null server address";
        goto error;
    }

    if (config == NULL)
    {
        error_message = "server_create: null config";
        goto error;
    }

    if (logger == NULL)
    {
        error_message = "server_create: null logger";
        goto error;
    }

    *server = (server_t)calloc(1, sizeof(struct server_s));
    if (*server == NULL)
    {
        error_message = "server_create: failed to allocate server";
        goto error;
    }

    config_ref(config);
    logger_ref(logger);
    (*server)->ref = 1;
    (*server)->config = config;
    (*server)->logger = logger;
    (*server)->private = NULL;

    if (server_private_create((server_private_t *)&(*server)->private, config, &error_message) != 0)
    {
        goto error;
    }

    goto done;
error:
    if (*server != NULL)
    {
        server_destroy(*server);
    }

    if (error != NULL)
    {
        *error = error_message;
    }
    status = -1;
done:
    return status;
}

void server_deploy(server_t server)
{
    GstRTSPMountPoints *mount_points;
    device_t device;
    int index;

    g_unix_signal_add(SIGINT, (GSourceFunc)server_signal, server);
    g_unix_signal_add(SIGTERM, (GSourceFunc)server_signal, server);

    // mount_points = gst_rtsp_server_get_mount_points(((server_private_t)server->private)->rtsp_server);
    for (index = 0; index < server->config->ndevices; index++)
    {
        device = server->config->devices + (index * sizeof(device_t));
        logger_infof(server->logger, "mounting device \"%s\" at endpoint /%s\n", device->name, device->endpoint);
        //goto done;
    }

done:
    logger_infoln(server->logger, "server_deploy: server shutting down");
}

void server_ref(server_t server)
{
    server->ref++;
}

void server_unref(server_t server)
{
    if (--server->ref == 0)
    {
        server_destroy(server);
    }
}

int server_private_create(server_private_t *server_private, config_t config, const char **error)
{
    int status;
    const char *error_message;
    server_private_t new_server_private;

    status = 0;

    new_server_private = calloc(1, sizeof(struct server_private_s));
    if (new_server_private == NULL)
    {
        error_message = "server_create: failed to allocate private struct";
        goto error;
    }

    new_server_private->rtsp_server = gst_rtsp_server_new();
    if (new_server_private->rtsp_server == NULL)
    {
        error_message = "server_create: gst_rtsp_server_new() failed";
        goto error;
    }

    g_object_set(new_server_private->rtsp_server, "service", config->port, NULL);

    new_server_private->loop = g_main_loop_new(NULL,FALSE);

    *server_private = new_server_private;
    goto done;
error:
    *error = error_message;
    status = -1;
    if (new_server_private != NULL)
    {
        free(new_server_private);
    }
done:
    return status;
}

void server_private_destroy(server_private_t server)
{
    g_main_loop_unref(server->loop);
}

void server_destroy(server_t server)
{
    if (server->private != NULL)
    {
        server_private_destroy((server_private_t)server->private);
    }

    if (server->config != NULL)
    {
        config_unref(server->config);
    }

    if (server->logger != NULL)
    {
        logger_unref(server->logger);
    }
    free(server);
}

void server_signal(void *user_data)
{
    server_t server;
    server = (server_t)user_data;
    logger_infoln(server->logger, "server_signal: signal received");
    g_main_loop_quit(((server_private_t)server->private)->loop);
}