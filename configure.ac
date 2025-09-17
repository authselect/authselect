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

AC_PROG_CC
AC_PROG_MKDIR_P
AC_PROG_LN_S
AC_PROG_SED
AC_LANG([C])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.19)

AC_SUBST([RELEASE_NUMBER], m4_esyscmd(./scripts/release-version.sh RELEASE_VERSION))

AC_CONFIG_MACRO_DIRS([m4])
m4_include(src/build_macros.m4)
m4_include(src/conf_macros.m4)
m4_include(external/a2x.m4)
m4_include(external/po4a.m4)

dnl Check if functions are present
AC_CHECK_FUNCS_ONCE([reallocarray])

dnl Required libraries
REQUIRE_POPT
REQUIRE_CMOCKA
REQUIRE_SELINUX

dnl Optional build dependencies - man pages generation
CHECK_ASCIIDOC_TOOLS
CHECK_PO4A
AM_CONDITIONAL([HAVE_MANPAGES], [test ! -z "$A2X" -a ! -z "$PO4A_TRANSLATE"])

AC_CONFIG_FILES([Makefile
                 po/Makefile.in
                 profiles/Makefile
                 rpm/authselect.spec
                 src/common/Makefile
                 src/cli/Makefile
                 src/lib/Makefile
                 src/lib/authselect.pc
                 src/man/Makefile
                 src/tests/Makefile])

AC_CONFIG_FILES([scripts/manpages-build.sh],
                [chmod +x scripts/manpages-build.sh])
AC_CONFIG_FILES([scripts/manpages-install.sh],
                [chmod +x scripts/manpages-install.sh])
AC_CONFIG_FILES([scripts/manpages-uninstall.sh],
                [chmod +x scripts/manpages-uninstall.sh])

AC_OUTPUT
