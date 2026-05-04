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

#include "internal.h"

#ifdef G_OS_WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif
#include <glib.h>
#include <errno.h>
#include <stddef.h>

static char *get_directory_id(const char *path, GError **error)
{
#ifdef G_OS_WIN32
    char *canonical = g_canonicalize_filename(path, NULL);
    if (canonical == NULL) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "failed to canonicalize path: %s",
                    path);

        return NULL;
    }

    char *lower = g_ascii_strdown(canonical, -1);
    g_free(canonical);

    return lower;
#else

    struct stat st;
    if (stat(path, &st) != 0) {
        g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(errno), "failed to stat path: %s",
                    path);

        return NULL;
    }

    return g_strdup_printf("%llu:%llu", (unsigned long long)st.st_dev,
                           (unsigned long long)st.st_ino);
#endif
}

TraversalTracker *tracker_new(void)
{
    TraversalTracker *tracker = g_new0(TraversalTracker, 1);
    tracker->visited_inodes = g_hash_table_new(g_str_hash, g_str_equal);

    return tracker;
}

gboolean tracker_has_visited(TraversalTracker *tracker, const char *path)
{
    GError *error = NULL;
    char *id = get_directory_id(path, &error);
    gboolean visited = FALSE;

    if (id == NULL) {
        g_warning("cannot get ID for %s: %s", path, error->message);
        g_error_free(error);

        return FALSE;
    }

    visited = g_hash_table_contains(tracker->visited_inodes, id);
    g_free(id);

    return visited;
}

void tracker_mark_visited(TraversalTracker *tracker, const char *path)
{
    GError *error = NULL;
    char *id = get_directory_id(path, &error);

    if (id != NULL) {
        g_hash_table_add(tracker->visited_inodes, id);
    } else {
        g_warning("Cannot mark visited for %s: %s", path, error->message);
        g_error_free(error);
    }
}

void tracker_free(TraversalTracker *tracker)
{
    if (tracker) {
        if (tracker->visited_inodes) {
            g_hash_table_destroy(tracker->visited_inodes);
        }
        g_free(tracker);
    }
}
