#include "logger.h"
#include <pthread.h>

const char *const log_colors[] = {
    "\e[0;37m\e[41m",
    "\e[0;37m\e[43m",
    "\e[0;37m\e[44m",
    "\e[0;37m\e[42m",
    "\e[0;37m\e[45m",
};

#define LEVEL_STRING(LEVEL) #LEVEL
#define RESET_COLOR "\e[0m"

#define LOGGER_PRINT_LEVEL(LOGGER, LEVEL)                                         \
    if (LOGGER->color == 1)                                                       \
    {                                                                             \
        fprintf(LOGGER->file, "%s%s%s ", log_colors[LEVEL], #LEVEL, RESET_COLOR); \
    }                                                                             \
    else                                                                          \
    {                                                                             \
        fprintf(LOGGER->file, "%s ", #LEVEL);                                     \
    }

#define LOGGER_LOGF(LOGGER, LEVEL, FORMAT, ARGS)                     \
    pthread_mutex_t *const mutex = (pthread_mutex_t *)LOGGER->mutex; \
    if (LOGGER->level >= LEVEL)                                      \
    {                                                                \
        pthread_mutex_lock(mutex);                                   \
        LOGGER_PRINT_LEVEL(LOGGER, LEVEL)                            \
        vfprintf(LOGGER->file, FORMAT, ARGS);                        \
        pthread_mutex_unlock(mutex);                                 \
    }

#define LOGGER_LOGLN(LOGGER, LEVEL, MESSAGE)                         \
    pthread_mutex_t *const mutex = (pthread_mutex_t *)LOGGER->mutex; \
    if (LOGGER->level >= LEVEL)                                      \
    {                                                                \
        pthread_mutex_lock(mutex);                                   \
        LOGGER_PRINT_LEVEL(LOGGER, LEVEL)                            \
        fprintf(LOGGER->file, "%s\n", MESSAGE);                      \
        pthread_mutex_unlock(mutex);                                 \
    }

static void logger_destroy(logger_t logger);

int logger_create(logger_t *logger, int level, FILE *file, close_file_fn close_fn, const char **error)
{
    int status;
    logger_t new_logger;
    pthread_mutex_t *new_mutex;

    status = 0;
    new_logger = NULL;
    new_mutex = NULL;

    IF_THROW(logger == NULL, "logger_create: null logger")

    new_logger = calloc(1, sizeof(struct logger_s));
    IF_THROW(new_logger == NULL, "logger_create: failed to allocate logger")
    IF_THROW(file == NULL, "logger_create: null file");

    new_mutex = calloc(1, sizeof(pthread_mutex_t));
    IF_THROW(new_mutex == NULL, "logger_create: failed to allocate mutex")
    IF_THROW(pthread_mutex_init(new_mutex, NULL) != 0, "logger_create: failed to initialize mutex")

    new_logger->ref = 1;
    new_logger->level = level;
    new_logger->file = file;
    new_logger->close_fn = close_fn;
    new_logger->mutex = new_mutex;

    *logger = new_logger;

    goto done;
error:
    status = STATUS_ERROR;
    CLEANUP(new_mutex)
    CLEANUP(new_logger)
done:
    return status;
}

int logger_create_from_config(logger_t *logger, config_t config, const char **error)
{
    int status;
    logger_t new_logger;
    pthread_mutex_t *new_mutex;
    FILE *new_file;
    close_file_fn new_close_fn;

    status = STATUS_OK;
    new_logger = NULL;
    new_mutex = NULL;

    IF_THROW(logger == NULL, "logger_create_from_config: null logger")

    new_logger = calloc(1, sizeof(struct logger_s));
    IF_THROW(new_logger == NULL, "logger_create_from_config: failed to allocate logger")

    new_mutex = calloc(1, sizeof(pthread_mutex_t));
    IF_THROW(new_mutex == NULL, "logger_create: failed to allocate mutex")
    IF_THROW(pthread_mutex_init(new_mutex, NULL) != 0, "logger_create: failed to initialize mutex")

    new_logger->color = 1;
    if (strcmp(config->log_path, "stdout") == 0)
    {
        new_file = stdout;
        new_close_fn = NULL;
    }
    else if (strcmp(config->log_path, "stderr") == 0)
    {
        new_file = stderr;
        new_close_fn = NULL;
    }
    else
    {
        new_logger->color = 0;
        new_file = fopen(config->log_path, "a");
        new_close_fn = fclose;
    }

    IF_THROW(new_file == NULL, "logger_create: null file")

    new_logger->ref = 1;
    new_logger->file = new_file;
    new_logger->close_fn = new_close_fn;
    new_logger->level = config->log_level;
    new_logger->mutex = new_mutex;

    *logger = new_logger;

    goto done;
error:
    status = STATUS_ERROR;
    CLEANUP(new_mutex)
    CLEANUP(new_logger)
done:
    return status;
}

void logger_ref(logger_t logger)
{
    logger->ref++;
}

void logger_unref(logger_t logger)
{
    assert(logger != NULL);
    assert(logger->ref >= 1);
    if (--logger->ref == 0)
    {
        logger_destroy(logger);
    }
}

void logger_errorf(logger_t logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    LOGGER_LOGF(logger, ERROR, format, args);
    va_end(args);
}

void logger_errorln(logger_t logger, const char *const message)
{
    LOGGER_LOGLN(logger, ERROR, message)
}

void logger_warnf(logger_t logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    LOGGER_LOGF(logger, WARN, format, args);
    va_end(args);
}

void logger_warnln(logger_t logger, const char *const message)
{
    LOGGER_LOGLN(logger, WARN, message)
}

void logger_infof(logger_t logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    LOGGER_LOGF(logger, INFO, format, args);
    va_end(args);
}

void logger_infoln(logger_t logger, const char *const message)
{
    LOGGER_LOGLN(logger, INFO, message)
}

void logger_debugf(logger_t logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    LOGGER_LOGF(logger, DEBUG, format, args);
    va_end(args);
}

void logger_debugln(logger_t logger, const char *const message)
{
    LOGGER_LOGLN(logger, DEBUG, message)
}

void logger_tracef(logger_t logger, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    LOGGER_LOGF(logger, TRACE, format, args);
    va_end(args);
}

void logger_traceln(logger_t logger, const char *const message)
{
    LOGGER_LOGLN(logger, TRACE, message)
}

void logger_destroy(logger_t logger)
{
    if (logger != NULL)
    {
        if (logger->mutex != NULL)
        {
            pthread_mutex_destroy((pthread_mutex_t *)logger->mutex);
            free(logger->mutex);
        }

        if (logger->close_fn != NULL && logger->file != NULL)
        {
            logger->close_fn(logger->file);
        }
        free(logger);
    }
}
