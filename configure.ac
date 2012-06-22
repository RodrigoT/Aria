#
# configure.ac for Aria
#
# Process this file with autoconf to produce a configure script.
#

AC_INIT(aria, 1.0.0, tujikawa@rednoah.com)
# Check whether a recent enough version of autoconf is being used
AC_PREREQ(2.52)
AC_ARG_PROGRAM()
# Set package name and version
PACKAGE="aria"
VERSION="1.0.0"
DATE_ROLLOUT="16 Dec 2002"
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AC_DEFINE_UNQUOTED(ARIA_VERSION, "Aria - version $VERSION")
AC_DEFINE_UNQUOTED(DATE_ROLLOUT, "$DATE_ROLLOUT")

# Set revision stamp
AC_REVISION($Id: configure.ac,v 1.15 2002/12/18 15:40:53 tujikawa Exp $)

AC_CONFIG_SRCDIR(src/main.cc)

AC_CANONICAL_HOST()

AC_PROG_MAKE_SET()

# Check compilers
AC_PROG_CXX()
AC_PROG_CXXCPP()
AC_PROG_CC()
AC_ISC_POSIX()
AC_C_INLINE()
if test x$CC = xgcc; then
  CFLAGS="$CFLAGS -Wall"
fi	
AC_PROG_INSTALL()

AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE()

#Collect machine information
machine=`uname -smr`
AC_DEFINE_UNQUOTED(D_MACHINE_COMPILED, "$machine")
date=`date`
AC_DEFINE_UNQUOTED(D_DATE_COMPILED, "$date")

#Check libstdc++
#Set test language to C++
AC_LANG([C++])
LIBS_save=$LIBS
LIBS="$LIBS -lstdc++"
AC_MSG_CHECKING([whether libstdc++ is installed])
AC_CACHE_VAL(have_stdcpp,
[AC_TRY_LINK([
#include <string>
using namespace std;
],
[
  string dummy;
],
[have_stdcpp=yes], [have_stdcpp=no])])
AC_MSG_RESULT($have_stdcpp)
if test "x$have_stdcpp" = xyes; then
  LIBS="$LIBS_save -lstdc++"
else
  AC_MSG_ERROR([libstdc++ not found. Please install it first.])
fi
#Set test language to C
AC_LANG([C])

#Check zlib
LIBS_save=$LIBS
LIBS="$LIBS -lz"
AC_MSG_CHECKING([whether zlib is installed])
AC_CACHE_VAL(have_zlib,
[AC_TRY_LINK([
#include <zlib.h>
],
[
  gzFile *dummy;
],
[have_zlib=yes], [have_zlib=no])])
AC_MSG_RESULT($have_zlib)
#AC_CHECK_LIB(z, main, Z_LIB="yes")
if test "x$have_zlib" = xyes; then
  LIBS="$LIBS_save -lz"
  AC_DEFINE(HAVE_ZLIB)
else
  LIBS=$LIBS_save
fi

#Check for SSL
have_openssl="no"
AC_CHECK_LIB([ssl], [main], [have_openssl="yes"])
if test "x$have_openssl" = xyes; then
  have_openssl="no"
  AC_CHECK_LIB([crypto], [main], [have_openssl="yes"])
  if test "x$have_openssl" = xyes; then
    LIBS="$LIBS -lcrypto -lssl"
    AC_DEFINE(HAVE_OPENSSL)
  fi
fi

#Check whether system has -lresolv
AC_CHECK_LIB([resolv], [inet_aton], LIBS="$LIBS -lresolv")

#
AC_HEADER_DIRENT()
AC_HEADER_STDC()
AC_HEADER_SYS_WAIT()
AC_HEADER_TIME()
AC_CHECK_HEADERS([arpa/inet.h errno.h fcntl.h netdb.h netinet/in.h sys/param.h sys/socket.h sys/time.h])

#Check whether system has long int timezone in time.h, not func timezone
AC_MSG_CHECKING(long int timezone)
CFLAGS_save=$CFLAGS
CFLAGS="$CFLAGS -Werror"
AC_CACHE_VAL([have_timezone],
[AC_TRY_COMPILE(
[
#include <time.h>
],
[
long int x = timezone; x++;
],
[have_timezone=yes],
[have_timezone=no], [have_timezone=no])])
AC_MSG_RESULT($have_timezone)
CFLAGS=$CFLAGS_save
if test "x$have_timezone" = xyes; then
  AC_DEFINE(HAVE_TIMEZONE)
fi

