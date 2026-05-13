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
#include "libdsc_proc.h"

#include <glib.h>
#include <glib/gtypes.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *argv_start = NULL;
static char *argv_end = NULL;

static GPtrArray *subprocesses = NULL;

static void init_proc_title(int argc, char *argv[], char *envp[])
{
    char *env_start = NULL;

    for (int i = 0; envp[i] != NULL; i++) {
        if (i == 0)
            env_start = envp[i];
    }

    if (argc > 0) {
        argv_start = argv[0];
        if (env_start != NULL) {
            argv_end = env_start + strlen(env_start);
        } else {
            argv_end = argv[argc - 1] + strlen(argv[argc - 1]);
        }
        argv_end++;
    }
}

static gboolean signal_handler(gpointer user_data)
{
    int signum = GPOINTER_TO_INT(user_data);

    dsc_log_msg_now(DSC_LOG_INFO, "Got signal");

    switch (signum) {
    case SIGTERM:
    case SIGINT:
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

static void init_subprocesses(void)
{
    setpgid(0, 0); // create process group

    subprocesses = g_ptr_array_new();

    //g_unix_signal_add(SIGTERM, signal_handler, GINT_TO_POINTER(SIGTERM));
    //g_unix_signal_add(SIGINT, signal_handler, GINT_TO_POINTER(SIGINT));
}

void dsc_proc_init(int argc, char *argv[], char *envp[])
{
    subprocesses = g_ptr_array_new();

    init_proc_title(argc, argv, envp);
    init_subprocesses();
}

void dsc_proc_title_set(const char *fmt, ...)
{
    if (argv_start == NULL || argv_end == NULL)
        return;

    memset(argv_start, 0, argv_end - argv_start);

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(argv_start, (argv_end - argv_start) - 1, fmt, args);
        va_end(args);
    }
}

static void child_watch_callback(GPid pid, gint status, gpointer user_data)
{
    // TODO: log
    g_spawn_close_pid(pid);
}

gboolean dsc_proc_start(dsc_proc_func_t func, const void *args)
{
    pid_t pid = fork();

    if (pid == 0) {
        setpgid(0, getppid()); // join parent process group

        ProcArgs pargs;

        pargs.process_args = args;

        func(&pargs);
        exit(0);
    } else if (pid > 0) {
        g_ptr_array_add(subprocesses, GINT_TO_POINTER(pid));

        return TRUE;
    } else {
        return FALSE;
    }
}
