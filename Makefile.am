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

.PHONY: android-build android-clean android-install maintainer-check license-check

SUBDIRS = \
    src

EXTRA_DIST = \
    build.gradle \
    settings.gradle \
    gradle.properties \
    gradle/wrapper/gradle-wrapper.jar \
    gradle/wrapper/gradle-wrapper.properties

if BUILD_NATIVE_DEPS

SUBDIRS += include

if ENABLE_IWYU
iwyu:
	$(MAKE) $(AM_MAKEFLAGS) iwyu-all

  include $(top_srcdir)/iwyu.mk
endif # ENABLE_IWYU
endif # BUILD_NATIVE_DEPS

#########################################################

if BUILD_ANDROID_CLIENT

all-local: android-build

android-build:
	$(GRADLE) assembleDebug

android-clean:
	$(GRADLE) clean

android-install:
	$(GRADLE) installDebug

endif # BUILD_ANDROID_CLIENT

#########################################################

maintainer-check: license-check

license-check:
	@failed=0; \
	git ls-files --cached --others --exclude-standard | while read file; do \
		case $$file in \
			*.c|*.h|*.cpp|*.hpp|*.java|*.gradle|*.xml|Makefile.am|configure.ac) \
				echo -n "checking $$file ... "; \
				if ! grep -q "GNU Affero General Public License" "$$file"; then \
					echo "nok"; \
					failed=1; \
				else \
					echo "ok"; \
				fi; \
				;; \
		esac; \
	done; \
	if test $$failed -eq 1; then exit 1; fi
