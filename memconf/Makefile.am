# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2010 Red Hat, Inc.
# Copyright (c) 2010 Google Inc. All rights reserved.
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

libibus = $(top_builddir)/src/libibus.la

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	$(NULL)

AM_CXXFLAGS = \
	@GLIB2_CFLAGS@ \
	@DBUS_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	$(INCLUDES) \
	$(NULL)
AM_LDADD = \
	@GOBJECT2_LIBS@ \
	@GLIB2_LIBS@ \
	@DBUS_LIBS@ \
	$(libibus) \
	$(NULL)

libexec_PROGRAMS = \
	ibus-memconf \
	$(NULL)

ibus_memconf_SOURCES = \
	main.cc \
	config.cc \
	config.h \
	$(NULL)
ibus_memconf_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)
ibus_memconf_LDADD = \
	$(AM_LDADD) \
	$(NULL)

component_DATA = \
	memconf.xml \
	$(NULL)
componentdir = $(pkgdatadir)/component

CLEANFILES = \
	config.pb.cc \
	config.pb.h \
	memconf.xml \
	*.pyc \
	$(NULL)

EXTRA_DIST = \
	memconf.xml.in.in \
	$(NULL)

memconf.xml: memconf.xml.in
	$(AM_V_GEN) \
	( \
		libexecdir=${libexecdir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

$(libibus):
	$(MAKE) -C $(top_builddir)/src

BUILT_SOURCES = \
	main.cc \
	$(NULL)

main.cc: ../gconf/main.c
	cp -p ../gconf/main.c main.cc
