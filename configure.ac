# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

# if not 1, append datestamp to the version number.
m4_define([ibus_released], [0])
m4_define([ibus_major_version], [0])
m4_define([ibus_minor_version], [1])
m4_define([ibus_micro_version], [0])
m4_define(ibus_maybe_datestamp,
	m4_esyscmd([if test x]ibus_released[ != x1; then date +.%Y%m%d | tr -d '\n\r'; fi]))

m4_define([ibus_version],
	ibus_major_version.ibus_minor_version.ibus_micro_version[]ibus_maybe_datestamp)

AC_INIT([ibus], [ibus_version], [http://code.google.com/p/ibus/issues/entry],[ibus])
AM_INIT_AUTOMAKE([1.10])
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# define PACKAGE_VERSION_* variables
AS_VERSION
AS_NANO
AM_SANITY_CHECK
AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

# check inotify
AC_CHECK_HEADERS([sys/inotify.h])

# check glib2
AM_PATH_GLIB_2_0
PKG_CHECK_MODULES(GLIB2, [
	glib-2.0
])
PKG_CHECK_MODULES(GOBJECT2, [
	gobject-2.0
])
PKG_CHECK_MODULES(PYGOBJECT2, [
	pygobject-2.0
])


# check gtk & pygtk
PKG_CHECK_MODULES(GTK2, [
	gtk+-2.0
])

PKG_CHECK_MODULES(PYGTK2, [
	pygtk-2.0
])

# check dbus-glib
PKG_CHECK_MODULES(DBUS, [
	dbus-glib-1
])
QMAKE=`$PKG_CONFIG --variable=qmake Qt`
AC_SUBST(QMAKE)


# Check for Qt4
PKG_CHECK_MODULES(QT, [
	Qt >= 4.3.5
])


# check anthy
PKG_CHECK_MODULES(ANTHY, [
	anthy
])


# check swig
AC_PATH_PROG(SWIG, swig)
if test x"$SWIG" == x""; then
	 AC_MSG_ERROR([can not find swig])
fi
AC_SUBST(SWIG)

# check env
AC_PATH_PROG(ENV, env)
AC_SUBST(ENV)

# check python
AM_PATH_PYTHON([2.5])
PYTHON_CONFIG=`type -p python$PYTHON_VERSION-config`
if test "$PYTHON_CONFIG" != ""; then
    PYTHON_CFLAGS=`$PYTHON_CONFIG --includes`
    PYTHON_LIBS=`$PYTHON_CONFIG --libs`
else
    PYTHON_CFLAGS=`$PYTHON $srcdir/python-config.py --includes`
    PYTHON_LIBS=`$PYTHON $srcdir/python-config.py --libs`
fi
AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)


# define GETTEXT_* variables
GETTEXT_PACKAGE=ibus
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Package name for gettext])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.16.1)


# OUTPUT files
AC_CONFIG_FILES([ po/Makefile.in
Makefile
ibus.spec
ibus/Makefile
ibus/interface/Makefile
daemon/Makefile
daemon/ibus-daemon
panel/Makefile
panel/ibus-panel
engine/Makefile
engine/anthy/Makefile
engine/anthy/ibus-engine-anthy
engine/enchant/Makefile
engine/enchant/ibus-engine-enchant
gtk2/Makefile
qt4/Makefile
icons/Makefile
m4/Makefile
])

AC_OUTPUT
