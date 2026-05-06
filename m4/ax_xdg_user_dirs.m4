# ============================================================================
# AX_XDG_DIR - Detect XDG User Directories
# ============================================================================
#
# SYNOPSIS
#
#   AX_XDG_DIR(VAR_NAME, XDG_NAME, DEFAULT_SUBDIR)
#
# DESCRIPTION
#
#   Detects XDG user directories from ~/.config/user-dirs.dirs and provides
#   fallback paths. Creates command-line option --with-VAR_NAME to override.
#
#   Example usage:
#     AX_XDG_DIR([music_dir], [XDG_MUSIC_DIR], [Music])
#
# ============================================================================

#serial 2

AC_DEFUN([AX_XDG_DIR], [
    AC_ARG_WITH([$1],
        [AS_HELP_STRING([--with-$1=DIR], [Directory for $2 @<:@default=auto@:>@])],
        [],
        [with_$1=auto])

    AS_IF([test "x$with_$1" = "xauto"], [
        AC_MSG_CHECKING([for $2 directory])
        ax_xdg_file="$HOME/.config/user-dirs.dirs"
        ax_xdg_val=""
        AS_IF([test -f "$ax_xdg_file"], [
            ax_xdg_val=$(grep "^$2=" "$ax_xdg_file" 2>/dev/null | head -1 | cut -d'=' -f2- | tr -d '"' | tr -d "'")
        ])
        AS_IF([test -z "$ax_xdg_val"], [
            ax_xdg_val="$$HOME/$3"
        ])
        eval $1="$ax_xdg_val"
        AC_MSG_RESULT([$$1])
    ], [
        $1="$with_$1"
    ])

    AC_DEFINE_UNQUOTED(m4_toupper([$1]), ["$$1"], [Path to $2])
    AC_SUBST([$1])
    AC_SUBST(m4_toupper([$1]), [$$1])
])
