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

#include "libdsc_db.h"

extern const char *schema_setup_script;

GQuark media_error_quark(void)
{
    return g_quark_from_static_string("media-error");
}

gboolean media_db_schema_create(DBConnection *conn, gint64 version,
                                GError **error)
{
    GError *local_error = NULL;

    if (version == -1) {
        if (!dsc_db_exec(conn, schema_setup_script, DB_EXEC_TRANSACTION,
                         &local_error)) {
            g_propagate_error(error, local_error);
            return FALSE;
        }
    }

    


    return TRUE;
}
