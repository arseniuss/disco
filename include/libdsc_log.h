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

#ifndef __LIBDSC__LOG_H__
#define __LIBDSC__LOG_H__

typedef enum {
    DSC_LOG_INFO = 0,
    DSC_LOG_WARN,
    DSC_LOG_ERR,
    DSC_LOG_CRIT,
    DSC_LOG_DEBUG,
    DSC_LOG_TRACE
} LogLevel;

#define DSC_TRACE(fmt, ...) dsc_log_msg(DSC_LOG_TRACE, "%s() " fmt, __func__, ##__VA_ARGS__)

#define DSC_TRACE_FUNC(fmt, ...) dsc_log_msg(DSC_LOG_TRACE, "%s(" fmt ")", __func__, ##__VA_ARGS__)

int dsc_log_init(const char *filename);

void dsc_log_msg(int log_level, const char *fmt, ...);
void dsc_log_msg_now(int level, const char *fmt, ...);

void dsc_log_deinit(void);

#endif /* __LIBDSC__LOG_H__ */
