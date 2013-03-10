# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2010, Google Inc. All rights reserved.
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
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

libibus = $(top_builddir)/src/libibus-@IBUS_API_VERSION@.la

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	$(NULL)

AM_CFLAGS = \
	@GLIB2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@GTHREAD2_CFLAGS@ \
	$(INCLUDES) \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBINDIR=\"@bindir@\" \
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
	$(libibus) \
	$(NULL)

AM_VALAFLAGS = \
	--vapidir=$(top_builddir)/bindings/vala \
	--pkg=ibus-1.0 \
	$(NULL)

libexec_PROGRAMS = \
	ibus-engine-simple \
	$(NULL)

ibus_engine_simple_SOURCES = \
	main.vala \
	$(NULL)
ibus_engine_simple_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)
ibus_engine_simple_LDADD = \
	$(AM_LDADD) \
	$(NULL)
ibus_engine_simple_DEPENDENCIES = \
	$(libibus) \
	$(NULL)

component_DATA = \
	simple.xml \
	$(NULL)

componentdir = $(pkgdatadir)/component

CLEANFILES = \
	simple.xml \
	$(NULL)

EXTRA_DIST = \
	simple.xml.in \
	$(NULL)

simple.xml: simple.xml.in
	$(AM_V_GEN) sed \
		-e 's|@VERSION[@]|$(VERSION)|g' \
		-e 's|@libexecdir[@]|$(libexecdir)|g' $< > $@.tmp && \
		mv $@.tmp $@

$(libibus):
	$(MAKE) -C $(top_builddir)/src

-include $(top_srcdir)/git.mk
