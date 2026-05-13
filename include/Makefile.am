#
# Copyright (C) 2026 Armands Arseniuss Skolmeisters
#
# This file is part of Disco project.
#
# Disco project is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Disco project is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with Disco project.  If not, see <https://www.gnu.org/licenses/>.

noinst_HEADERS = \
  disco_server.h \
  dscmedia.h \
  libdsc_db.h \
  libdsc_fs.h \
  libdsc_ipc.h \
  libdsc_log.h \
  libdsc_proc.h \
  libdsccommon.h

iwyu_sources = $(noinst_HEADERS)

IWYU_FLAGS = $(GLIB_CFLAGS)

if ENABLE_IWYU
  include $(top_srcdir)/iwyu.mk
endif
