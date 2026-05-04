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

#include <errno.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gtypes.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

IPCClientContext *dsc_ipc_client_new(GSocketConnection *conn, gint clientid)
{
    IPCClientContext *client = g_new0(IPCClientContext, 1);

    client->connection = conn;
    client->socket = g_socket_connection_get_socket(conn);
    client->istream = g_io_stream_get_input_stream(G_IO_STREAM(conn));
    client->ostream = g_io_stream_get_output_stream(G_IO_STREAM(conn));
    client->connected = TRUE;
    client->conn_time = time(NULL);
    client->last_activity = time(NULL);
    client->client_id = clientid;

    return client;
}

void dsc_ipc_client_free(IPCClientContext *client)
{
    if (!client)
        return;

    if (client->connection) {
        g_io_stream_close(G_IO_STREAM(client->connection), NULL, NULL);
        g_object_unref(client->connection);
    }
    if (client->socket)
        g_object_unref(client->socket);
    g_free(client);
}

gboolean dsc_ipc_client_write(IPCClientContext *ctx, const void *data, const gsize sz, GError **error)
{
    if (!ctx || !ctx->connected || !ctx->ostream)
        return FALSE;

    gsize written = 0;
    GError *local_error;

    g_output_stream_write_all(ctx->ostream, data, sz, &written, NULL, &local_error);
    if (local_error) {
        g_propagate_error(error, local_error);

        return FALSE;
    }

    return TRUE;
}

int dsc_ipc_client_read(IPCClientContext *ctx, IPCMessage **msg, double timeout)
{
    fd_set read_fds;
    GSocket *client_socket = g_socket_connection_get_socket(ctx->connection);
    int client_fd = g_socket_get_fd(client_socket);
    struct timeval tv;

    FD_ZERO(&read_fds);
    FD_SET(client_fd, &read_fds);

    tv.tv_sec = (int)timeout;
    tv.tv_usec = (int)((timeout - tv.tv_sec) * 1000000);

    int ret = select(client_fd + 1, &read_fds, NULL, NULL, &tv);

    if (ret == 0)
        return 0; // timeout

    if (ret == -1) {
        if (errno == EINTR)
            return 0;
        return -1;
    }

    guint32 msg_id, data_len;
    gssize bytes_read;
    GError *error = NULL;

    bytes_read = g_input_stream_read(ctx->istream, &msg_id, sizeof(msg_id), NULL, &error);
    if (bytes_read != sizeof(msg_id))
        return -1;

    msg_id = g_ntohl(msg_id);

    bytes_read = g_input_stream_read(ctx->istream, &data_len, sizeof(data_len), NULL, &error);
    if (bytes_read != sizeof(data_len))
        return -1;

    data_len = g_ntohl(data_len);
    void *data = NULL;

    if (data_len > 0) {
        data = g_malloc0(data_len);
        bytes_read = g_input_stream_read(ctx->istream, &data, data_len, NULL, &error);
    }

    *msg = dsc_ipc_message_new(msg_id, data_len, data);

    ctx->last_activity = time(NULL);

    return 1;
}
