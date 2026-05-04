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
#include "libdsc_fs.h"
#include "libdsc_log.h"

#ifdef G_OS_WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif
#include <errno.h>
#include <glib.h>
#include <stdarg.h>
#include <unistd.h>

typedef struct {
    FileProcessorFunc processor;
    gpointer user_data;
    GPtrArray *warnings;
    gboolean stop_requested;
    TraverseFlags flags;
    TraversalTracker *tracker;
    guint max_depth;
    guint current_depth;
} TraversalContext;

static void add_warning(GPtrArray **warnings, const char *format, ...)
{
    g_return_if_fail(warnings != NULL);

    if (*warnings == NULL) {
        *warnings = g_ptr_array_new_with_free_func(g_free);
    }

    va_list args;

    va_start(args, format);
    char *message = g_strdup_vprintf(format, args);
    va_end(args);

    g_ptr_array_add(*warnings, message);
    dsc_log_msg(DSC_LOG_INFO, "warning: %s", message);
}

static char *resolve_symlink(const char *path, GError **error)
{
    char *resolved = NULL;

#ifdef G_OS_WIN32
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "failed to open symlink: %s", path);

        return NULL;
    }

    char buffer[MAX_PATH];
    DWORD len = GetFinalPathNameByHandleA(hFile, buffer, MAX_PATH, FILE_NAME_NORMALIZED);
    CloseHandle(hFile);

    if (len == 0 || len >= MAX_PATH) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "failed to resolve symlink: %s", path);

        return NULL;
    }

    char *result = g_strdup(buffer);
    if (g_str_has_prefix(result, "\\\\?\\")) {
        char *trimmed = g_strdup(result + 4);
        g_free(result);
        result = trimmed;
    }
    resolved = result;
#else
    ssize_t size = 1024;
    char *buffer = g_malloc(size);

    while (1) {
        ssize_t len = readlink(path, buffer, size - 1);
        if (len == -1) {
            g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(errno),
                        "failed to read symlink: %s", path);
            g_free(buffer);

            return NULL;
        }

        if (len < size - 1) {
            buffer[len] = '\0';
            break;
        }

        size *= 2;
        buffer = g_realloc(buffer, size);
    }

    if (!g_path_is_absolute(buffer)) {
        char *dir = g_path_get_dirname(path);
        char *combined = g_build_filename(dir, buffer, NULL);
        resolved = g_canonicalize_filename(combined, NULL);
        g_free(dir);
        g_free(combined);
    } else {
        resolved = g_canonicalize_filename(buffer, NULL);
    }

    g_free(buffer);
#endif

    return resolved;
}

static gboolean is_symlink(const char *path)
{
#ifdef G_OS_WIN32
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_REPARSE_POINT));
#else
    struct stat st;
    if (lstat(path, &st) != 0)
        return FALSE;
    return S_ISLNK(st.st_mode);
#endif
}

static gboolean traverse_directory_recursive(const char *path, TraversalContext *ctx,
                                             GError **error);

static gboolean process_entry_with_symlinks(const char *full_path, const char *filename,
                                            TraversalContext *ctx, GError **error)
{
    gboolean is_dir = FALSE;
    gboolean is_link = FALSE;
    char *target_path = NULL;
    gboolean should_process = TRUE;

    is_link = is_symlink(full_path);

    if (is_link && (ctx->flags & TRAVERSE_FOLLOW_SYMLINKS)) {
        target_path = resolve_symlink(full_path, error);
        if (target_path == NULL) {
            add_warning(&ctx->warnings, "cannot resolve symlink '%s' (%s)", full_path,
                        (*error)->message);
            g_clear_error(error);

            return TRUE;
        }

        if (g_file_test(target_path, G_FILE_TEST_IS_DIR)) {
            is_dir = TRUE;

            if (tracker_has_visited(ctx->tracker, target_path)) {
                add_warning(&ctx->warnings, "skipping symlink cycle: %s -> %s", full_path,
                            target_path);
                g_free(target_path);

                return TRUE;
            }
        }
    } else {
        is_dir = g_file_test(full_path, G_FILE_TEST_IS_DIR);
        if (is_link && !(ctx->flags & TRAVERSE_FOLLOW_SYMLINKS)) {
            add_warning(&ctx->warnings, "skipping symlink: %s", full_path);
        }
    }

    if (!is_dir) {
        const char *process_path = (target_path && is_dir) ? target_path : full_path;

        should_process = ctx->processor(process_path, filename, ctx->user_data, error);
    }

    if (should_process && is_dir && ctx->current_depth < ctx->max_depth) {
        if (target_path) {
            tracker_mark_visited(ctx->tracker, target_path);
        }

        ctx->current_depth++;
        should_process =
            traverse_directory_recursive(target_path ? target_path : full_path, ctx, error);
        ctx->current_depth--;
    }

    g_free(target_path);

    return should_process;
}

