dnl Checks for tools needed to generate manual pages
AC_DEFUN([CHECK_ASCIIDOC_TOOLS],
[
  AC_PATH_PROG([A2X], [a2x])
  if test x"$A2X" = "x"; then
    AC_MSG_WARN([Could not find a2x, man pages will not be generated])
  fi
])
