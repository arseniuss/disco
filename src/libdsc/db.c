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

#include "libdsc.h"
#include "libdsc_db.h"
#include "libdsc_log.h"

#include <glib.h>
#include <sqlite3.h>
#include <stdarg.h>

#define GQUARCK_SQLITE_ERROR (sqlite_error_quark())

struct DBConnection {
    sqlite3 *db;
    int rc;
};

struct DBQuery {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
};

struct DBRow {
    int rc;
    sqlite3_stmt *stmt;
};

GQuark sqlite_error_quark(void)
{
    return g_quark_from_static_string("sqlite-error");
}

static void set_sqlite_error(GError **error, int code, const char *msg)
{
    if (error)
        *error =
            g_error_new(GQUARCK_SQLITE_ERROR, code, "SQLite error: %s", msg);
}

gboolean dsc_db_open_create(const char *conn_str, DBCreateFunc create_func,
                            DBConnection **connection, GError **error)
{
    gint64 version = -1;
    int rc;

    DSC_TRACE_FUNC("conn_str = %s", conn_str);

    if (!g_file_test(conn_str, G_FILE_TEST_EXISTS)) {
        char *dir = g_path_get_dirname(conn_str);
        rc = g_mkdir_with_parents(dir, 0755);
        g_free(dir);

        if (rc != 0) {
            // TODO: error
            return FALSE;
        }
    }

    GError *local_error = NULL;
    sqlite3 *db = NULL;

    if ((rc = sqlite3_open(conn_str, &db)) != SQLITE_OK) {
        set_sqlite_error(error, rc, sqlite3_errmsg(db));
        return FALSE;
    }

    DBConnection *conn = g_new0(DBConnection, 1);
    DBRow row = NULL;
    DBQuery result = NULL;

    conn->db = db;

    if (dsc_db_table_column_exists(conn, "schema_migrations", "version", NULL))
        version = 0;

    if ((result = dsc_db_select1(conn, "schema_migrations", "version", &row)) !=
        NULL) {
        version = dsc_db_row_get_int64(row, 0);
        dsc_db_result_close(result);
    }

    if (create_func) {
        if (!create_func(conn, version, &local_error))
            goto err1;
    }

    if (connection)
        *connection = conn;
    else
        goto err0;

    DSC_TRACE();

    return TRUE;
err1:
    g_propagate_error(error, local_error);
err0:
    g_free(conn);
    sqlite3_close(db);

    DSC_TRACE("error = %s", (*error)->message);

    return FALSE;
}

void dsc_db_result_close(DBQuery result)
{
    sqlite3_finalize(result->stmt);
}

gboolean dsc_db_exec(DBConnection *conn, const char *sql, DBExecFlags flags,
                     GError **error)
{
    char *err = NULL;

    DSC_TRACE_FUNC("sql = %s", sql);

    if ((flags & DB_EXEC_TRANSACTION)) {
        if ((conn->rc = sqlite3_exec(conn->db, "BEGIN", NULL, NULL, &err)) !=
            SQLITE_OK) {
            goto err0;
        }
    }

    if ((conn->rc = sqlite3_exec(conn->db, sql, NULL, NULL, &err)) != SQLITE_OK)
        goto err0;

    if ((flags & DB_EXEC_TRANSACTION)) {
        if ((conn->rc = sqlite3_exec(conn->db, "COMMIT", NULL, NULL, &err)) !=
            SQLITE_OK) {
            goto err0;
        }
    }

    DSC_TRACE();

    return TRUE;
err0:
    set_sqlite_error(error, conn->rc, err);
    sqlite3_free(err);

    DSC_TRACE();

    return FALSE;
}

DBQuery dsc_db_select1(DBConnection *conn, const char *table,
                       const char *column, DBRow *row)
{
    int rc;
    DBQuery result = NULL;

    DSC_TRACE_FUNC("table = %s, column = %s", table, column);

    char *sql = g_strdup_printf("select %s from %s limit 1", column, table);

    sqlite3_stmt *stmt = NULL;

    if ((rc = sqlite3_prepare_v2(conn->db, sql, -1, &stmt, NULL)))
        goto ret;

    if ((rc = sqlite3_step(stmt)) != SQLITE_ROW)
        goto ret;

    *row = g_new0(struct DBRow, 1);
    (*row)->rc = rc;
    (*row)->stmt = stmt;

    result = g_new0(struct DBQuery, 1);
    result->stmt = stmt;
ret:
    g_free(sql);

    DSC_TRACE("result = %p", result);

    return result;
}

gboolean dsc_db_table_column_exists(DBConnection *conn, const char *table_name,
                                    const char *column_name, GError **error)
{
    DSC_TRACE_FUNC("table_name = %s, column_name = %s", table_name,
                   column_name);

    conn->rc = sqlite3_table_column_metadata(
        conn->db, NULL, table_name, column_name, NULL, NULL, NULL, NULL, NULL);

    if (conn->rc != SQLITE_OK) {
        set_sqlite_error(error, conn->rc, sqlite3_errmsg(conn->db));
    }

    DSC_TRACE("rc = %d", conn->rc);

    return (conn->rc == SQLITE_OK);
}

DBQuery dsc_db_select(DBConnection *conn, const char *sql, ...)
{
    char *sql_text = NULL;
    va_list args;
    DBQuery query = NULL;

    DSC_TRACE_FUNC("sql = %s", sql);

    va_start(args, sql);
    sql_text = g_strdup_vprintf(sql, args);
    va_end(args);

    sqlite3_stmt *stmt = NULL;

    if ((conn->rc = sqlite3_prepare_v2(conn->db, sql_text, -1, &stmt, NULL)) !=
        SQLITE_OK) {
        goto ret;
    }

    query = g_new0(struct DBQuery, 1);
    query->stmt = stmt;
    query->db = conn->db;

ret:
    g_free(sql_text);

    DSC_TRACE("%p", query);

    return query;
}

DBRow dsc_db_row_read(DBQuery query)
{
    DBRow row = NULL;

    DSC_TRACE("query = %p", query);

    if ((query->rc = sqlite3_step(query->stmt)) != SQLITE_ROW) {
        DSC_TRACE("sql error:%s", sqlite3_errmsg(query->db));
        goto ret;
    }

    row = g_new0(struct DBRow, 1);
    row->stmt = query->stmt;
    row->rc = query->rc;

ret:
    DSC_TRACE("row = %p", row);

    return row;
}

gint64 dsc_db_row_get_int64(DBRow row, int column)
{
    return sqlite3_column_int64(row->stmt, column);
}

GError *dsc_db_get_error(DBConnection *conn)
{
    GError *error = NULL;

    set_sqlite_error(&error, conn->rc, sqlite3_errmsg(conn->db));

    return error;
}
