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

AC_INIT([disco], [0.1], [arseniuss@arseniuss.id.lv])

AC_CONFIG_SRCDIR([src/server/main.c])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_MAINTAINER_MODE([enable])

#
# Check for build programs
#

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_SED
PKG_PROG_PKG_CONFIG


#
# Argument checks
#

AC_ARG_ENABLE([kde-client],
    [AS_HELP_STRING([--enable-kde-client], [Build KDE client])],
    [enable_kde_client=yes],
    [enable_kde_client=no])
AM_CONDITIONAL([BUILD_KDE_CLIENT], [test "$enable_kde_client" = "yes"])

AC_ARG_ENABLE([server],
    [AS_HELP_STRING([--enable-server], [Build server])],
    [enable_server=yes],
    [enable_server=no])
AM_CONDITIONAL([BUILD_SERVER], [test "$enable_server" = "yes"])

AC_ARG_ENABLE([android-client],
    [AS_HELP_STRING([--enable-android-client], [Build Android client])],
    [enable_android_client=yes],
    [enable_android_client=no])
AM_CONDITIONAL([BUILD_ANDROID_CLIENT], [test "$enable_android_client" = "yes"])

AM_CONDITIONAL([BUILD_NATIVE_DEPS], [test "$enable_server" = "yes" || test "$enable_kde_client" = "yes"])

#
# General tests to build native programs
#
AS_IF([test "$enable_server" = "yes" || test "$enable_kde_client" = "yes"], [
    #
    #   Check for headers
    #

    AC_CHECK_HEADERS([stdarg.h stdio.h stdlib.h string.h stdbool.h], [],
        [AC_MSG_ERROR([Required header not found])], [])
    AC_CHECK_HEADERS([fcntl.h unistd.h], [],
        [AC_MSG_ERROR([Required header not found])], [])
    AC_CHECK_HEADERS([sys/file.h], [],
        [AC_MSG_ERROR([Required header not found])], [])

    #
    #   External libraries
    #

    PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.40])
    PKG_CHECK_MODULES([GIO], [gio-2.0 >= 2.40])

    PKG_CHECK_MODULES([CONFUSE], [libconfuse], [
        AC_SUBST([CONFUSE_CFLAGS])
        AC_SUBST([CONFUSE_LIBS])
    ], [
        AC_MSG_ERROR([libconfuse not found])
    ])

    PKG_CHECK_MODULES([LIBEVENT], [libevent >= 2.0], [
        AC_SUBST([LIBEVENT_CFLAGS])
        AC_SUBST([LIBEVENT_LIBS])
    ], [
        AC_MSG_ERROR([libevent 2.0 or higher is required but was not found.])
    ])

    #
    # Check for build tools
    #

    m4_include([m4/iwyu.m4])
    AX_IWYU([], [AC_MSG_WARN([IWYU not enabled or not found])])

    #
    # Check compiler
    #
    AX_PRINTF_FORMAT_ATTRIBUTE
])
AM_CONDITIONAL([ENABLE_IWYU], [test "x$ax_iwyu_enable" = "xyes"])

#
#
#

AS_IF([test "$enable_server" = "yes" || test "$enable_kde_client" = "yes" || test "$enable_android_client" = "yes"], [

    PKG_CHECK_MODULES([PROTOBUF], [protobuf >= 3.0])

])

#
# KDE client specific app
#
AS_IF([test "$enable_kde_client" = "yes"], [
    true
])

#
# Server specific stuff
#

AS_IF([test "$enable_server" = "yes"], [
    AC_CHECK_HEADERS([getopt.h], [],
        [AC_MSG_ERROR([Required header not found])], [])


    #
    # Configuration
    #
    sysconf_dir_expanded=$(eval echo ${sysconfdir})
    AC_DEFINE_UNQUOTED([SERVER_CONFIG_FILE], ["${sysconf_dir_expanded}/disco_server.conf"],
        [Disco server configuration file])

    #
    # Detect configuration
    #

    AC_ARG_WITH([music-dir],
        [AS_HELP_STRING([--with-music-dir=DIR],
            [Music directory path [default=auto-detect]])],
        [MUSIC_DIR="$withval"],
        [MUSIC_DIR="auto"])

    AS_IF([test "x$MUSIC_DIR" = "xauto"], [
        AS_IF([test -f "$HOME/.config/user-dirs.dirs"], [
            MUSIC_DIR=$(grep "XDG_MUSIC_DIR" "$HOME/.config/user-dirs.dirs" | cut -d '"' -f 2)
        ])
        AS_IF([test -z "$MUSIC_DIR"], [
            MUSIC_DIR="$HOME/Music"
        ])
    ])

    MUSIC_DIR=$(eval echo "$MUSIC_DIR")
    AC_SUBST([MUSIC_DIR])
])

#
# Android specific stuff
#

AS_IF([test "$enable_android_client" = "yes"], [
    #
    # Check for programs
    #

    AC_CHECK_PROGS([JAVA], [java], [no])
    AC_CHECK_PROGS([JAVAC], [javac], [no])

    AC_CHECK_PROGS([GRADLE], [gradle], [no])
    AC_MSG_CHECKING([for local gradlew])
    AS_IF([test -f "./gradlew"], [
        GRADLE="./gradlew"
        AC_MSG_RESULT([./gradlew])
    ], [test "$GRADLE" != "no"], [
        AC_MSG_RESULT([found system gradle])
    ], [
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([Gradle or gradlew required])
    ])
    AC_SUBST([GRADLE])

    AC_ARG_WITH([android-sdk],
        [AS_HELP_STRING([--with-android-sdk=DIR], [Path to Android SDK])],
        [ANDROID_SDK="$withval"],
        [ANDROID_SDK="$HOME/Android/Sdk"])

    AC_SUBST([ANDROID_SDK])
])

AC_CONFIG_FILES([
 include/Makefile
 Makefile
 src/libdsc/Makefile
 src/media/Makefile
 src/Makefile
 src/server/Makefile
 conf/disco_server.conf:conf/disco_server.conf.in
])

AC_OUTPUT
