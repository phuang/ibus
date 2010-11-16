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

INCLUDES =                \
	-I$(top_srcdir)/src   \
	-I$(top_builddir)/src \
	$(NULL)

AM_CFLAGS =                        \
	@GLIB2_CFLAGS@                 \
	@GIO2_CFLAGS@                  \
	@GTHREAD2_CFLAGS@              \
	-DG_LOG_DOMAIN=\"IBUS\"        \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBINDIR=\"@bindir@\"          \
	$(INCLUDES)                    \
	$(NULL)
AM_LDADD =                  \
	@GOBJECT2_LIBS@         \
	@GLIB2_LIBS@            \
	@GIO2_LIBS@             \
	@GTHREAD2_LIBS@         \
	$(libibus)              \
	$(NULL)

desktopdir = $(datadir)/applications
desktop_in_files = ibus.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

noinst_PROGRAMS = $(TESTS)
bin_PROGRAMS = ibus-daemon
ibus_daemon_DEPENDENCIES =  \
	$(libibus)              \
	$(NULL)
ibus_daemon_SOURCES =       \
	main.c                  \
	component.c             \
	component.h             \
	dbusimpl.c              \
	dbusimpl.h              \
	ibusimpl.c              \
	ibusimpl.h              \
	inputcontext.c          \
	inputcontext.h          \
	engineproxy.c           \
	engineproxy.h           \
	panelproxy.c            \
	panelproxy.h            \
	factoryproxy.c          \
	factoryproxy.h          \
	server.c                \
	server.h                \
	connection.c            \
	connection.h            \
	matchrule.c             \
	matchrule.h             \
	registry.c              \
	registry.h              \
	option.h                \
	marshalers.c            \
	marshalers.h            \
	types.h                 \
	$(NULL)
ibus_daemon_CFLAGS =        \
	$(AM_CFLAGS)            \
	$(NULL)
ibus_daemon_LDADD =         \
	$(AM_LDADD)             \
	$(NULL)

BUILT_SOURCES =             \
	marshalers.h            \
	marshalers.c            \
	$(NULL)

# test_registry_SOURCES =     \
# 	registry.c              \
# 	registry.h              \
# 	factoryproxy.c          \
# 	factoryproxy.h          \
# 	test-registry.c         \
# 	$(NULL)
# 
# test_matchrule_SOURCES =    \
# 	connection.c            \
# 	matchrule.c             \
# 	test-matchrule.c        \
# 	$(NULL)
# test_matchrule_CFLAGS =     \
# 	$(AM_CFLAGS)            \
# 	$(NULL)
# test_matchrule_LDADD =      \
# 	$(AM_LDADD)             \
# 	$(NULL)

# gen marshal
marshalers.h: marshalers.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=bus_marshal $(srcdir)/marshalers.list --header --internal > $@.tmp && \
	mv $@.tmp $@

marshalers.c: marshalers.h marshalers.list
	$(AM_V_GEN) (echo "#include \"marshalers.h\""; \
	$(GLIB_GENMARSHAL) --prefix=bus_marshal $(srcdir)/marshalers.list --body --internal) > $@.tmp && \
	mv $@.tmp $@

test_stress_SOURCES = \
	test-client.c \
	test-client.h \
	test-stress.c \
	$(NULL)
test_stress_CFLAGS = \
	$(AM_CFLAGS) \
	@GTK2_CFLAGS@ \
	@X11_CFLAGS@ \
	$(NULL)
test_stress_LDADD = \
	$(AM_LDADD) \
	@GTK2_LIBS@ \
	@X11_LIBS@ \
	$(NULL)

EXTRA_DIST =                \
	$(desktop_in_files)     \
	marshalers.list         \
	$(NULL)

DISTCLEANFILES =            \
	$(desktop_DATA)         \
	$(NULL)

$(libibus):
	$(MAKE) -C $(top_builddir)/src

test: ibus-daemon
	$(ENV_IBUS_TEST) \
		G_DEBUG=fatal_warnings \
		$(builddir)/ibus-daemon -v
