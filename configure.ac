# vim:set et ts=4:
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
m4_define([ibus_micro_version], [1])
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


# check gtk, gdk & pygtk
PKG_CHECK_MODULES(GTK2, [
    gtk+-2.0
])

GTK_BINARY_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
GTK_IM_MODULEDIR="$libdir"/gtk-2.0/immodules

PKG_CHECK_MODULES(GDK2, [
    gdk-2.0
])

# check dbus-glib
PKG_CHECK_MODULES(DBUS, [
    dbus-glib-1
])


# Check for Qt4
PKG_CHECK_MODULES(QT,
[QtCore >= 4.3.5 QtDBus >= 4.3.5],
[IBUS_HAS_QT4=yes],
[IBUS_HAS_QT4=no]
)

if test x"$IBUS_HAS_QT4" == x"yes"; then
    QMAKE=`$PKG_CONFIG --variable=qmake Qt`
    if test x"$QMAKE" == x""; then
        AC_PATH_PROG(QMAKE, qmake-qt4)
    fi
    if test x"$QMAKE" == x""; then
        AC_PATH_PROG(QMAKE, qmake)
    fi
    if test x"$QMAKE" == x""; then
        IBUS_HAS_QT4=no
        AC_MSG_WARN([Can not find qmake.])
    fi
    AC_SUBST(QMAKE)
fi

# Check for x11
PKG_CHECK_MODULES(X11, [
    x11
])

# check env
AC_PATH_PROG(ENV, env)
AC_SUBST(ENV)


# check python
AM_PATH_PYTHON([2.5])
AC_PATH_PROG(PYTHON_CONFIG, python$PYTHON_VERSION-config)
if test x"$PYTHON_CONFIG" == x""; then
    AC_PATH_PROG(PYTHON_CONFIG, python-config)
fi
if test x"$PYTHON_CONFIG" != x""; then
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
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.16.1)


# define configure arguments
AC_ARG_WITH(gtk-im-module-dir,
    AS_HELP_STRING([--with-gtk-im-module-dir[=DIR]],
        [Select gtk immodule dir]),
    GTK_IM_MODULEDIR=$with_gtk_im_module_dir
)
AC_SUBST(GTK_IM_MODULEDIR)

AC_ARG_ENABLE(qt4-immodule,
    AS_HELP_STRING([--disable-qt4-immodule],
        [do not build qt4 immodule]),
    [enable_qt4=$enableval],
    [enable_qt4=yes],
)
if test x"$IBUS_HAS_QT4" != x"yes"; then
    enable_qt4=no
fi
AM_CONDITIONAL(IBUS_BUILD_QT4, [test x"$enable_qt4" = x"yes" ])


# OUTPUT files
AC_CONFIG_FILES([ po/Makefile.in
Makefile
ibus.spec
ibus/Makefile
ibus/interface/Makefile
daemon/Makefile
daemon/ibus-daemon
launcher/ibus
launcher/ibus.desktop
launcher/Makefile
panel/Makefile
panel/ibus-panel
gconf/Makefile
gconf/ibus-gconf
engine/Makefile
gtk2/Makefile
qt4/Makefile
x11/Makefile
x11/IMdkit/Makefile
setup/Makefile
setup/ibus-setup
setup/ibus-setup.desktop
icons/Makefile
m4/Makefile
])

AC_OUTPUT

AC_MSG_RESULT([
Build options:
  Version                  $VERSION
  Install prefix           $prefix
  Build shared libs        $enable_shared
  Build static libs        $enable_static
  Gtk immodule dir         $GTK_IM_MODULEDIR
  Enable Qt4 IM module     $enable_qt4
])

