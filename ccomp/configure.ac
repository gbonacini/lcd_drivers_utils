#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([simple_lcd], m4_esyscmd([ tr -d '\n' < version ]))

CFLAGS=""
LDFLAGS=""

AC_ARG_WITH([test],
        [AS_HELP_STRING([    --with-test], [Test mode On])],
        [],
        [with_test=no])

AS_IF([test "x$with_test" != xno],
        [
        AM_CONDITIONAL(WITH_TEST, true)
        AC_DEFINE(WITH_TEST,[],[Use Test Mode])
        ], [
        AM_CONDITIONAL(WITH_TEST, false)
        ])


AC_CONFIG_SRCDIR([src/simple_lcd.c])

AC_CONFIG_HEADER([include/config.h])

LT_INIT
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([disable])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([Makefile
            src/Makefile])

# Checks for programs.
AC_PROG_CC([ccomp gcc])
AC_PROG_INSTALL

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

AC_CANONICAL_HOST
case $host_os in
  darwin* )
              OPTIMIZATION="-O2 "
              if test "x$CC" = xgcc; then
                    CFLAGS=" -std=c11 -g -Weverything \
                             -D_FORTIFY_SOURCE=2 ${OPTIMIZATION} -fstack-protector \
                             --param=ssp-buffer-size=4 -Wformat -Werror=format-security \
                             -Wno-deprecated-declarations -Wno-documentation -Wno-c++98-compat \
                             -Wno-padded -Wno-c++98-compat-pedantic -Wno-undefined-func-template "
		            LDFLAGS="$LDFLAGS "
              else
                    CFLAGS=" -std=c18 -O2 -g -Wall "
		            LDFLAGS="$LDFLAGS  "
              fi
        ;;
  linux*)
              OPTIMIZATION="-O2 "
              if test "x$CC" = xgcc; then
                    CFLAGS=" -std=c11 -g -Wall -Wextra  -DLINUX_OS \
                             -D_FORTIFY_SOURCE=2 ${OPTIMIZATION} -fstack-protector   \
                             --param=ssp-buffer-size=4 -Wformat -Werror=format-security "
                    LDFLAGS=" $LDFLAGS -Wl,-z,relro  "
              else
                    CFLAGS="-DLINUX_OS -std=c18 -O2 -g -Wall  "
		        LDFLAGS=" $LDFLAGS  "
              fi

        ;;
    *)
            CFLAGS=" -std=c11  -O2 -g -Wall  "
	      LDFLAGS=" $LDFLAGS   "
            #AC_MSG_ERROR([Your platform is not currently supported])
        ;;
esac


AC_OUTPUT

