#include "server.h"
#include <glib-unix.h>
#include <gst/rtsp-server/rtsp-server.h>

#define ERRORF(FORMAT, ...) logger_errorf(server->logger, FORMAT, __VA_ARGS__);
#define WARNF(FORMAT, ...) logger_warnf(server->logger, FORMAT, __VA_ARGS__);
#define INFOF(FORMAT, ...) logger_infof(server->logger, FORMAT, __VA_ARGS__);
#define DEBUGF(FORMAT, ...) logger_debugf(server->logger, FORMAT, __VA_ARGS__);
#define TRACEF(FORMAT, ...) logger_tracef(server->logger, FORMAT, __VA_ARGS__);

#define ERRORLN(STRING) logger_errorln(server->logger, STRING);
#define WARNLN(STRING) logger_warnln(server->logger, STRING);
#define INFOLN(STRING) logger_infoln(server->logger, STRING);
#define DEBUGLN(STRING) logger_debugln(server->logger, STRING);
#define TRACELN(STRING) logger_traceln(server->logger, STRING);

struct server_internal_s
{
    GstRTSPServer *rtsp_server;
    GMainLoop *main_loop;
};
typedef struct server_internal_s *server_internal_t;

struct mount_device_user_data_s
{
    server_t server;
    GstRTSPMountPoints *mount_points;
    gboolean has_error;
    int index;
};
typedef struct mount_device_user_data_s *mount_device_user_data_t;

static int server_internal_create(server_internal_t *server_internal, server_t server, config_t config, const char **error);

static void server_mount_device(device_t device, void *user_data);

static void server_internal_destroy(server_internal_t server_internal);

static void server_destroy(server_t server);

static void server_signal(void *user_data);

int server_create(server_t *server, config_t config, logger_t logger, const char **error)
{
    int status;
    server_t new_server;

    status = STATUS_OK;
    *server = NULL;

    // check for NULL function args
    IF_THROW(server == NULL, "server_create: null server address")
    IF_THROW(config == NULL, "server_create: null config")
    IF_THROW(logger == NULL, "server_create: null logger")

    new_server = (server_t)calloc(1, sizeof(struct server_s));
    IF_THROW(new_server == NULL, "server_create: failed to allocate server")

    new_server->ref = 1;
    new_server->config = config;
    new_server->logger = logger;
    new_server->internal = NULL;
    if (server_internal_create((server_internal_t *)&new_server->internal, new_server, config, error) != STATUS_OK)
    {
        goto error;
    }

    config_ref(config);
    logger_ref(logger);

    *server = new_server;

    goto done;
error:
    CLEANUP_FUNCTION(new_server, server_destroy(new_server))
    status = STATUS_ERROR;
done:
    return status;
}

