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

#ifndef __DSC__MEDIA_H__
#define __DSC__MEDIA_H__

#include "libdsc_proc.h"

#define IPC_SERVICE_MEDIA_MANAGER "media_manager"

#define IPC_MEDIA_SCAN_MSG_ID 1

typedef struct {
    const char *music_path;
} MediaManagerArgs;

void media_manager_main(const ProcArgs *args);

#endif /* __DSC__MEDIA_H__ */
