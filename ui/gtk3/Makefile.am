# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2015 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2015 Red Hat, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
# USA

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

# force include config.h before gi18n.h.
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-include $(CONFIG_HEADER) \
	$(NULL)

AM_CFLAGS = \
	@GLIB2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@GTHREAD2_CFLAGS@ \
	@GTK3_CFLAGS@ \
	@X11_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DBINDIR=\"$(bindir)\" \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DIBUS_DISABLE_DEPRECATED \
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
	--target-glib="$(VALA_TARGET_GLIB_VERSION)" \
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

if ENABLE_APPINDICATOR
AM_VALAFLAGS += --define=INDICATOR
endif

if ENABLE_APPINDICATOR_ENGINE_ICON
AM_VALAFLAGS += --define=INDICATOR_ENGINE_ICON
endif

if ENABLE_EMOJI_DICT
AM_VALAFLAGS += --define=EMOJI_DICT
endif

libexec_PROGRAMS = ibus-ui-gtk3

ibus_ui_gtk3_SOURCES = \
	application.vala \
	candidatearea.vala \
	candidatepanel.vala \
	emojier.vala \
	handle.vala \
	iconwidget.vala \
	indicator.vala \
	keybindingmanager.vala \
	panel.vala \
	pango.vala \
	property.vala \
	propertypanel.vala \
	separator.vala \
	switcher.vala \
	xkblayout.vala \
	gen-notification-item.xml.c \
	gen-notification-watcher.xml.c \
	$(NULL)

ibus_ui_gtk3_LDADD = \
	$(AM_LDADD) \
	$(NULL)

gen-%.xml.c: %.xml
	echo "Building $@ from $<"
	echo "const char * _$(subst -,_,$(subst .,_,$(basename $(notdir $<)))) = " > $@
	sed -e "s:\":\\\\\":g" -e s:^:\": -e s:\$$:\\\\n\": $< >> $@
	echo ";" >> $@

CLEANFILES = \
	gtkpanel.xml \
	$(NULL)

# References:
# libappindicator/src/notification-item.xml
# libappindicator/src/notification-watcher.xml
# kdelibs/kdeui/knotifications/src/org.kde.StatusNotifierItem.xml
# kdelibs/kdeui/knotifications/src/org.kde.StatusNotifierWatcher.xml
EXTRA_DIST = \
	gtkpanel.xml.in \
	notification-item.xml \
	notification-watcher.xml \
	$(NULL)


-include $(top_srcdir)/git.mk