void server_deploy(server_t server)
{
    INFOLN("server_deploy: starting deployment")
    server_internal_t server_internal;
    mount_device_user_data_t mount_device_user_data;
    GstRTSPMountPoints *mount_points;

    server_internal = (server_internal_t)server->internal;
    mount_device_user_data = NULL;
    mount_points = NULL;

    DEBUGLN("server_deploy: adding signal handlers")
    g_unix_signal_add(SIGINT, (GSourceFunc)server_signal, server);
    g_unix_signal_add(SIGTERM, (GSourceFunc)server_signal, server);

    DEBUGLN("server_deploy: mounting devices")

    mount_device_user_data = calloc(1, sizeof(struct mount_device_user_data_s));
    if (mount_device_user_data == NULL)
    {
        ERRORLN("server_deploy: failed to allocate mount_device_user_data")
        goto error;
    }

    mount_points = gst_rtsp_server_get_mount_points(server_internal->rtsp_server);
    if (mount_points == NULL)
    {
        ERRORLN("server_deploy: failed to get mount_points from rtsp_server")
        goto error;
    }

    mount_device_user_data->server = server;
    mount_device_user_data->mount_points = mount_points;

    config_iterate_devices(server->config, server_mount_device, mount_device_user_data);
    if (mount_device_user_data->has_error)
    {
        ERRORLN("server_deploy: failed to mount device(s)")
        goto error;
    }
    free(mount_device_user_data);
    g_object_unref(mount_points);

    DEBUGLN("server_deploy: attaching RTSP server")
    gst_rtsp_server_attach(server_internal->rtsp_server, NULL);

    DEBUGLN("server_deploy: starting main loop")
    g_main_loop_run(server_internal->main_loop);

    goto done;
error:
    ERRORLN("server_deploy: error(s) occurred while deploying server")
    CLEANUP(mount_device_user_data)
    CLEANUP_FUNCTION(mount_points, g_object_unref(mount_points))
done:
    INFOLN("server_deploy: server exit")
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

void server_mount_device(device_t device, void *user_data)
{
    mount_device_user_data_t mount_device_user_data;
    server_t server;
    GstRTSPMediaFactory *factory;
    char *launch_string;
    char *endpoint_string;

    mount_device_user_data = (mount_device_user_data_t)user_data;
    server = mount_device_user_data->server;
    
    launch_string = g_strdup_printf("( decodebin name=depay0 ! pulsesink device=%s )", device->name);
    endpoint_string = g_strdup_printf("/%s", device->endpoint);

    factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_transport_mode(factory, GST_RTSP_TRANSPORT_MODE_RECORD);
    gst_rtsp_media_factory_set_launch(factory, launch_string);
    gst_rtsp_media_factory_set_latency(factory,500);
    gst_rtsp_mount_points_add_factory(mount_device_user_data->mount_points, endpoint_string, factory);
    INFOF("mounted device \"%s\" at endpoint %s\n", device->name, endpoint_string)
    DEBUGF("launch string: %s\n", launch_string)
    g_free(launch_string);
    g_free(endpoint_string);
    mount_device_user_data->index++;
}

int server_internal_create(server_internal_t *server_internal, server_t server, config_t config, const char **error)
{
    int status;
    server_internal_t new_server_internal;
    GMainLoop *new_main_loop;
    GstRTSPServer *new_rtsp_server;

    status = STATUS_OK;
    new_server_internal = NULL;
    new_main_loop = NULL;
    new_rtsp_server = NULL;

    // NOTE: skipping null throws for function args as they are currenty unreachable

    new_server_internal = calloc(1, sizeof(struct server_internal_s));
    IF_THROW(new_server_internal == NULL, "server_create: server_private_create: failed to allocate server_internal")

    new_rtsp_server = gst_rtsp_server_new();
    IF_THROW(new_rtsp_server == NULL, "server_create: server_private_create: failed to allocate RTSP server")

    new_main_loop = g_main_loop_new(NULL, FALSE);
    IF_THROW(new_main_loop == NULL, "server_create: server_private_create: failed to allocate loop")

    g_object_set(new_rtsp_server, "service", config->port, NULL);

    new_server_internal->rtsp_server = new_rtsp_server;
    new_server_internal->main_loop = new_main_loop;

    *server_internal = new_server_internal;

    goto done;
error:
    CLEANUP(new_server_internal)
    CLEANUP_FUNCTION(new_rtsp_server, g_object_unref(new_rtsp_server))
    CLEANUP_FUNCTION(new_main_loop, g_main_loop_unref(new_main_loop))
    status = STATUS_ERROR;
done:
    return status;
}

void server_internal_destroy(server_internal_t server_internal)
{
    g_main_loop_unref(server_internal->main_loop);
    g_object_unref(server_internal->rtsp_server);
    free(server_internal);
}

void server_destroy(server_t server)
{
    CLEANUP_FUNCTION(server->internal, server_internal_destroy((server_internal_t)server->internal))
    CLEANUP_FUNCTION(server->config, config_unref(server->config))
    CLEANUP_FUNCTION(server->logger, logger_unref(server->logger))
    free(server);
}

void server_signal(void *user_data)
{
    server_t server;
    server = (server_t)user_data;
    INFOLN("server_signal: signal received");
    g_main_loop_quit(((server_internal_t)server->internal)->main_loop);
}