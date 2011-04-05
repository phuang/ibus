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

INCLUDES =              \
	-g                  \
	@GLIB2_CFLAGS@      \
	@GIO2_CFLAGS@       \
	-I$(top_srcdir)/src \
	$(NULL)

prog_ldadd = 			\
	@GLIB2_LIBS@        \
	@GIO2_LIBS@			\
	$(top_builddir)/src/libibus-@IBUS_API_VERSION@.la	\
	$(NULL)

noinst_PROGRAMS = $(TESTS)
TESTS =               \
	ibus-bus          \
	ibus-inputcontext \
	ibus-inputcontext-create \
	ibus-keynames     \
	ibus-serializable \
	ibus-share        \
	ibus-factory      \
	ibus-configservice\
	$(NULL)

ibus_bus_SOURCES = ibus-bus.c
ibus_bus_LDADD = $(prog_ldadd)

ibus_inputcontext_SOURCES = ibus-inputcontext.c
ibus_inputcontext_LDADD = $(prog_ldadd)

ibus_inputcontext_create_SOURCES = ibus-inputcontext-create.c
ibus_inputcontext_create_LDADD = $(prog_ldadd)

ibus_keynames_SOURCES = ibus-keynames.c
ibus_keynames_LDADD = $(prog_ldadd)

ibus_serializable_SOURCES = ibus-serializable.c
ibus_serializable_LDADD = $(prog_ldadd)

ibus_share_SOURCES = ibus-share.c
ibus_share_CFLAGS = @DBUS_CFLAGS@
ibus_share_LDADD = $(prog_ldadd) @DBUS_LIBS@

ibus_factory_SOURCES = ibus-factory.c
ibus_factory_LDADD = $(prog_ldadd)

ibus_configservice_SOURCES = ibus-configservice.c
ibus_configservice_LDADD = $(prog_ldadd)

-include $(top_srcdir)/git.mk
