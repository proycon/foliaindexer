#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# $Id: configure.ac 15158 2012-09-04 14:23:56Z sloot $
# $URL: https://ilk.uvt.nl/svn/sources/ucto/trunk/configure.ac $

AC_PREREQ(2.59)
AC_INIT([foliaindexer], [0.1], [proycon@anaproy.nl])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])

SYSCONFDIR=$sysconfdir
AC_SUBST([SYSCONFDIR])

if test x"${CXXFLAGS+set}" = xset; then
  # the user set CXXFLAGS; don't override it.
  cxx_flags_were_set=true
else
  cxx_flags_were_set=false
fi

# Checks for programs.
AC_PROG_CXX( [g++] )

if $cxx_flags_were_set; then
  CXXFLAGS=$CXXFLAGS
fi

AC_PROG_RANLIB
# use libtool
AC_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])

# when running tests, use CXX
AC_LANG([C++])





# Checks for libraries.

if test $prefix = "NONE"; then
   prefix="$ac_default_prefix"
fi

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.

# ugly hack when PKG_CONFIG_PATH isn't defined.
# couldn't get it to work otherwise
if test "x$PKG_CONFIG_PATH" = x; then
    export PKG_CONFIG_PATH="/usr/lib/pkgconfig"
fi

useICU=1;
# inspired by feh-1.3.4/configure.ac.  Tnx Tom Gilbert and feh hackers.
AC_ARG_WITH(icu,
       [  --with-icu=DIR       use ICU installed in <DIR>],
       [if test "$with_icu" = "no"; then
           useICU=0
	else    
         CXXFLAGS="$CXXFLAGS -I$withval/include"
        LIBS="-L$withval/lib $LIBS"
    fi] )

if test "$useICU" = "1"; then
    AX_ICU_CHECK( [3.6],
	[CXXFLAGS="$CXXFLAGS $ICU_CPPSEARCHPATH"
	LIBS="$ICU_LIBPATH $ICU_LIBS $ICU_IOLIBS $LIBS"],
	[AC_MSG_FAILURE( "No ICU development environment found. Please check if libicu-dev or the like is installed" )] )
     AC_DEFINE(HAVE_ICU, 1, we want to use ICU )
else
  AC_MSG_ERROR("ICU support is required")
fi


BOOST_REQUIRE([1.34])
dnl Then look for specific Boost libraries you need:
BOOST_SYSTEM
BOOST_FILESYSTEM

#PKG_CHECK_MODULES([XML2], [libxml-2.0 >= 2.6.16] )
#CXXFLAGS="$CXXFLAGS $XML2_CFLAGS"
#LIBS="$LIBS $XML2_LIBS"

AC_ARG_WITH(folia,
       [  --with-folia=DIR       use libfolia installed in <DIR>; 
               note that you can install folia in a non-default directory with
               ./configure --prefix=<DIR> in the folia installation directory],
       [PKG_CONFIG_PATH="$withval/lib/pkgconfig:$PKG_CONFIG_PATH"],
       [PKG_CONFIG_PATH="$prefix/lib/pkgconfig:$PKG_CONFIG_PATH"])
AC_MSG_NOTICE( [pkg-config search path: $PKG_CONFIG_PATH] )
PKG_CHECK_MODULES([folia], [folia >= 0.9] )
CXXFLAGS="$folia_CFLAGS $CXXFLAGS"
LIBS="$folia_LIBS $LIBS"

#AC_ARG_WITH(ticcutils,
#       [  --with-ticcutils=DIR       use ticcutils installed in <DIR>; 
#               note that you can install ticcutils in a non-default directory with
#               ./configure --prefix=<DIR> in the ticcutils installation directory],
#       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$withval/lib/pkgconfig"],
#       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$prefix/lib/pkgconfig"])
##  AC_MSG_NOTICE( [pkg-config search path: $PKG_CONFIG_PATH] )
#PKG_CHECK_MODULES([ticcutils], [ticcutils >= 0.1] )
#CXXFLAGS="$CXXFLAGS $ticcutils_CFLAGS"
#LIBS="$LIBS $ticcutils_LIBS"


# Checks for library functions.

AC_OUTPUT([
  Makefile
  foliaindexer.pc
  m4/Makefile
  src/Makefile
])
