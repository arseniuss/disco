# ===========================================================================
#          IWYU - Include What You Use Integration for Autoconf
# ===========================================================================
#
# SYNOPSIS
#
#   AX_IWYU([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   This macro checks for IWYU (Include What You Use) and provides
#   configuration options for integrating it into the build system.
#
#   Options:
#     --enable-iwyu        Enable IWYU checking (default: no)
#     --with-iwyu-mappings Specify IWYU mapping file
#     --with-iwyu-flags    Additional IWYU flags
#
#   Variables:
#     IWYU                 Path to IWYU binary
#     IWYU_FLAGS           IWYU flags to use
#     IWYU_MAPPINGS        Mapping file path
#
#   Automake conditionals:
#     ENABLE_IWYU          True if IWYU is enabled
#

#serial 1

AC_DEFUN([AX_IWYU], [
    AC_REQUIRE([AC_PROG_SED])

    # Command line options
    AC_ARG_ENABLE([iwyu],
        [AS_HELP_STRING([--enable-iwyu],
            [enable include-what-you-use checks @<:@default=no@:>@])],
        [ax_iwyu_enable=$enableval],
        [ax_iwyu_enable=no])

    AC_ARG_WITH([iwyu-mappings],
        [AS_HELP_STRING([--with-iwyu-mappings=FILE],
            [mapping file for IWYU @<:@default=.iwyu_mappings@:>@])],
        [ax_iwyu_mappings=$withval],
        [ax_iwyu_mappings=".iwyu_mappings"])

    AC_ARG_WITH([iwyu-flags],
        [AS_HELP_STRING([--with-iwyu-flags=FLAGS],
            [additional flags for IWYU @<:@optional@:>@])],
        [ax_iwyu_extra_flags=$withval],
        [ax_iwyu_extra_flags=""])

    # Save original values
    ax_iwyu_save_CPPFLAGS="$CPPFLAGS"
    ax_iwyu_save_CFLAGS="$CFLAGS"
    ax_iwyu_save_CXXFLAGS="$CXXFLAGS"

    # Check if IWYU is enabled
    AS_IF([test "x$ax_iwyu_enable" = "xyes"], [
        # Find IWYU binary
        AC_CHECK_PROGS([IWYU], [include-what-you-use iwyu], [no])

        AS_IF([test "x$IWYU" = "xno"], [
            AC_MSG_WARN([IWYU requested but not found])
            ax_iwyu_enable=no
        ], [
             ax_iwyu_enable=yes
        ])
    ])

    # Setup IWYU flags
    AS_IF([test "x$ax_iwyu_enable" = "xyes"], [
        # Base IWYU flags
        IWYU_FLAGS=""

        # Add mapping file if it exists
        AS_IF([test -f "$ax_iwyu_mappings"], [
            IWYU_FLAGS="$IWYU_FLAGS -Xiwyu --mapping_file=$ax_iwyu_mappings"
        ], [test "x$ax_iwyu_mappings" != "x.iwyu_mappings"], [
            AS_IF([test -f "$ax_iwyu_mappings"], [
                IWYU_FLAGS="$IWYU_FLAGS -Xiwyu --mapping_file=$ax_iwyu_mappings"
            ], [
                AC_MSG_WARN([IWYU mapping file $ax_iwyu_mappings not found])
            ])
        ])

        # Common IWYU flags
        IWYU_FLAGS="$IWYU_FLAGS -Xiwyu --verbose=3"
        IWYU_FLAGS="$IWYU_FLAGS -Xiwyu --max_line_length=100"
        IWYU_FLAGS="$IWYU_FLAGS -Xiwyu --no_fwd_decls"

        # Add extra flags if specified
        AS_IF([test "x$ax_iwyu_extra_flags" != "x"], [
            IWYU_FLAGS="$IWYU_FLAGS $ax_iwyu_extra_flags"
        ])

        # Export IWYU variables
        AC_SUBST([IWYU])
        AC_SUBST([IWYU_FLAGS])
        AC_SUBST([IWYU_MAPPINGS], [$ax_iwyu_mappings])

        # Create Automake conditionals
        AM_CONDITIONAL([ENABLE_IWYU], [true])

        # Provide action if found
        AS_IF([test "x$1" != "x"], [$1])
    ], [
        # IWYU not enabled or not found
        AM_CONDITIONAL([ENABLE_IWYU], [false])

        # Provide action if not found
        AS_IF([test "x$2" != "x"], [$2])
    ])

    # Restore original values
    CPPFLAGS="$ax_iwyu_save_CPPFLAGS"
    CFLAGS="$ax_iwyu_save_CFLAGS"
    CXXFLAGS="$ax_iwyu_save_CXXFLAGS"
])
