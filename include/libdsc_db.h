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

#ifndef __LIBDSC__DB_H__
#define __LIBDSC__DB_H__

#include <glib.h>
#include <glib/gtypes.h>

typedef enum {
    DB_COLUMN_IDENTITY = 0,
    DB_COLUMN_INT,
    DB_COLUMN_TEXT,
    DB_COLUMN_FLOAT,
    DB_COLUMN_BINARY,
    DB_COLUMN_TIMESTAMP,
    DB_COLUMN_DATE
} DBColumnType;

typedef struct {
    const char *name;
    DBColumnType column_type;
} DBColumn;

typedef struct {
    const char *name;
    const DBColumn *columns;
} DBTable;

typedef struct DBConnection DBConnection;

typedef gboolean (*DBCreateFunc)(DBConnection *conn, gint64 version,
                                 GError **error);

gboolean dsc_db_open_create(const char *conn_str, DBCreateFunc create_func,
                            DBConnection **connection, GError **error);

typedef enum {
    DB_EXEC_TRANSACTION = (1 << 0)
} DBExecFlags;

gboolean dsc_db_exec(DBConnection *conn, const char *sql, DBExecFlags flags,
                     GError **error);

typedef struct DBQuery *DBQuery;
typedef struct DBRow *DBRow;

DBQuery dsc_db_select1(DBConnection *conn, const char *table,
                       const char *column, DBRow *row);

gboolean dsc_db_table_column_exists(DBConnection *conn, const char *table_name,
                                    const char *column_name, GError **error);

DBQuery dsc_db_select(DBConnection *conn, const char *sql, ...);

DBRow dsc_db_row_read(DBQuery result);

gint64 dsc_db_row_get_int64(DBRow row, int column);

void dsc_db_result_close(DBQuery result);

GError *dsc_db_get_error(DBConnection *conn);

#endif /* __LIBDSC__DB_H__ */
