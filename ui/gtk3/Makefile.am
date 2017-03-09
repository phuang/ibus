# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2015 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2015-2017 Takao Fujwiara <takao.fujiwara1@gmail.com>
# Copyright (c) 2007-2017 Red Hat, Inc.
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

CLEANFILES =            \
    gtkpanel.xml        \
    $(NULL)

emoji_headers =         \
    ibusemojidialog.h   \
    $(NULL)

# References:
# libappindicator/src/notification-item.xml
# libappindicator/src/notification-watcher.xml
# kdelibs/kdeui/knotifications/src/org.kde.StatusNotifierItem.xml
# kdelibs/kdeui/knotifications/src/org.kde.StatusNotifierWatcher.xml
EXTRA_DIST =                            \
    $(emoji_headers)                    \
    IBusEmojiDialog-1.0.metadata        \
    gtkpanel.xml.in                     \
    ibus-emoji-dialog-1.0.deps          \
    notification-item.xml               \
    notification-watcher.xml            \
    $(NULL)

if ENABLE_EMOJI_DICT
AM_VALAFLAGS += --define=EMOJI_DICT

libibus_emoji_dialog = libibus-emoji-dialog-1.0.la

lib_LTLIBRARIES = $(libibus_emoji_dialog)

libibus_emoji_dialog_1_0_la_CFLAGS = $(AM_CFLAGS)
libibus_emoji_dialog_1_0_la_LDFLAGS =   \
    -no-undefined                       \
    -export-symbols-regex "ibus_.*"     \
    -version-info @LT_VERSION_INFO@     \
    $(NULL)
libibus_emoji_dialog_1_0_la_SOURCES =   \
    candidatearea.c                     \
    emojier.c                           \
    iconwidget.c                        \
    pango.c                             \
    separator.c                         \
    $(NULL)

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_SCANNER_ARGS =
INTROSPECTION_COMPILER_ARGS =      \
    --includedir=$(srcdir)         \
    --includedir=.                 \
    --includedir=$(top_srcdir)/src \
    $(NULL)

if HAVE_INTROSPECTION
ibusincludedir = $(includedir)/ibus-@IBUS_API_VERSION@
ibusinclude_HEADERS =                  \
    $(emoji_headers)                   \
    $(NULL)
introspection_sources =                \
    $(emoji_headers)                   \
    $(NULL)
IBusEmojiDialog-1.0.gir: $(libibus_emoji_dialog) Makefile
IBusEmojiDialog_1_0_gir_SCANNERFLAGS = \
    --pkg-export=ibus-1.0              \
    --pkg=gtk+-3.0                     \
    $(IBUS_GIR_SCANNERFLAGS)           \
    $(NULL)
IBusEmojiDialog-1.0.gir: $(libibus_emoji_dialog) Makefile
IBusEmojiDialog_1_0_gir_INCLUDES = Gtk-3.0 GLib-2.0 GObject-2.0 Gio-2.0
IBusEmojiDialog_1_0_gir_LIBS = $(libibus) $(libibus_emoji_dialog)
IBusEmojiDialog_1_0_gir_FILES =                      \
    $(addprefix $(srcdir)/,$(introspection_sources)) \
    $(NULL)
IBusEmojiDialog_1_0_gir_CFLAGS =       \
    -DIBUS_COMPILATION                 \
    -I$(srcdir)                        \
    -I$(builddir)                      \
    -I$(top_srcdir)/src                \
    $(NULL)
INTROSPECTION_GIRS = IBusEmojiDialog-1.0.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibsdir = $(libdir)/girepository-1.0
typelibs_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelibs_DATA)

if ENABLE_VAPIGEN
-include $(VAPIGEN_MAKEFILE)

ibus-emoji-dialog-1.0.vapi: $(INTROSPECTION_GIRS) IBusEmojiDialog-1.0.metadata

VAPIGEN_VAPIS = ibus-emoji-dialog-1.0.vapi

ibus_emoji_dialog_1_0_vapi_DEPS = gtk+-3.0 gio-2.0
ibus_emoji_dialog_1_0_vapi_METADATADIRS = $(srcdir)
ibus_emoji_dialog_1_0_vapi_FILES = $(INTROSPECTION_GIRS)

vapidir = $(datadir)/vala/vapi
vapi_DATA = $(VAPIGEN_VAPIS) $(VAPIGEN_VAPIS:.vapi=.deps)

MAINTAINERCLEANFILES = $(VAPIGEN_VAPIS)
# for make distclean
CONFIG_CLEAN_FILES = $(VAPIGEN_VAPIS)
EXTRA_DIST += $(VAPIGEN_VAPIS)

endif
endif
endif

-include $(top_srcdir)/git.mk
