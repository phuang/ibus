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

-include $(VAPIGEN_MAKEFILE)

vapi_deps = \
	IBus-1.0.metadata \
	IBus-1.0-custom.vala \
	$(top_builddir)/src/IBus-1.0.gir \
	$(NULL)

ibus-1.0.vapi: $(vapi_deps)

VAPIGEN_VAPIS = ibus-1.0.vapi

ibus_1_0_vapi_DEPS = gio-2.0
ibus_1_0_vapi_METADATADIRS = $(srcdir)
ibus_1_0_vapi_FILES = \
	$(top_builddir)/src/IBus-1.0.gir \
	$(srcdir)/IBus-1.0-custom.vala \
	$(NULL)

vapidir = $(datadir)/vala/vapi
vapi_DATA = $(VAPIGEN_VAPIS) $(VAPIGEN_VAPIS:.vapi=.deps)

MAINTAINERCLEANFILES = $(VAPIGEN_VAPIS)

EXTRA_DIST = \
	$(VAPIGEN_VAPIS) \
	IBus-1.0.metadata \
	IBus-1.0-custom.vala \
	ibus-1.0.deps \
	config.vapi \
	xi.vapi \
	$(NULL)

-include $(top_srcdir)/git.mk
