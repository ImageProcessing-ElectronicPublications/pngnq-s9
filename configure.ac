# Autoconf for pngnq-s9

AC_INIT([pngnq-s9],[2.0.2], [apr-apr-apr@sourceforge.net])
AM_INIT_AUTOMAKE[-Wall -Werror]
AC_COPYRIGHT([Copyright 1994-2012, Coyle, Dekker, Lesiński, Pellas-Rice, Roelf et al])

# check for progs
AC_CHECK_PROG([HAS_LIBPNG_CONFIG],[libpng-config],[1])

# checks for header files
AC_CHECK_HEADERS([ctype.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/time.h]) 
AC_CHECK_HEADERS([valgrind/callgrind.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([getopt.h])
AC_CHECK_HEADERS([stdlib.h])
                   
# checks for compiler characteristics
AC_PROG_CC
AC_C_CONST
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_HEADER_STDBOOL

# checks for libraries
AC_SEARCH_LIBS([zlibVersion],[z])
AC_SEARCH_LIBS([sqrt],[m])
PKG_CHECK_MODULES([PNG], [libpng >= 1.2.0])

# checks for library functions
AC_CHECK_FUNCS([getopt])
AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset]) 
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strrchr])

AC_CONFIG_HEADERS([src/config.h]) 
AM_CONDITIONAL([USE_FREEGETOPT],[ test $ac_cv_func_getopt = "no" ])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
