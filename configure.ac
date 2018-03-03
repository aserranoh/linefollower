AC_INIT([linefollower], [0.1], [toni.serranoh@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AM_CPPFLAGS=-DSYSCONFDIR='\"$(sysconfdir)\"'
AC_SUBST(AM_CPPFLAGS)

PKG_CHECK_MODULES([opencv], [opencv])

# Use GLES2 (Virtual Camera)
AC_ARG_WITH([gles2],
    AS_HELP_STRING([--with-gles2], [Add support for the virtual camera]))
AS_IF([test "x$with_gles2" != "xno"], [
    PKG_CHECK_MODULES([glesv2], [glesv2])
    PKG_CHECK_MODULES([egl], [egl])
    PKG_CHECK_MODULES([x11], [x11])
    AC_DEFINE([WITH_GLES2], [1], [Define if using GLES2])
])
AM_CONDITIONAL([GLES2], [test "x$with_gles2" != "xno"])

# Use GTK
AC_ARG_WITH([gtk],
    AS_HELP_STRING([--with-gtk], [Add support to show the camera frames]))
AS_IF([test "x$with_gtk" != "xno"], [
    PKG_CHECK_MODULES([gtk], [gtk+-2.0])
    AC_DEFINE([WITH_GTK], [1], [Define if using GTK])
])
AM_CONDITIONAL([GTK], [test "x$with_gtk" != "xno"])

# Use Python
AC_ARG_WITH([python],
    AS_HELP_STRING([--with-python], [generate a python script and module]))
AS_IF([test "x$with_python" != "xno"], [
    AM_PATH_PYTHON
])
AM_CONDITIONAL([PYTHON], [test "x$with_python" != "xno"])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
   Makefile
   src/Makefile
   data/Makefile
])
AC_OUTPUT
