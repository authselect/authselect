dnl Checks for tools needed to translate manual pages
AC_DEFUN([CHECK_PO4A],
[
  AC_PATH_PROG([PO4A_TRANSLATE],[po4a-translate])
  if test x"$PO4A_TRANSLATE" = "x"; then
    AC_MSG_WARN([Could not find po4a-translate, man pages will not be generated])
  fi
])
