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

DEPS = $(top_builddir)/src/libibus-@IBUS_API_VERSION@.la

AM_CPPFLAGS = \
	-g                                      \
	@GLIB2_CFLAGS@                          \
	@GIO2_CFLAGS@                           \
	-DIBUS_DISABLE_DEPRECATION_WARNINGS     \
	-I$(top_srcdir)/src                     \
	-I$(top_builddir)/src                   \
	$(NULL)

prog_ldadd = 			\
	@GLIB2_LIBS@        \
	@GIO2_LIBS@			\
	$(top_builddir)/src/libibus-@IBUS_API_VERSION@.la	\
	$(NULL)

noinst_PROGRAMS = $(TESTS)
TESTS = \
	ibus-bus                        \
	ibus-config                     \
	ibus-configservice              \
	ibus-factory                    \
	ibus-inputcontext               \
	ibus-inputcontext-create        \
	ibus-keynames                   \
	ibus-registry                   \
	ibus-serializable               \
	ibus-share                      \
	ibus-util                       \
	$(NULL)

if ENABLE_ENGINE
TESTS += ibus-engine-switch
endif

if ENABLE_GTK3
TESTS += ibus-compose
endif

TESTS_ENVIRONMENT = \
	top_builddir=$(top_builddir) \
	top_srcdir=$(top_srcdir) \
	builddir=$(builddir) \
	$(NULL)

LOG_COMPILER = $(srcdir)/runtest

EXTRA_DIST = runtest

ibus_bus_SOURCES = ibus-bus.c
ibus_bus_LDADD = $(prog_ldadd)

ibus_compose_SOURCES = ibus-compose.c
ibus_compose_CFLAGS = @GTK3_CFLAGS@
ibus_compose_LDADD = $(prog_ldadd) @GTK3_LIBS@

ibus_config_SOURCES = ibus-config.c
ibus_config_LDADD = $(prog_ldadd)

ibus_configservice_SOURCES = ibus-configservice.c
ibus_configservice_LDADD = $(prog_ldadd)

ibus_engine_switch_SOURCES = ibus-engine-switch.c
ibus_engine_switch_LDADD = $(prog_ldadd)

ibus_factory_SOURCES = ibus-factory.c
ibus_factory_LDADD = $(prog_ldadd)

ibus_inputcontext_SOURCES = ibus-inputcontext.c
ibus_inputcontext_LDADD = $(prog_ldadd)

ibus_inputcontext_create_SOURCES = ibus-inputcontext-create.c
ibus_inputcontext_create_LDADD = $(prog_ldadd)

ibus_keynames_SOURCES = ibus-keynames.c
ibus_keynames_LDADD = $(prog_ldadd)

ibus_registry_SOURCES = ibus-registry.c
ibus_registry_LDADD = $(prog_ldadd)

ibus_serializable_SOURCES = ibus-serializable.c
ibus_serializable_LDADD = $(prog_ldadd)

ibus_share_SOURCES = ibus-share.c
ibus_share_CFLAGS = @DBUS_CFLAGS@
ibus_share_LDADD = $(prog_ldadd) @DBUS_LIBS@

ibus_util_SOURCES = ibus-util.c
ibus_util_LDADD = $(prog_ldadd)

-include $(top_srcdir)/git.mk
