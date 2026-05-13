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

#ifndef __LIBDSC__COMMON_H__
#define __LIBDSC__COMMON_H__

#include <stdarg.h>

typedef enum {
    DC_MEDIA_SCANNER_PROC = 0,

    DC_PROC_TYPE_COUNT
} dc_process_type_t;

int vsappendf(char **str, const char *fmt, va_list args);

#endif /* __LIBDSC__COMMON_H__ */
