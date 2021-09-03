#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(VERSION, BUG-REPORT-ADDRESS)
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/rastertosii.c])
AM_CONFIG_HEADER([config.h])
AC_PREFIX_DEFAULT([/usr/local])

AC_ARG_WITH(cups_dir, [  --with-cups-dir         Specify CUPS install directory], [CUPSDIR=$with_cups_dir],[CUPSDIR=/usr/local])
if test ! -d $CUPSDIR/lib/cups/filter ; then
	AC_MSG_ERROR([CUPS filter directory is not found.])
fi
AC_CHECK_LIB(cups, cupsParseOptions, , AC_MSG_ERROR([CUPS is required.]), [-L$CUPSDIR])
AC_SUBST(CUPSDIR)

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero gettimeofday memset pow select])

AC_CONFIG_FILES([Makefile
                 ppd/Makefile
                 src/Makefile])
AC_OUTPUT
