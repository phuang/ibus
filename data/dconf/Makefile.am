# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2010 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2011 Daiki Ueno <ueno@unixuser.org>
# Copyright (c) 2007-2011 Red Hat, Inc.
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

gsettings_schemas_in_files = org.freedesktop.ibus.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_schemas_in_files:.gschema.xml.in=.gschema.xml)
gsettingsconvertdir = $(datadir)/GConf/gsettings
dist_gsettingsconvert_DATA = ibus.convert
@GSETTINGS_RULES@
@INTLTOOL_XML_NOMERGE_RULE@

EXTRA_DIST = \
	$(gsettings_schemas_in_files) \
	make-dconf-override-db.sh \
	profile/ibus \
	$(NULL)

DISTCLEANFILES = \
	$(gsettings_SCHEMAS) \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(gsettings_schemas_in_files) \
	$(NULL)

CLEANFILES = \
	db/ibus \
	$(NULL)

dconfprofiledir = $(sysconfdir)/dconf
nobase_dconfprofile_DATA = db/ibus profile/ibus

org.freedesktop.ibus.gschema.xml.in: $(top_srcdir)/data/ibus.schemas.in
	$(AM_V_GEN) gsettings-schema-convert --force --gconf --xml \
		--schema-id "org.freedesktop.ibus" \
		--output $@ $<

db/ibus: $(srcdir)/make-dconf-override-db.sh $(gsettings_SCHEMAS)
	@$(MKDIR_P) db
	$(AM_V_GEN) $(srcdir)/make-dconf-override-db.sh $@ || \
		{ rc=$$?; $(RM) -rf $@; exit $$rc; }

