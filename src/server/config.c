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

#include "disco_server.h"
#include "internal.h"
#include "libdsc.h"
#include "libdsc_log.h"

#include <assert.h>
#include <confuse.h>
#include <glib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int validate_paths(cfg_t *cfg, cfg_opt_t *opt, const char *value,
                          void *result);

static cfg_opt_t media_opts[] = {
    CFG_STR_LIST_CB("music", NULL, CFGF_NONE, validate_paths), CFG_END()};

static cfg_opt_t cfg_opts[] = {CFG_SEC("media", media_opts, CFGF_NONE),
                               CFG_END()};

static GString *config_err = NULL;

static int validate_paths(cfg_t *cfg, cfg_opt_t *opt, const char *value,
                          void *result)
{
    struct stat st;

    if (stat(value, &st) != 0) {
        cfg_error(cfg, "directory does not exist: '%s'", value);
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        cfg_error(cfg, "not a directory: '%s'", value);
        return -1;
    }

    *(const char **)result = value;

    return 0;
}

static void config_errfunc(cfg_t *cfg, const char *fmt, va_list ap)
{
    g_string_append_vprintf(config_err, fmt, ap);
}

static int config_check(cfg_t *cfg, cfg_opt_t *opts, const char *old_path,
                        const char *section_name);

static int config_elem_check(cfg_t *cfg, cfg_opt_t *opt, const char *path,
                             int index)
{
    int total_errors = 0;

#define GET_ELEM(single, multi, index)                                         \
    (index < 0 ? single(cfg, opt->name) : multi(cfg, opt->name, index))

    switch (opt->type) {
    case CFGT_SEC: {
        cfg_t *sec = GET_ELEM(cfg_getsec, cfg_getnsec, index);

        total_errors += config_check(sec, opt->subopts, path, opt->name);
        break;
    }
    case CFGT_BOOL: {
        bool value = GET_ELEM(cfg_getbool, cfg_getnbool, index);

        DSC_TRACE("%s.%s = %s", path, opt->name, value ? "true" : "false");
        break;
    }
    case CFGT_STR: {
        char *value = GET_ELEM(cfg_getstr, cfg_getnstr, index);

        DSC_TRACE("%s.%s[%d] = %s", path, opt->name, index, value);
        break;
    }
    case CFGT_INT: {
        long value = GET_ELEM(cfg_getint, cfg_getnint, index);

        DSC_TRACE("%s.%s = %ld", path, opt->name, value);
        break;
    }
    case CFGT_FLOAT: {
        double value = GET_ELEM(cfg_getfloat, cfg_getnfloat, index);

        DSC_TRACE("%s.%s = %f", path, opt->name, value);
        break;
    }
    default:
        DSC_TRACE("Unknown option type: %d for %s", opt->type, opt->name);
        break;
    }

    return total_errors;
}

static int config_check(cfg_t *cfg, cfg_opt_t *opts, const char *old_path,
                        const char *section_name)
{
    DSC_TRACE_FUNC("%s %s", old_path, section_name);

    cfg_opt_t *opt;
    int total_errors = 0;

    if (!cfg || !opts) {
        return -1;
    }

    char path[1024] = "";

    strcat(path, DSC_NULL2STR(old_path));
    if (section_name != NULL) {
        strcat(path, ".");
        strcat(path, DSC_NULL2STR(section_name));
    }

    for (opt = opts; opt->name != NULL; opt++) {
        int count = cfg_opt_size(opt);

        for (int i = 0; i < count; i++)
            config_elem_check(cfg, opt, path, i);
    }

    DSC_TRACE("total_errors = %d", total_errors);

    return total_errors;
}

cfg_t *config_init(const char *config_file)
{
    DSC_TRACE_FUNC("%s", config_file);

    cfg_t *cfg = cfg_init(cfg_opts, CFGF_NONE);
    config_err = g_string_new("");
    cfg_set_error_function(cfg, config_errfunc);

    int ret = cfg_parse(cfg, config_file);

    if (ret != CFG_SUCCESS) {
        switch (ret) {
        case CFG_FILE_ERROR:
            g_string_printf(config_err, "file error");
            break;
        case CFG_PARSE_ERROR:
        default:
            dsc_log_msg(DSC_LOG_CRIT, "Failed to parse configuration:%s: %s",
                        config_file, config_err->str);
            exit(DSC_EXIT_PARSEERR);
        }
    }

    config_check(cfg, cfg_opts, "root", NULL);

    DSC_TRACE("ret = %p", cfg);

    return cfg;
}
