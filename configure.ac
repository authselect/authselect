dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(authselect, 0.1)

AC_CONFIG_SRCDIR(src/cli/main.c)
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_HEADERS(config.h)

m4_ifdef([AC_USE_SYSTEM_EXTENSIONS],
         [AC_USE_SYSTEM_EXTENSIONS],
         [AC_GNU_SOURCE])

AM_INIT_AUTOMAKE([subdir-objects])
LT_INIT

AC_PROG_CC_C99
AC_PROG_CPP
AC_LANG([C])

AC_HEADER_STDC
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.19)

AC_CONFIG_MACRO_DIRS([m4])
m4_include(src/build_macros.m4)
m4_include(src/conf_macros.m4)

dnl Reqired libraries
REQUIRE_POPT

AC_CONFIG_FILES([Makefile
                 po/Makefile.in
                 src/cli/Makefile
                 src/lib/Makefile])

AC_OUTPUT
