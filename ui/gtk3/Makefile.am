# vim:set noet ts=4:
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

NULL =

libibus = $(top_builddir)/src/libibus-@IBUS_API_VERSION@.la

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	$(NULL)

AM_CFLAGS = \
	@GLIB2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@GTHREAD2_CFLAGS@ \
	@GTK3_CFLAGS@ \
	@X11_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBINDIR=\"@bindir@\" \
    -DIBUS_DISABLE_DEPRECATED \
	$(INCLUDES) \
	$(NULL)

AM_LDADD = \
	@GOBJECT2_LIBS@ \
	@GLIB2_LIBS@ \
	@GIO2_LIBS@ \
	@GTHREAD2_LIBS@ \
	@GTK3_LIBS@ \
	@X11_LIBS@ \
	$(libibus) \
	$(NULL)

AM_VALAFLAGS = \
	--vapidir=$(top_builddir)/bindings/vala \
	--pkg=gtk+-3.0 \
	--pkg=gdk-x11-3.0 \
	--pkg=ibus-1.0 \
	$(NULL)

bin_PROGRAMS = ibus-ui-gtk3

ibus_ui_gtk3_SOURCES = \
	application.vala \
	candidatearea.vala \
	candidatepanel.vala \
	handle.vala \
	iconwidget.vala \
	keybindingmanager.vala \
	panel.vala \
	pango.vala \
	separator.vala \
	$(NULL)

ibus_ui_gtk3_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)

ibus_ui_gtk3_LDADD = \
	$(AM_LDADD) \
	$(NULL)

-include $(top_srcdir)/git.mk