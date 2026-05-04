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

#include "libdsccommon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int vsappendf(char **str, const char *fmt, va_list args)
{
    int ret;
    char buf[BUFSIZ];
    size_t old_size = *str ? strlen(*str) : 0;

    if ((ret = vsnprintf(buf, sizeof(buf), fmt, args)) < 0) {
        return ret;
    }

    char *new_str = realloc(*str, old_size + ret + 1);

    strcat(new_str, buf);

    *str = new_str;

    return old_size + ret + 1;
}
