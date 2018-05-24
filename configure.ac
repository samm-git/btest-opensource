AC_INIT([btest], [1.0], [bug-report@address])
AM_INIT_AUTOMAKE
AC_ARG_ENABLE(debugging, [  --disable-debugging             disable debugging code], ac_cv_debugging=$enableval, ac_cv_debugging=yes)
AC_PROG_CC
dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS()
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
