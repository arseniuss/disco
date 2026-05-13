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

#include "config.h"
#include "disco_server.h"
#include "dscmedia.h"
#include "internal.h"
#include "libdsc_log.h"
#include "libdsc_proc.h"

#include <confuse.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static void start_processes(cfg_t *cfg)
{
    MediaManagerArgs media_manager_args = {0};

    cfg_t *media = cfg_getsec(cfg, "media");
    if (!media) {
        dsc_log_msg(DSC_LOG_INFO, "Configuration has no media section");
        exit(DSC_EXIT_PARSEERR);
    }

    media_manager_args.media_database_str = SERVER_MEDIA_DATABASE;

    media_manager_args.music_path = cfg_getstr(media, "music");
    media_manager_args.videos_path = cfg_getstr(media, "videos");
    media_manager_args.pictures_path = cfg_getstr(media, "pictures");

    dsc_proc_start(&media_manager_main, &media_manager_args);
}

void print_usage(const char *progname)
{
    printf("Usage %s [options]\n", progname);
    printf("Options:\n");
    printf("   -h, --help           Show this help\n");
}

int main(int argc, char *argv[], char *envp[])
{
    char *config_file = SERVER_CONFIG_FILE;
    char *log_file = SERVER_LOG_FILE;

    int opt;

    static struct option log_options[] = {
        {"config", required_argument, 0, 'c'}, {"help", no_argument, 0, 'h'}, {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "c:h", log_options, NULL)) != -1) {
        switch (opt) {
        case 'c':
            config_file = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        case '?':
            print_usage(argv[0]);
            return 1;
        default:
            abort();
        }
    }

    dsc_proc_init(argc, argv, envp);
    dsc_log_init(log_file);

    dsc_log_msg(DSC_LOG_INFO, "Starting Disco server ...");

    cfg_t *cfg = config_init(config_file);

    start_processes(cfg);

    while (wait(NULL) > 0)
        ;

    dsc_log_msg(DSC_LOG_INFO, "Ending Disco server ...");

    dsc_log_deinit();
    cfg_free(cfg);

    return 0;
}
