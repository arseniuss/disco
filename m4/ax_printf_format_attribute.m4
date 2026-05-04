# ============================================================================
#  AX_PRINTF_FORMAT_ATTRIBUTE
# ============================================================================
#
# SYNOPSIS
#
#   AX_PRINTF_FORMAT_ATTRIBUTE
#
# DESCRIPTION
#
#   Check for printf format attribute support. Sets:
#     HAVE_FORMAT_ATTRIBUTE - if attribute is supported
#     PRINTF_FORMAT_ATTRIBUTE - macro to use in code (expands to
#                               __attribute__((format(printf, a, b))) or nothing)
#
#   Also provides PRINTF_FORMAT_ATTRIBUTE_MSVC for MSVC's _Printf_format_string_
#   if available.
#
# LICENSE
#
#   This macro is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.

#serial 1

AC_DEFUN([AX_PRINTF_FORMAT_ATTRIBUTE], [
    AC_REQUIRE([AC_PROG_CC])

    AC_CACHE_CHECK([for printf format attribute],
                   [ax_cv_printf_format_attribute], [
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            __attribute__((format(printf, 1, 2)))
            int test(const char *fmt, ...);
        ]], [[
            test("%s", "test");
        ]])],
        [ax_cv_printf_format_attribute=yes],
        [ax_cv_printf_format_attribute=no])
    ])

    AC_CACHE_CHECK([for __has_attribute(format) support],
                   [ax_cv_has_attribute_format], [
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
            #ifndef __has_attribute
            #error __has_attribute not defined
            #endif
            #if !__has_attribute(format)
            #error __has_attribute(format) false
            #endif
            int main(void) { return 0; }
        ]])],
        [ax_cv_has_attribute_format=yes],
        [ax_cv_has_attribute_format=no])
    ])

    # Define the PRINTF_ATTR macro
    AS_IF([test "x$ax_cv_printf_format_attribute" = "xyes"], [
        AC_DEFINE([HAVE_FORMAT_ATTRIBUTE], [1],
                  [Define if format attribute is supported])

        AC_DEFINE([PRINTF_ATTR(a, b)],
                  [__attribute__((format(printf, a, b)))],
                  [Define to printf attribute macro if supported])
    ], [
        AC_DEFINE([PRINTF_ATTR(a, b)], [],
                  [Define to printf attribute macro (empty if not supported)])
    ])

    # Check for MSVC annotation
    AC_CACHE_CHECK([for MSVC _Printf_format_string_],
                   [ax_cv_msvc_printf_format], [
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            #include <stdarg.h>
            void test(_Printf_format_string_ const char *fmt, ...);
        ]], [[
            test("%s", "test");
        ]])],
        [ax_cv_msvc_printf_format=yes],
        [ax_cv_msvc_printf_format=no])
    ])

    AS_IF([test "x$ax_cv_msvc_printf_format" = "xyes"], [
        AC_DEFINE([HAVE_MSVC_PRINTF_FORMAT], [1],
                  [Define if MSVC _Printf_format_string_ is supported])
        AC_DEFINE([PRINTF_FORMAT_MSVC], [_Printf_format_string_],
                  [Define to MSVC format annotation if supported])
    ], [
        AC_DEFINE([PRINTF_FORMAT_MSVC], [],
                  [Define to MSVC format annotation (empty if not supported)])
    ])
])