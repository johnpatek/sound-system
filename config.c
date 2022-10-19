#include "config.h"
#include <cjson/cJSON.h>

static void config_parse(config_t config, cJSON *json);

static void config_destroy(config_t config);

int config_create(config_t *config, const char **error)
{
    int status;

    status = 0;

    *config = (config_t)calloc(1, sizeof(struct config_s));
    if (*config == NULL)
    {
        *error = "config_create: failed to allocate config";
        goto error;
    }

    (*config)->ref = 1;
    (*config)->port = "8554";
    (*config)->log_level = INFO;
    (*config)->log_path = "stdout";
    (*config)->devices = NULL;
    (*config)->ndevices = 0;
    (*config)->json = NULL;

    goto done;
error:
    status = -1;
done:
    return status;
}

int config_load(config_t config, const char *const path, const char **error)
{
    int status;
    FILE *config_file;
    int file_size;
    void *file_buffer;
    cJSON *json;

    status = 0;
    file_buffer = NULL;
    json = NULL;

    config_file = fopen(path, "r");
    IF_THROW(config_file == NULL, "config_load: failed to open config file")

    IF_THROW(fseek(config_file, 0, SEEK_END) != 0, "config_load: failed to seek end of file")

    file_size = ftell(config_file);

    IF_THROW(fseek(config_file, 0, SEEK_SET) != 0, "config_load: failed to seek start of file")

    file_buffer = calloc(file_size, 1);
    IF_THROW(file_buffer == NULL, "config_load: failed to allocate file buffer")

    fread(file_buffer, sizeof(char), file_size, config_file);

    json = cJSON_ParseWithLength(file_buffer, file_size);
    IF_THROW(json == NULL, "config_load: failed to create json parser")

    config_parse(config, json);

    CLEANUP_FUNCTION(config->json, cJSON_Delete((cJSON *)config->json))

    config->json = (void *)json;

    goto done;
error:
    status = STATUS_ERROR;
done:
    CLEANUP_FUNCTION(config_file, fclose(config_file))
    CLEANUP(file_buffer)
    return status;
}

int config_validate(config_t config, const char **error)
{
    int status;
    int port;
    status = STATUS_OK;

    port = atoi(config->port);
    IF_THROW(port < 1 || port > UINT16_MAX, "config_validate: invalid port")

    IF_THROW(strlen(config->log_path) > PATH_MAX,"config_validate: log path exceeds max length")

    IF_THROW(config->log_level < ERROR || config->log_level > TRACE, "config_validate: invalid log level")

    goto done;
error:
    status = STATUS_ERROR;
done:
    return 0;
}

void config_iterate_devices(config_t config, device_iterator_fn device_fn, void * user_data)
{
    assert(config != NULL);
    assert(device_fn != NULL);
    int index;
    device_t device;
    for(index = 0; index < config->ndevices; index++)
    {
        device = (config->devices + index);
        device_fn(device,user_data);
    }
}

void config_ref(config_t config)
{
    config->ref++;
}

void config_unref(config_t config)
{
    assert(config != NULL);
    assert(config->ref >= 1);
    if (--config->ref == 0)
    {
        config_destroy(config);
    }
}

void config_parse(config_t config, cJSON *json)
{
    const cJSON *port;
    const cJSON *log;
    const cJSON *log_path;
    const cJSON *log_level;
    const cJSON *devices;
    const cJSON *device;
    const cJSON *device_name;
    const cJSON *device_endpoint;

    port = cJSON_GetObjectItem(json, "port");
    if (port != NULL && cJSON_IsNumber(port))
    {
        config->port = port->valuestring;
    }

    log = cJSON_GetObjectItem(json, "log");
    if (log != NULL && cJSON_IsObject(log))
    {
        log_path = cJSON_GetObjectItem(log, "path");
        if (log_path != NULL && cJSON_IsString(log_path))
        {
            config->log_path = log_path->valuestring;
        }

        log_level = cJSON_GetObjectItem(log, "level");
        if (log_level != NULL && cJSON_IsString(log_level))
        {
            if (strcmp(log_level->valuestring, "error") == 0)
            {
                config->log_level = ERROR;
            }
            else if (strcmp(log_level->valuestring, "warn") == 0)
            {
                config->log_level = WARN;
            }
            else if (strcmp(log_level->valuestring, "info") == 0)
            {
                config->log_level = INFO;
            }
            else if (strcmp(log_level->valuestring, "debug") == 0)
            {
                config->log_level = DEBUG;
            }
            else if (strcmp(log_level->valuestring, "trace") == 0)
            {
                config->log_level = TRACE;
            }
            else
            {
                config->log_level = -1;
            }
        }
    }

    devices = cJSON_GetObjectItem(json, "devices");
    cJSON_ArrayForEach(device, devices)
    {
        config->devices = realloc(
            config->devices,
            sizeof(struct config_s) * (config->ndevices + 1));
        (config->devices + config->ndevices)->name = NULL;
        (config->devices + config->ndevices)->endpoint = NULL;

        device_name = cJSON_GetObjectItem(device, "name");
        if (device_name != NULL && cJSON_IsString(device_name))
        {
            (config->devices + config->ndevices)->name = device_name->valuestring;
        }

        device_endpoint = cJSON_GetObjectItem(device, "endpoint");
        if (device_endpoint != NULL && cJSON_IsString(device_endpoint))
        {
            (config->devices + config->ndevices)->endpoint = device_endpoint->valuestring;
        }

        config->ndevices++;
    }
}

void config_destroy(config_t config)
{
    if (config->json != NULL)
    {
        cJSON_Delete((cJSON *)config->json);
    }
    if (config->devices != NULL)
    {
        free(config->devices);
    }
    free(config);
}