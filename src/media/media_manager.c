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

#include "dscmedia.h"
#include "libdsc_ipc.h"
#include "libdsc_log.h"
#include "libdsc_proc.h"

#include <unistd.h>
#include <glib.h>

static gboolean print_filename(const char *file_path, const char *filename, gpointer user_data,
                               GError **error)
{
    dsc_log_msg(DSC_LOG_INFO, "Filename %s", file_path);

    return TRUE;
}

static void media_scan(void) {}

void media_manager_main(const ProcArgs *pargs)
{
    MediaManagerArgs *args = (MediaManagerArgs *)pargs->process_args;

    IPCServerContext *srv = NULL;
    GError *error = NULL;

    dsc_log_msg(DSC_LOG_INFO, "Media manager process %d started", getpid());

    dsc_proc_title_set("disco_server/%s started", IPC_SERVICE_MEDIA_MANAGER);

    if (!dsc_ipc_server_init(IPC_SERVICE_MEDIA_MANAGER, &srv, &error)) {
        dsc_log_msg(DSC_LOG_CRIT, "failed to init %s service: %s", IPC_SERVICE_MEDIA_MANAGER,
                    error->message);
        g_error_free(error);
        g_abort();
    }

    while (srv->running) {
        IPCClientContext *client = NULL;
        IPCMessage *msg = NULL;
        int ret = dsc_ipc_server_wait(srv, &client, &msg, 1.0);

        switch (ret) {
        case 1:
            switch (msg->msg_id) {
            case IPC_MEDIA_SCAN_MSG_ID:
                break;
            }
            break;
        case 0:
            dsc_log_msg(DSC_LOG_INFO, "timeout");
            // timeout
            break;
        case -1:
            // Client disconnected
            break;
        }
    }

    dsc_ipc_server_deinit(srv);
}
