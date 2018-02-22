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

AC_DEFUN([REQUIRE_CMOCKA],
[
    PKG_CHECK_EXISTS(cmocka >= 1.0.0,
        [AC_CHECK_HEADERS([stdarg.h stddef.h setjmp.h],
            [], dnl We are only intrested in action-if-not-found
            [AC_MSG_WARN([Header files stdarg.h stddef.h setjmp.h are required by cmocka])
             cmocka_required_headers="no"
            ]
        )
        AS_IF([test x"$cmocka_required_headers" != x"no"],
              [PKG_CHECK_MODULES([CMOCKA], [cmocka], [have_cmocka="yes"])]
        )],
        [AC_MSG_WARN([No libcmocka-1.0.0 or newer library found, cmocka tests will not be built])]
    )
    AM_CONDITIONAL([HAVE_CMOCKA], [test x$have_cmocka = xyes])
])
