#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").  
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at sunpinyin/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at sunpinyin/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
# Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# Process this file with autoconf to produce a configure script.
AC_INIT(sunpinyin, 2.0)
AC_CANONICAL_SYSTEM()
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(src/slm/slm.cpp)
AM_CONFIG_HEADER(config.h)
AM_CONDITIONAL(DEBUG, test "$enable-debug" = yes)

GETTEXT_PACKAGE=sunpinyin2
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])

dnl Init gettext
ALL_LINGUAS="zh_CN"
AM_GLIB_GNU_GETTEXT
LTLIBINTL=
AC_SUBST(LTLIBINTL)

dnl AC_C_BIGENDIAN defines WORDS_BIGENDIAN if it is true by default
AC_C_BIGENDIAN
AC_C_BIGENDIAN([my_host_endian=be],[my_host_endian=le],[
AC_MSG_ERROR([Cannot determine endianness])])

case "${host_os}" in
*bsd|*linux*)       my_host_os=linux;;
*solaris*)          my_host_os=solaris;;
*darwin*)           my_host_os=mac;;
*beos*|*haiku*)     my_host_os=beos;;
esac

AC_LINK_FILES(src/config/${my_host_os}.h, src/host_os.h)

RAW_DATA_DIR="raw"
AC_SUBST(RAW_DATA_DIR)

SWAP_DATA_DIR="swap"
AC_SUBST(SWAP_DATA_DIR)

RESULT_DATA_DIR="data"
AC_SUBST(RESULT_DATA_DIR)

SOURCE_DIR="src"
AC_SUBST(SOURCE_DIR)

SLM_SRC_DIR="src/slm"
AC_SUBST(SLM_SRC_DIR)

LEXICON_SRC_DIR="src/lexicon"
AC_SUBST(LEXICON_SRC_DIR)

PINYIN_SRC_DIR="src/pinyin"
AC_SUBST(PINYIN_SRC_DIR)


dnl-----------------------------------------------------------------------
dnl    --enable-debug
dnl-----------------------------------------------------------------------
AC_ARG_ENABLE([debug],
    AC_HELP_STRING([--enable-debug], [compile in debug mode (default no) ]),
    [DEBUGMODE="${enableval}"],
    [DEBUGMODE="no"])

if test "X$DEBUGMODE" = "Xyes" ; then
    AC_DEFINE(DEBUG, 1, [Define if you want to make for debugging.])
fi

AC_ARG_WITH([sunpinyin-datadir],
    AC_HELP_STRING([--with-sunpinyin-datadir=PATH],
                   [specify the directory where the lexicon data is installed]),
    [SUNPINYIN_DATADIR="${withval}"],
    [SUNPINYIN_DATADIR=""])
AC_SUBST(SUNPINYIN_DATADIR)

PKG_CHECK_MODULES(sqlite3, [sqlite3 >= 3.0])

dnl-----------------------------------------------------------------------
dnl    --enable-documents
dnl-----------------------------------------------------------------------
AC_ARG_ENABLE(documents,
    [AS_HELP_STRING([--enable-documents], [build documents])], 
    [ENABLE_DOCUMENTS="${enableval}"], 
    [ENABLE_DOCUMENTS="no"])

if test "X$ENABLE_DOCUMENTS" = "Xyes"; then
  BUILD_DIR_DOCUMENTS=doc
  AC_SUBST(BUILD_DIR_DOCUMENTS)
  AC_CONFIG_FILES([doc/Makefile])
fi

AC_ARG_ENABLE([gtkstandalone],
    AC_HELP_STRING([--enable-gtkstandalone], [compile standalone gtk program for demo/debug(default=no) ]),
    [ENABLE_GTKSTANDALONE="${enableval}"],
    [ENABLE_GTKSTANDALONE="no"])
AM_CONDITIONAL(WRAPPER_GTK_STANDALONE, [test "X${ENABLE_GTKSTANDALONE}" = "Xyes"])

dnl 
dnl for scim wrapper
dnl 
AC_ARG_ENABLE([scim],
    AC_HELP_STRING([--enable-scim], [compile as scim (default=no) ]),
    [ENABLE_SCIM="${enableval}"],
    [ENABLE_SCIM="no"])

if test "X${ENABLE_SCIM}" = "Xyes" ; then
    # Check SCIM
    SCIM_VERSION=1.4.0
    AC_SUBST(SCIM_VERSION)
    PKG_CHECK_MODULES(SCIM,[scim >= $SCIM_VERSION])

    # Check if we should build setup module
    PKG_CHECK_MODULES(SCIM_GTKUTILS,[scim-gtkutils >= $SCIM_VERSION],
                                    [SCIM_HAS_GTKUTILS=yes],
                                    [SCIM_HAS_GTKUTILS=no])
    
    # Set paths
    SCIM_ICONDIR=`$PKG_CONFIG --variable=icondir scim`
    SCIM_MODULEDIR=`$PKG_CONFIG --variable=moduledir scim`
    SCIM_DATADIR=`$PKG_CONFIG --variable=scimdatadir scim`
    SCIM_SUNPINYIN_DATADIR=$SCIM_DATADIR/sunpinyin
    AC_SUBST(SCIM_ICONDIR)
    AC_SUBST(SCIM_MODULEDIR)
    AC_SUBST(SCIM_DATADIR)
    AC_SUBST(SCIM_SUNPINYIN_DATADIR)
fi
AM_CONDITIONAL(WRAPPER_SCIM, [test "X${ENABLE_SCIM}" = "Xyes"])

