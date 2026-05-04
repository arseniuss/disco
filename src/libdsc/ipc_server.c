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
#include "libdsc_log.h"

#include "gio/gunixsocketaddress.h"
#include <errno.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

gboolean dsc_ipc_server_init(const char *name, IPCServerContext **srv, GError **error)
{
    DSC_TRACE_FUNC("name = %s", name);

    g_return_val_if_fail(srv != NULL, FALSE);

    GError *local_error = NULL;
    IPCServerContext *ctx = g_new0(IPCServerContext, 1);

    ctx->running = TRUE;
    ctx->max_fd = 0;
    FD_ZERO(&ctx->master_fds);

    ctx->socket_path = g_strdup_printf("/tmp/disco_%s.socket", name);
    unlink(ctx->socket_path);

    ctx->socket = g_socket_new(G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM,
                               G_SOCKET_PROTOCOL_DEFAULT, &local_error);
    if (!ctx->socket)
        goto err0;

    ctx->address = g_unix_socket_address_new(ctx->socket_path);
    if (!g_socket_bind(ctx->socket, ctx->address, TRUE, &local_error))
        goto err1;

    if (!g_socket_listen(ctx->socket, &local_error))
        goto err1;

    g_socket_set_blocking(ctx->socket, FALSE);

    int listen_fd = g_socket_get_fd(ctx->socket);
    FD_SET(listen_fd, &ctx->master_fds);
    ctx->max_fd = listen_fd;

    *srv = ctx;

    DSC_TRACE("TRUE");

    return TRUE;

err1:
    g_object_unref(ctx->socket);
err0:
    g_free(ctx->socket_path);
    g_free(ctx);

    g_propagate_error(error, local_error);

    DSC_TRACE("TRUE");

    return FALSE;
}

void dsc_ipc_server_deinit(IPCServerContext *ctx)
{
    if (!ctx)
        return;

    IPCClientContext *client = ctx->clients;
    while (client) {
        IPCClientContext *next = client->next;
        dsc_ipc_client_free(client);
        client = next;
    }

    if (ctx->socket)
        g_object_unref(ctx->socket);
    if (ctx->address)
        g_object_unref(ctx->address);
    if (ctx->socket_path) {
        unlink(ctx->socket_path);
        g_free(ctx->socket_path);
    }

    g_free(ctx);
}

static void add_client(IPCServerContext *ctx, IPCClientContext *client)
{
    client->next = ctx->clients;
    ctx->clients = client;

    int fd = g_socket_get_fd(client->socket);
    FD_SET(fd, &ctx->master_fds);
    if (fd > ctx->max_fd)
        ctx->max_fd = fd;
}

static void remove_client(IPCServerContext *ctx, IPCClientContext *client)
{
    IPCClientContext **curr = &ctx->clients;

    while (*curr) {
        if (*curr == client) {
            *curr = client->next;
            break;
        }
        curr = &(*curr)->next;
    }

    int fd = g_socket_get_fd(client->socket);
    FD_CLR(fd, &ctx->master_fds);

    if (fd == ctx->max_fd) {
        ctx->max_fd = g_socket_get_fd(ctx->socket);

        IPCClientContext *c = ctx->clients;

        while (c) {
            int f = g_socket_get_fd(c->socket);

            if (f > ctx->max_fd)
                ctx->max_fd = f;
            c = c->next;
        }
    }
}

static IPCClientContext *accept_new_client(IPCServerContext *ctx)
{
    GError *error = NULL;

    GSocket *client_socket = g_socket_accept(ctx->socket, NULL, &error);

    if (!client_socket) {
        if (error && error->code != G_IO_ERROR_WOULD_BLOCK) {
            g_printerr("accept failed: %s", error->message);
            g_error_free(error);
        } else if (error) {
            g_error_free(error);
        }

        return NULL;
    }

    GSocketConnection *conn = g_socket_connection_factory_create_connection(client_socket);

    IPCClientContext *client = dsc_ipc_client_new(conn, ctx->next_client_id++);

    add_client(ctx, client);

    return client;
}

int dsc_ipc_server_wait(IPCServerContext *ctx, IPCClientContext **client, IPCMessage **msg,
                        double timeout)
{
    struct timeval tv;

    if (timeout < 0) {
        tv.tv_sec = 60;
        tv.tv_usec = 0;
    } else if (timeout == 0) {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    } else {
        tv.tv_sec = (int)timeout;
        tv.tv_usec = (int)((timeout - tv.tv_sec) * 1000000);
    }

    fd_set read_fds = ctx->master_fds;

    int ret = select(ctx->max_fd + 1, &read_fds, NULL, NULL, &tv);

    if (ret == 0) {
        *client = NULL;
        *msg = NULL;

        return 0;
    }

    if (ret < 0) {
        *client = NULL;
        *msg = NULL;

        if (errno == EINTR) {
            return 0;
        }

        return -1;
    }

    int listen_fd = g_socket_get_fd(ctx->socket);
    if (FD_ISSET(listen_fd, &read_fds)) {
        accept_new_client(ctx);
    }

    IPCClientContext *current = ctx->clients;
    IPCClientContext *prev = NULL;

    while (current) {
        int client_fd = g_socket_get_fd(current->socket);

        if (FD_ISSET(client_fd, &read_fds)) {
            IPCMessage *recv_msg = NULL;
            int result = dsc_ipc_client_read(current, &recv_msg, 0);

            if (result == 1) {
                *client = current;
                *msg = recv_msg;

                return 1;
            } else if (result == -1) {
                IPCClientContext *to_remove = current;

                current = current->next;
                if (prev) {
                    prev->next = current;
                } else {
                    ctx->clients = current;
                }

                FD_CLR(client_fd, &ctx->master_fds);

                if (client_fd == ctx->max_fd) {
                    ctx->max_fd = g_socket_get_fd(ctx->socket);
                    IPCClientContext *c = ctx->clients;

                    while (c) {
                        int fd = g_socket_get_fd(c->socket);
                        if (fd > ctx->max_fd)
                            ctx->max_fd = fd;
                        c = c->next;
                    }
                }

                // TODO
                remove_client(ctx, to_remove);

                return -1;
            }
        }

        prev = current;
        current = current->next;
    }

    *client = NULL;
    *msg = NULL;

    return 0;
}
