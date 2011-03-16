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

ibus-@IBUS_API_VERSION@.vapi:
	vapigen --library ibus-@IBUS_API_VERSION@ \
			--pkg gio-2.0 \
			--metadata=ibus-@IBUS_API_VERSION@/ibus-@IBUS_API_VERSION@.metadata \
			ibus-@IBUS_API_VERSION@/ibus-@IBUS_API_VERSION@.gi \
			ibus-@IBUS_API_VERSION@/ibus-@IBUS_API_VERSION@-custom.vala

generate-vala:
	vala-gen-introspect ibus-@IBUS_API_VERSION@ ibus-@IBUS_API_VERSION@

-include $(top_srcdir)/git.mk
