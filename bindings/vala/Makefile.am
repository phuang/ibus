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

vapidir = $(datadir)/vala/vapi
dist_vapi_DATA = \
	ibus-@IBUS_API_VERSION@.vapi \
	ibus-@IBUS_API_VERSION@.deps \
	$(NULL)

# Don't rebuild vapi every time gir is updated.
vapi_deps = \
	$(srcdir)/IBus-1.0.metadata \
	$(srcdir)/IBus-1.0-custom.vala \
	| \
	$(top_srcdir)/src/IBus-@IBUS_API_VERSION@.gir \
	$(NULL)

ibus-@IBUS_API_VERSION@.vapi: $(vapi_deps)
	$(AM_V_GEN) $(VAPIGEN) --library ibus-@IBUS_API_VERSION@ \
			--pkg gio-2.0 \
			--metadatadir=$(srcdir) \
			$(top_srcdir)/src/IBus-@IBUS_API_VERSION@.gir \
			$(srcdir)/IBus-1.0-custom.vala

EXTRA_DIST = \
	IBus-1.0.metadata \
	IBus-1.0-custom.vala \
	config.vapi \
	$(NULL)

MAINTAINERCLEANFILES = ibus-@IBUS_API_VERSION@.vapi

-include $(top_srcdir)/git.mk
