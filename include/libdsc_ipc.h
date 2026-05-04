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

#ifndef __LIBDSC__IPC_H__
#define __LIBDSC__IPC_H__

#include <gio/gio.h>
#include <glib.h>
#include <glib/gtypes.h>
#include <sys/select.h>
#include <time.h>

typedef struct {
    guint32 msg_id;
    guint32 data_len;
    void *data;
} IPCMessage;

IPCMessage *dsc_ipc_message_new(guint32 msg_id, guint32 data_len, void *data);
void dsc_ipc_message_free(IPCMessage *msg);

typedef struct IPCClientContext IPCClientContext;

struct IPCClientContext {
    GSocket *socket;
    GSocketConnection *connection;
    GInputStream *istream;
    GOutputStream *ostream;
    gboolean connected;

    time_t conn_time;
    time_t last_activity;
    gint client_id;

    IPCClientContext *next;
};

IPCClientContext *dsc_ipc_client_new(GSocketConnection *conn, gint clientid);
void dsc_ipc_client_free(IPCClientContext *ctx);

int dsc_ipc_client_read(IPCClientContext *ctx, IPCMessage **msg, double timeout);
gboolean dsc_ipc_client_write(IPCClientContext *ctx, const void *data, const gsize sz,
                              GError **error);

typedef struct {
    gboolean running;

    GSocket *socket;
    GSocketAddress *address;
    gchar *socket_path;

    IPCClientContext *clients;
    gint next_client_id;

    fd_set master_fds;
    int max_fd;
} IPCServerContext;

gboolean dsc_ipc_server_init(const char *name, IPCServerContext **ctx, GError **error);
void dsc_ipc_server_deinit(IPCServerContext *ctx);
int dsc_ipc_server_wait(IPCServerContext *ctx, IPCClientContext **client, IPCMessage **msg,
                        double timeout);

#endif /* __LIBDSC__IPC_H__ */
