/*
 * Copyright (C) 2026 Armands Arseniuss Skolmeisters
 *
 * This file is part of Disco project.
 *
 * Disco project is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Disco project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Disco project.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "libdsc_log.h"

#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <time.h>
#include <unistd.h>

static int log_initialized = 0;
static int min_log_level = DSC_LOG_TRACE;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static char *log_filename = NULL;
static int log_fd = -1;
static FILE *log_file = NULL;

int dsc_log_init(const char *filename)
{
    pthread_mutex_lock(&log_mutex);

    if (log_initialized) {
        pthread_mutex_unlock(&log_mutex);
        return 0;
    }

    if (!(log_filename = strdup(filename)))
        goto err0;

    if ((log_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644)) < 0)
        goto err1;

    if (!(log_file = fdopen(log_fd, "a")))
        goto err2;

    setvbuf(log_file, NULL, _IOLBF, 0);

    log_initialized = 1;

    pthread_mutex_unlock(&log_mutex);

    return 0;
err2:
    close(log_fd);
err1:
    free(log_filename);
    log_filename = NULL;
err0:
    pthread_mutex_unlock(&log_mutex);
    return -1;
}

static int log_msg(char *buf, size_t bufsz, int level, const char *fmt, va_list args)
{
    int offset =
        snprintf(buf, bufsz - 1, "%ld.%lld.%d ", (long)getpid(), (long long)time(NULL), level);

    offset += vsnprintf(buf + offset, bufsz - offset, fmt, args);

    buf[offset] = '\n';
    buf[++offset] = '\0';

    return offset;
}

void dsc_log_msg_now(int level, const char *fmt, ...)
{
    char msg[BUFSIZ];
    va_list args;

    va_start(args, fmt);
    int sz = log_msg(msg, sizeof(msg), level, fmt, args);
    va_end(args);

    fprintf(log_file, "%s", msg);
    fflush(log_file);
}

void dsc_log_msg(int level, const char *fmt, ...)
{
    if (!log_initialized || level > min_log_level)
        return;

    char msg[BUFSIZ];
    va_list args;

    va_start(args, fmt);
    int sz = log_msg(msg, sizeof(msg), level, fmt, args);
    va_end(args);

    pthread_mutex_lock(&log_mutex);

    flock(log_fd, LOCK_EX);

    fprintf(log_file, "%s", msg);
    fflush(log_file);

    flock(log_fd, LOCK_UN);

    pthread_mutex_unlock(&log_mutex);
}

void dsc_log_deinit(void)
{
    pthread_mutex_lock(&log_mutex);

    if (log_initialized) {
        if (log_file) {
            fflush(log_file);
            fclose(log_file);
            log_file = NULL;
        }

        if (log_filename) {
            free(log_filename);
            log_filename = NULL;
        }

        log_fd = -1;
        log_initialized = 0;
    }

    pthread_mutex_unlock(&log_mutex);
}
