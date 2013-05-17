# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2013 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2013 Red Hat, Inc.
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

component_DATA = \
	gtkpanel.xml \
	$(NULL)
componentdir = $(pkgdatadir)/component

gtkpanel.xml: gtkpanel.xml.in
	$(AM_V_GEN) sed \
		-e 's|@VERSION[@]|$(VERSION)|g' \
		-e 's|@libexecdir[@]|$(libexecdir)|g' $< > $@.tmp && \
		mv $@.tmp $@

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	$(NULL)

USE_SYMBOL_ICON = FALSE

# force include config.h before gi18n.h.
AM_CPPFLAGS = -include $(CONFIG_HEADER)

AM_CFLAGS = \
	@GLIB2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@GTHREAD2_CFLAGS@ \
	@GTK3_CFLAGS@ \
	@X11_CFLAGS@ \
	$(INCLUDES) \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DBINDIR=\"$(bindir)\" \
	-DIBUS_DISABLE_DEPRECATED \
	-DSWITCHER_USE_SYMBOL_ICON=$(USE_SYMBOL_ICON) \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable \
	-Wno-unused-function \
	$(NULL)

AM_LDADD = \
	@GOBJECT2_LIBS@ \
	@GLIB2_LIBS@ \
	@GIO2_LIBS@ \
	@GTHREAD2_LIBS@ \
	@GTK3_LIBS@ \
	@X11_LIBS@ \
	-lXi \
	$(libibus) \
	$(NULL)

AM_VALAFLAGS = \
	--vapidir=$(top_builddir)/bindings/vala \
	--vapidir=$(top_srcdir)/bindings/vala \
	--pkg=posix \
	--pkg=gtk+-3.0 \
	--pkg=gdk-x11-3.0 \
	--pkg=ibus-1.0 \
	--pkg=config \
	--pkg=xi \
	$(NULL)

if ENABLE_LIBNOTIFY
AM_CFLAGS += \
	@LIBNOTIFY_CFLAGS@ \
	$(NULL)

AM_LDADD += \
	@LIBNOTIFY_LIBS@ \
	$(NULL)

AM_VALAFLAGS += \
	--pkg=libnotify \
	-D ENABLE_LIBNOTIFY \
	$(NULL)
endif

libexec_PROGRAMS = ibus-ui-gtk3

ibus_ui_gtk3_SOURCES = \
	application.vala \
	candidatearea.vala \
	candidatepanel.vala \
	handle.vala \
	iconwidget.vala \
	keybindingmanager.vala \
	panel.vala \
	pango.vala \
	property.vala \
	separator.vala \
	switcher.vala \
	$(NULL)

ibus_ui_gtk3_LDADD = \
	$(AM_LDADD) \
	$(NULL)

CLEANFILES = \
	gtkpanel.xml \
	$(NULL)

EXTRA_DIST = \
	gtkpanel.xml.in \
	$(NULL)


-include $(top_srcdir)/git.mk
