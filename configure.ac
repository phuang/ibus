# vim:set et ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2010 Red Hat, Inc.
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
m4_define([ibus_released], [1])
m4_define([ibus_major_version], [1])
m4_define([ibus_minor_version], [3])
m4_define([ibus_micro_version], [2])
m4_define(ibus_maybe_datestamp,
    m4_esyscmd([if test x]ibus_released[ != x1; then date +.%Y%m%d | tr -d '\n\r'; fi]))

m4_define([ibus_version],
    ibus_major_version.ibus_minor_version.ibus_micro_version[]ibus_maybe_datestamp)

AC_INIT([ibus], [ibus_version], [http://code.google.com/p/ibus/issues/entry],[ibus])
AM_INIT_AUTOMAKE([1.10])
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# define PACKAGE_VERSION_* variables
AS_VERSION
AS_NANO
AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.35.0])

# For dislpay Date
m4_define(ibus_datedisplay,
    m4_esyscmd(date '+%a %b %d %Y' | tr -d '\n\r'))
DATE_DISPLAY="ibus_datedisplay"
AC_SUBST(DATE_DISPLAY)

# If only source code changed, lt_revision + 1
# If any interface added, lt_age + 1
# If any interfaces changed or removed, lt_current + 1, lt_revision = 0, lt_age = 0
m4_define([lt_current], [2])
m4_define([lt_revision], [0])
m4_define([lt_age], [0])
LT_VERSION_INFO="lt_current:lt_revision:lt_age"
AC_SUBST(LT_VERSION_INFO)

# check inotify
AC_CHECK_HEADERS([sys/inotify.h])

# check funcs
AC_CHECK_FUNCS(daemon)

# check glib2
AM_PATH_GLIB_2_0
PKG_CHECK_MODULES(GLIB2, [
    glib-2.0 >= 2.18
])
PKG_CHECK_MODULES(GOBJECT2, [
    gobject-2.0 >= 2.18
])
PKG_CHECK_MODULES(GIO2, [
    gio-2.0 >= 2.18
])
PKG_CHECK_MODULES(GTHREAD2, [
    gthread-2.0 >= 2.18
])
# PKG_CHECK_MODULES(PYGOBJECT2, [
#     pygobject-2.0 >= 2.14
# ])

# check for gtk, gdk & pygtk
PKG_CHECK_MODULES(GTK2, [
    gtk+-2.0
])

gtk_binary_version=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
GTK_IM_MODULEDIR="$libdir"/gtk-2.0/$gtk_binary_version/immodules

PKG_CHECK_MODULES(GDK2, [
    gdk-2.0
])

# check for gtk-doc
GTK_DOC_CHECK(1.9)

# check for dbus-glib
PKG_CHECK_MODULES(DBUS, [
    dbus-1
])

AC_ARG_ENABLE(gconf,
    AS_HELP_STRING([--disable-gconf],
                   [Do not use GConf code]),
    [enable_gconf=$enableval],
    [enable_gconf=yes]
)

AM_CONDITIONAL([ENABLE_GCONF], [test "x$enable_gconf" = "xyes"])

if test x"$enable_gconf" != x"no"; then
    # check gconf
    PKG_CHECK_MODULES(GCONF,
        [gconf-2.0 >= 2.12],
    )

    AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
    if test x"$GCONFTOOL" = xno; then
        AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
    fi

    AM_GCONF_SOURCE_2
    # GCONF_SCHEMAS_INSTALL should be set in macro AM_GCONF_SOURCE_2
else
    AM_CONDITIONAL([GCONF_SCHEMAS_INSTALL], [false])
fi

# Check for x11
PKG_CHECK_MODULES(X11, [
    x11
])

AC_CHECK_HEADERS([X11/XKBlib.h])

# check env
AC_PATH_PROG(ENV, env)
AC_SUBST(ENV)

AC_ARG_ENABLE(python,
    AS_HELP_STRING([--disable-python],
                   [Do not use Python code]),
    [enable_python=$enableval],
    [enable_python=yes]
)

