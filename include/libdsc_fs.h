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

#ifndef __LIBDSC__FS_H__
#define __LIBDSC__FS_H__

#include <glib.h>

typedef gboolean (*FileProcessorFunc)(const char *file_path, const char *filename,
                                      gpointer user_data, GError **error);

typedef enum {
    TRAVERSE_FOLLOW_SYMLINKS = (1 << 0),
    TRAVERSE_NO_RECURSION = (1 << 1),
    TRAVERSE_CASE_INSENSITIVE = (1 << 3)
} TraverseFlags;

gboolean dsc_traverse_dir_full(const char *root_path, FileProcessorFunc processor,
                               gpointer user_data, TraverseFlags flags, guint max_depth,
                               GPtrArray **warnings, GError **error);

#endif /* __LIBDSC__FS_H__ */
