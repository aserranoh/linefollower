AC_INIT([linefollower], [0.1], [toni.serranoh@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AM_CPPFLAGS=-DSYSCONFDIR='\"$(sysconfdir)\"'
AC_SUBST(AM_CPPFLAGS)
AC_CONFIG_HEADERS([config.h])
AM_PATH_PYTHON
PKG_CHECK_MODULES([glesv2], [glesv2])
PKG_CHECK_MODULES([egl], [egl])
PKG_CHECK_MODULES([x11], [x11])
PKG_CHECK_MODULES([opencv], [opencv])
AC_CONFIG_FILES([
   Makefile
   src/Makefile
   data/Makefile
])
AC_OUTPUT
