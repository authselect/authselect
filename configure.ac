dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
m4_include([version.m4])
AC_INIT(authselect, VERSION_NUMBER)

AC_CONFIG_SRCDIR(src/cli/main.c)
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_HEADERS(config.h)

m4_ifdef([AC_USE_SYSTEM_EXTENSIONS],
         [AC_USE_SYSTEM_EXTENSIONS],
         [AC_GNU_SOURCE])

AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT

AC_PROG_CC_C99
AC_PROG_CPP
AC_LANG([C])

AC_HEADER_STDC
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.19)

AC_SUBST([RELEASE_NUMBER], m4_esyscmd(./release-version.sh RELEASE_VERSION))

AC_CONFIG_MACRO_DIRS([m4])
m4_include(src/build_macros.m4)
m4_include(src/conf_macros.m4)
m4_include(external/a2x.m4)

dnl Reqired libraries
REQUIRE_POPT

dnl Optional build dependencies - man pages generation
CHECK_ASCIIDOC_TOOLS
AM_CONDITIONAL([HAVE_MANPAGES], [test "x$A2X" != "x"])

AC_CONFIG_FILES([Makefile
                 po/Makefile.in
                 profiles/Makefile
                 rpm/authselect.spec
                 src/common/Makefile
                 src/cli/Makefile
                 src/lib/Makefile
                 src/man/Makefile
                 src/man/authselect.8.txt.in
                 src/man/authselect-profiles.5.txt.in])

AC_OUTPUT
