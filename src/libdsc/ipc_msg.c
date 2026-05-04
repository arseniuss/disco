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

#include "libdsc_ipc.h"

#include <glib.h>
#include <glib/gtypes.h>

IPCMessage *dsc_ipc_message_new(guint32 msg_id, guint32 data_len, void *data)
{
    IPCMessage *msg = g_new0(IPCMessage, 1);

    msg->msg_id = msg_id;
    msg->data_len = data_len;
    msg->data = data;

    return msg;
}

void dsc_ipc_message_free(IPCMessage *msg)
{
    if (msg) {
        if (msg->data)
            g_free(msg->data);
        g_free(msg);
    }
}