static gboolean traverse_directory_recursive(const char *path, TraversalContext *ctx,
                                             GError **error)
{
    DSC_TRACE_FUNC("%s", path);

    if (ctx->max_depth > 0 && ctx->current_depth >= ctx->max_depth) {
        return TRUE;
    }

    gboolean result = TRUE;
    GError *local_error = NULL;
    const char *traverse_path = path;
    GDir *dir = g_dir_open(traverse_path, 0, &local_error);

    if (dir == NULL) {
        if (local_error->code == G_FILE_ERROR_ACCES) {
            add_warning(&ctx->warnings, "permission denied: %s", path);
            g_error_free(local_error);

            return TRUE;
        } else if (local_error->code == G_FILE_ERROR_NOENT) {
            add_warning(&ctx->warnings, "does not exist: %s", traverse_path);
            g_error_free(local_error);
            return TRUE;
        } else {
            g_propagate_error(error, local_error);
            return FALSE;
        }
    }

    const char *filename;
    while ((filename = g_dir_read_name(dir)) != NULL) {
        if (g_strcmp0(filename, ".") == 0 || g_strcmp0(filename, "..") == 0)
            continue;

        char *full_path = g_build_filename(traverse_path, filename, NULL);

        if (!process_entry_with_symlinks(full_path, filename, ctx, error)) {
            result = FALSE;
            g_free(full_path);
            break;
        }

        g_free(full_path);

        if (ctx->stop_requested) {
            break;
        }
    }

    g_dir_close(dir);

    return result;
}

gboolean dsc_traverse_dir_full(const char *root_path, FileProcessorFunc processor,
                               gpointer user_data, TraverseFlags flags, guint max_depth,
                               GPtrArray **warnings, GError **error)
{
    gboolean result;

    g_return_val_if_fail(root_path != NULL, FALSE);
    g_return_val_if_fail(processor != NULL, FALSE);

    char *canonical_root = g_canonicalize_filename(root_path, NULL);
    if (canonical_root == NULL) {
        g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "invalid path: %s", root_path);
        return FALSE;
    }

    TraversalContext ctx;

    ctx.processor = processor;
    ctx.user_data = user_data;
    ctx.warnings = NULL;
    ctx.stop_requested = FALSE;
    ctx.flags = flags;
    ctx.max_depth = max_depth;
    ctx.current_depth = 0;
    ctx.tracker = tracker_new();

    tracker_mark_visited(ctx.tracker, canonical_root);

    result = traverse_directory_recursive(root_path, &ctx, error);

    tracker_free(ctx.tracker);
    g_free(canonical_root);

    if (warnings != NULL)
        *warnings = ctx.warnings;
    else if (ctx.warnings != NULL)
        g_ptr_array_free(ctx.warnings, TRUE);

    return result;
}

gboolean dsc_traverse_dir(const char *root_path, FileProcessorFunc processor, gpointer user_data,
                          GPtrArray **warnings, GError **error)
{
    return dsc_traverse_dir_full(root_path, processor, user_data, 0, 0, warnings, error);
}
