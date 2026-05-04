#!/bin/bash
#
# bootstrap.sh - Bootstrap script for autoconf/automake projects
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

set -e

rm -rf autom4te.cache
rm -f config.cache config.log config.status
find . -name "Makefile.in" -delete

mkdir -p build-aux

aclocal -I m4
autoconf
autoheader
automake -a
automake