AM_CONDITIONAL(SCIM_BUILD_SETUP, [test "X$SCIM_HAS_GTKUTILS" = "Xyes"])

dnl 
dnl for ibus wrapper
dnl 
AC_ARG_ENABLE([ibus],
    AC_HELP_STRING([--enable-ibus], [compile as ibus (default=no) ]),
    [ENABLE_IBUS="${enableval}"],
    [ENABLE_IBUS="no"])

if test "X${ENABLE_IBUS}" = "Xyes" ; then
    # Check IBus
    AC_SUBST(IBUS_VERSION)
    PKG_CHECK_MODULES(IBUS,
                      [ibus-1.0 >= 1.2.0],
                      [AC_DEFINE(WITH_IBUS_1_2_0, 1, ["With keycode"])],
                      [PKG_CHECK_MODULES(IBUS, 
                                        [ibus-1.0 >= 1.1.0],
                                        [AC_DEFINE(WITH_IBUS_1_1_0, 1, ["Without keycode"])])])
    AM_PATH_PYTHON([2.5])
fi
AM_CONDITIONAL(WRAPPER_IBUS, [test "X${ENABLE_IBUS}" = "Xyes"])

dnl
dnl Xim wrapper
dnl
AC_ARG_ENABLE([xim],
        AC_HELP_STRING([--enable-xim], [compile as xim (defualt=no)]),
        [ENABLE_XIM="${enableval}"],
        [ENABLE_XIM="no"])

if test "X${ENABLE_XIM}" = "Xyes" ; then
   GTK_VERSION=2.12.0
   AC_SUBST(GTK_VERSION)
   PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= $GTK_VERSION])
   PKG_CHECK_MODULES(X11, x11)
fi
AM_CONDITIONAL(WRAPPER_XIM, [test "X${ENABLE_XIM}" = "Xyes"])

dnl override the default setting of DATADIRs
if test "x$SUNPINYIN_DATADIR" != "x" ; then
    SCIM_SUNPINYIN_DATADIR="$SUNPINYIN_DATADIR"
    CLE_SUNPINYIN_DATADIR="$SUNPINYIN_DATADIR"
    IBUS_SUNPINYIN_DATADIR="$SUNPINYIN_DATADIR"
    XIM_SUNPINYIN_DATADIR="$SUNPINYIN_DATADIR"
    AC_SUBST(SCIM_SUNPINYIN_DATADIR)
    AC_SUBST(CLE_SUNPINYIN_DATADIR)
    AC_SUBST(IBUS_SUNPINYIN_DATADIR)
    AC_SUBST(XIM_SUNPINYIN_DATADIR)
fi

AM_CONDITIONAL(OVERRIDE_DEFAULT_PINYINDATADIR, [test "x${SUNPINYIN_DATADIR}" != "x"])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.35.0])

AC_CHECK_PROGS([WGET], [wget curl])
if test -z "$WGET"; then 
   AC_MSG_WARN([cannot find wget (or equivalent) command])
fi
if test "x${WGET}" = "xwget" ; then
   WGET="wget -q"
fi
if test "x${WGET}" = "xcurl" ; then
   WGET="curl -O"
fi

AC_CHECK_DECL([__SUNPRO_C], [SUNCC="yes"], [SUNCC="no"])
if test "$SUNCC" = "yes"
then
    CFLAGS="-features=extensions $CFLAGS"
    CXXFLAGS="-features=extensions $CXXFLAGS"
fi  

# Checks for libraries.

# Checks for header files.
AC_PATH_X
AC_HEADER_STDC
AC_CHECK_HEADERS(limit.h assert.h getopt.h fcntl.h locale.h stdlib.h string.h unistd.h wchar.h sys/mman.h libintl.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS(getopt_long get_opt munmap memset setlocale strdup strndup strlen strcpy log2 exp2)

have_iconv=no
AC_MSG_CHECKING(for iconv)
    for iconv_lib in none iconv ; do
        save_LIBS="$LIBS"
        if test "$iconv_lib" != "none" ; then
            LIBS="$LIBS -l$iconv_lib"
        fi
        AC_TRY_LINK(
            [#include <iconv.h>],
            [iconv_t ic = iconv_open("UTF-16", "UTF-8")],
            have_iconv=yes,
            have_iconv=no)
        LIBS="$save_LIBS"
        if test $have_iconv = yes; then
            AC_DEFINE(HAVE_ICONV_H, 1, [Define if you have the 'iconv.h'.])
            if test "$iconv_lib" != "none" ; then
                LIBS="$LIBS -l$iconv_lib"
            fi
            break
        fi
    done
AC_MSG_RESULT($have_iconv)
if test "x$have_iconv" = "xno" ; then
    AC_MSG_WARN([*** iconv library not found, "pytrie_gen" cannot work properly.])
fi

if test "x$gt_cv_have_gettext" != "xyes" ; then
    USE_NLS=no
fi
AC_SUBST(USE_NLS)

AC_OUTPUT(Makefile
          build/Makefile
          wrapper/Makefile
          wrapper/gtk_standalone/Makefile
          wrapper/ibus/Makefile
          wrapper/ibus/src/Makefile
          wrapper/ibus/src/sunpinyin.xml.in
          wrapper/ibus/setup/Makefile
          wrapper/ibus/setup/ibus-setup-sunpinyin
          wrapper/ibus/setup/config.py
          wrapper/ibus/data/Makefile
          wrapper/xim/Makefile
          python/Makefile
          data/Makefile
          raw/Makefile
          po/Makefile.in)
          