AC_STRUCT_TM()
AC_FUNC_ALLOCA()
AC_FUNC_FORK()
AC_FUNC_MALLOC()
AC_FUNC_MKTIME()
AC_FUNC_MMAP()
AC_TYPE_SIGNAL()
AC_FUNC_STAT()
AC_FUNC_STRTOD()
AC_CHECK_FUNCS([gethostbyname gettimeofday getwd inet_ntoa mempcpy memset mkdir select socket strerror strncasecmp strtol strtoul])

#Taken from configure.in of gnupg
#QUOTE START
#GNUPG_CHECK_ENDIAN

#GNUPG_CHECK_TYPEDEF(byte, HAVE_BYTE_TYPEDEF)
#GNUPG_CHECK_TYPEDEF(ushort, HAVE_USHORT_TYPEDEF)
#GNUPG_CHECK_TYPEDEF(ulong, HAVE_ULONG_TYPEDEF)
#GNUPG_CHECK_TYPEDEF(u32, HAVE_U32_TYPEDEF)

AC_CHECK_SIZEOF(unsigned short, 2)
AC_CHECK_SIZEOF(unsigned int, 4)
AC_CHECK_SIZEOF(unsigned long, 4)

if test "$ac_cv_sizeof_unsigned_short" = "0" \
   || test "$ac_cv_sizeof_unsigned_int" = "0" \
   || test "$ac_cv_sizeof_unsigned_long" = "0"; then
    AC_MSG_WARN([Hmmm, something is wrong with the sizes - using defaults]);
fi
#QUOTE END

#Check IPv6 aware functions
AC_CHECK_FUNCS(getaddrinfo getnameinfo freeaddrinfo)

#Internationalization macros
#Set of available languages.
ALL_LINGUAS="cs de es fr hu it pl ru ja zh_TW"
AM_GNU_GETTEXT()

localedir=${datadir}/locale
AC_SUBST(localedir)
AC_SUBST(datadir)

#Check glib and gtk+
AM_PATH_GLIB_2_0(2.1.0,,AC_MSG_ERROR([*** GLIB >= 2.1.0 not installed - please install first ***]))
AM_PATH_GTK_3_0(3.0.0,,AC_MSG_ERROR([*** GTK+ >= 3.0.0 not installed - please install first ***]),gthread)
GIVEN_CFLAGS=$CFLAGS

#Path to X
LIBS_save=$LIBS
LIBS="$LIBS $GTK_LIBS"
LIBS=$LIBS_save

AC_PATH_X()

CFLAGS_save=$CFLAGS
LIBS_save=$LIBS
CFLAGS="$CFLAGS $GTK_CFLAGS"
LIBS="$LIBS $GTK_LIBS"

#Check pthread.h
AC_CHECK_HEADER(pthread.h,,AC_MSG_ERROR([*** POSIX thread support not installed
- please install first ***]))

PTHREAD_LIBS=error
AC_CHECK_LIB(pthread, pthread_attr_init,
             PTHREAD_LIBS="-lpthread")
if test "x$PTHREAD_LIBS" = xerror; then
        AC_CHECK_LIB(pthreads, pthread_attr_init,
                PTHREAD_LIBS="-lpthreads")
fi
if test "x$PTHREAD_LIBS" = xerror; then
        AC_CHECK_LIB(pthread, pthread_attr_init,
                PTHREAD_LIBS="-pthread")
fi
if test "x$PTHREAD_LIBS" = xerror; then
        AC_CHECK_LIB(c_r, pthread_attr_init,
                PTHREAD_LIBS="-lc_r")
fi
if test "x$PTHREAD_LIBS" = xerror; then
        AC_CHECK_FUNC(pthread_attr_init, PTHREAD_LIBS="")
fi
if test "x$PTHREAD_LIBS" = xerror; then
        AC_MSG_ERROR(*** Unable to locate working posix thread library ***)
fi
AC_SUBST(PTHREAD_LIBS)

#Check whether Xlib is thread-safe
AC_MSG_CHECKING(whether the Xlib is thread-safe)
AC_CACHE_VAL(thread_safe_xlibs,
[AC_TRY_RUN([
#include <X11/Xlib.h>
int main()
{
   exit(!(XInitThreads()));
}],
thread_safe_xlibs=yes,
thread_safe_xlibs=no,
thread_safe_xlibs=no)])
AC_MSG_RESULT($thread_safe_xlibs)

AC_OUTPUT([
Makefile
src/Makefile
src/pixmaps/Makefile
intl/Makefile
po/Makefile.in
resources/Makefile
],
[touch Makefile.in
sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makfile])

AC_MSG_NOTICE([])
AC_MSG_NOTICE([Configulation Summary])
AC_MSG_NOTICE([  Install path:          ${prefix}])
AC_MSG_NOTICE([  gzip, deflate support: $have_zlib])
AC_MSG_NOTICE([  https support:         $have_openssl])