AM_CONDITIONAL([ENABLE_PYTHON], [test "x$enable_python" = "xyes"])

if test x"$enable_python" != x"no"; then
    # check python
    AM_PATH_PYTHON([2.5])
    AC_PATH_PROG(PYTHON_CONFIG, python$PYTHON_VERSION-config)
    if test x"$PYTHON_CONFIG" = x""; then
        AC_PATH_PROG(PYTHON_CONFIG, python-config)
    fi
    if test x"$PYTHON_CONFIG" != x""; then
        PYTHON_CFLAGS=`$PYTHON_CONFIG --includes`
        PYTHON_LIBS=`$PYTHON_CONFIG --libs`
    else
        PYTHON_CFLAGS=`$PYTHON $srcdir/python-config.py --includes`
        PYTHON_LIBS=`$PYTHON $srcdir/python-config.py --libs`
    fi
    PYTHON_INCLUDES="$PYTHON_CFLAGS"
    AC_SUBST(PYTHON_CFLAGS)
    AC_SUBST(PYTHON_INCLUDES)
    AC_SUBST(PYTHON_LIBS)
fi

#
# REBUILD = \#
# AC_SUBST(REBUILD)

# define GETTEXT_* variables
GETTEXT_PACKAGE=ibus
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT

# define configure arguments
AC_ARG_WITH(gtk-im-module-dir,
    AS_HELP_STRING([--with-gtk-im-module-dir[=DIR]],
        [Select gtk immodule dir]),
    GTK_IM_MODULEDIR=$with_gtk_im_module_dir
)
AC_SUBST(GTK_IM_MODULEDIR)

if test x"$enable_python" != x"no"; then
    # check for dbus-python
    AC_ARG_ENABLE(dbus-python-check,
        AS_HELP_STRING([--disable-dbus-python-check],
            [Do not check dbus-python]),
        [enable_dbus_python_check=$enableval],
        [enable_dbus_python_check=yes],
    )
    PKG_CHECK_MODULES(DBUS_PYTHON,
        [dbus-python >= 0.83.0],
        [IBUS_HAS_DBUS_PYTHON=yes],
        [IBUS_HAS_DBUS_PYTHON=no]
    )
    if test x"$IBUS_HAS_DBUS_PYTHON" != x"yes"; then
        if test x"$enable_dbus_python_check" != x"no"; then
            AC_MSG_ERROR([can not find dbus-python >= 0.83.0. Please install or update dbus-python.])
        else
            AC_MSG_WARN([can not find dbus-python >= 0.83.0. It is required.])
        fi
    fi
fi

# check iso-codes
PKG_CHECK_MODULES(ISOCODES, [
    iso-codes
])
ISOCODES_PREFIX=`pkg-config iso-codes --variable=prefix`
AC_SUBST(ISOCODES_PREFIX)

# OUTPUT files
AC_CONFIG_FILES([ po/Makefile.in
Makefile
ibus-1.0.pc
ibus.spec
xinput-ibus
client/Makefile
client/gtk2/Makefile
client/x11/Makefile
src/Makefile
src/ibusversion.h
bus/Makefile
util/Makefile
util/IMdkit/Makefile
data/Makefile
data/icons/Makefile
data/keymaps/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/ibus/Makefile
m4/Makefile
ibus/_config.py
ibus/Makefile
ibus/interface/Makefile
ui/Makefile
ui/gtk/Makefile
ui/gtk/ibus-ui-gtk
ui/gtk/gtkpanel.xml.in
setup/Makefile
setup/ibus-setup
gconf/Makefile
gconf/gconf.xml.in
])

AC_OUTPUT
AC_MSG_RESULT([
Build options:
  Version                  $VERSION
  Install prefix           $prefix
  Build shared libs        $enable_shared
  Build static libs        $enable_static
  Gtk immodule dir         $GTK_IM_MODULEDIR
  Build python modules     $enable_python
  Build gconf modules      $enable_gconf
  Build document           $enable_gtk_doc
])

