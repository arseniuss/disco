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

#ifndef __LIBDSC__PROC_H__
#define __LIBDSC__PROC_H__

#include <glib.h>

void dsc_proc_init(int argc, char *argv[], char *envp[]);

void dsc_proc_title_set(const char *fmt, ...);

typedef struct {
    const void *process_args;
} ProcArgs;

typedef void (*dsc_proc_func_t)(const ProcArgs *args);

gboolean dsc_proc_start(dsc_proc_func_t func, const void *args);

#endif /* __LIBDSC__PROC_H__ */
