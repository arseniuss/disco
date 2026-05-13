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

noinst_LIBRARIES = libdscmedia.a

EXTRA_DIST = database.dbtmpl

BUILT_SOURCES = media_db_scripts.c
CLEANFILES = media_db_scripts.c

.PHONY: media_db_scripts.c

media_db_scripts.c: database.dbtmpl
	$(SHELL) $(top_srcdir)/scripts/gen_db.sh -v -c $< $@

libdscmedia_a_SOURCES = \
    media_db_scripts.c \
    media_db.c \
    internal.h \
    media_manager.c

libdscmedia_a_DEPENDENCIES = media_db_scripts.c

libdscmedia_a_CFLAGS = $(GLIB_CFLAGS)

IWYU_FLAGS = $(GLIB_CFLAGS)
if ENABLE_IWYU
  include $(top_srcdir)/iwyu.mk
endif
