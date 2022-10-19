#ifndef LOGGER_H
#define LOGGER_H
#include "common.h"

int logger_create(logger_t *logger, int level, FILE *file, close_file_fn close_fn, const char ** error);

int logger_create_from_config(logger_t *logger, config_t config, const char ** error);

void logger_ref(logger_t logger);

void logger_unref(logger_t logger);

void logger_errorf(logger_t logger, const char *format, ...);

void logger_errorln(logger_t logger, const char *const message);

void logger_warnf(logger_t logger, const char *format, ...);

void logger_warnln(logger_t logger, const char *const message);

void logger_infof(logger_t logger, const char *format, ...);

void logger_infoln(logger_t logger, const char *const message);

void logger_debugf(logger_t logger, const char *format, ...);

void logger_debugln(logger_t logger, const char *const message);

void logger_tracef(logger_t logger, const char *format, ...);

void logger_traceln(logger_t logger, const char *const message);

#endif