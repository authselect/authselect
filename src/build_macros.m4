dnl Require popt library and store ld and c flags in POPT_LIBS and POPT_CFLAGS.
AC_DEFUN([REQUIRE_POPT], [
    AC_SUBST(POPT_LIBS)
    AC_SUBST(POPT_CFLAGS)

    PKG_CHECK_MODULES([POPT], [popt], [found_popt=yes], [found_popt=no])

    AS_IF([test x"$found_popt" != xyes], [AC_CHECK_HEADERS([popt.h],
        [AC_CHECK_LIB([popt], [poptGetContext], [POPT_LIBS="-lpopt"],
            AC_MSG_ERROR([POPT library must support poptGetContext]))],
        [AC_MSG_ERROR([POPT header files are not installed])]
    )])
])
