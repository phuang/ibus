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

INCLUDES = \
	-I$(top_srcdir) \
	$(NULL)

# ibus library
lib_LTLIBRARIES = libibus.la
ibus_built_public_h_sources = \
	ibusmarshalers.h \
	ibusenumtypes.h \
	$(NULl)
ibus_built_c_sources = \
	ibusmarshalers.c \
	ibusenumtypes.c \
	$(NULL)
ibus_built_sources = \
	$(ibus_built_public_h_sources) \
	$(ibus_built_c_sources) \
	$(NULL)
ibus_public_h_sources = \
	ibus.h \
	ibusversion.h \
	ibusshare.h \
	ibusdebug.h \
	ibusobject.h \
	ibusserializable.h \
	ibusconnection.h \
	ibusserver.h \
	ibusproxy.h \
	ibusservice.h \
	ibusfactory.h \
	ibusengine.h \
	ibustext.h \
	ibuskeymap.h \
	ibusattribute.h \
	ibusattrlist.h \
	ibusproperty.h \
	ibusproplist.h \
	ibuslookuptable.h \
	ibusinputcontext.h \
	ibusconfig.h \
	ibusconfigservice.h \
	ibuspanelservice.h \
	ibusmessage.h \
	ibuspendingcall.h \
	ibuserror.h \
	ibuskeysyms.h \
	ibustypes.h \
	ibusbus.h \
	ibushotkey.h \
	ibusxml.h \
	ibusenginedesc.h \
	ibusobservedpath.h \
	ibuscomponent.h \
	ibusmainloop.h \
	$(NULL)
ibus_h_sources = \
	ibusinternal.h \
	ibusconfigprivate.h \
	keyname-table.h \
	$(ibus_public_h_sources) \
	$(NULL)
ibus_c_sources = \
	ibusshare.c \
	ibusinternal.c \
	ibusobject.c \
	ibusserializable.c \
	ibusconnection.c \
	ibusserver.c \
	ibusproxy.c \
	ibusservice.c \
	ibusfactory.c \
	ibusengine.c \
	ibustext.c \
	ibuskeymap.c \
	ibusattribute.c \
	ibusattrlist.c \
	ibusproperty.c \
	ibusproplist.c \
	ibuslookuptable.c \
	ibusinputcontext.c \
	ibusconfig.c \
	ibusconfigservice.c \
	ibuspanelservice.c \
	ibusmessage.c \
	ibuspendingcall.c \
	ibuserror.c \
	ibusbus.c \
	ibuskeynames.c \
	ibushotkey.c \
	ibusxml.c \
	ibusenginedesc.c \
	ibusobservedpath.c \
	ibuscomponent.c \
	ibusmainloop.c \
	$(NULL)
ibusincludedir = $(includedir)/ibus-1.0
ibusinclude_HEADERS = \
	$(ibus_public_h_sources) \
	$(ibus_built_public_h_sources) \
	$(NULL)
libibus_la_SOURCES = \
	$(ibus_h_sources) \
	$(ibus_c_sources) \
	$(NULL)
nodist_libibus_la_SOURCES = \
	$(ibus_built_h_sources) \
	$(ibus_built_c_sources) \
	$(NULL)
libibus_la_CFLAGS = \
	@X11_CFLAGS@ \
	@GLIB2_CFLAGS@ \
	@GOBJECT2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@DBUS_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DIBUS_DATA_DIR=\"$(pkgdatadir)\" \
	$(NULL)
libibus_la_LIBADD = \
	@X11_LIBS@ \
	@GLIB2_LIBS@ \
	@GOBJECT2_LIBS@ \
	@GIO2_LIBS@ \
	@DBUS_LIBS@ \
	$(NULL)
libibus_la_LDFLAGS = \
	-export-symbols-regex "ibus_.*" \
	-version-info @LT_VERSION_INFO@ \
	$(NULL)

BUILT_SOURCES = \
	$(ibus_built_sources) \
	$(NULL)


# test programs
DEPS = \
	libibus.la \
	$(NULL)
AM_CFLAGS = \
	@X11_CFLAGS@ \
	@GLIB2_CFLAGS@ \
	@GOBJECT2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@DBUS_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	$(NULL)
AM_LDADD = \
	$(DEPS) \
	@GLIB2_LIBS@ \
	@GOBJECT2_LIBS@ \
	$(NULL)

TESTS = \
	test-bus \
	test-text \
	test-keymap \
	test-keynames \
	test-attribute \
	test-lookuptable \
	test-global-engine \
	$(NULL)
noinst_PROGRAMS = $(TESTS)
test_bus_DEPENDENCIES = $(DEPS)
test_text_DEPENDENCIES = $(DEPS)
test_keymap_DEPENDENCIES = $(DEPS)
test_keynames_DEPENDENCIES = $(DEPS)
test_attribute_DEPENDENCIES = $(DEPS)
test_lookuptable_DEPENDENCIES = $(DEPS)
test_global_engine_DEPENDENCIES = $(DEPS)
test_bus_LDADD = $(AM_LDADD)
test_text_LDADD = $(AM_LDADD)
test_keymap_LDADD = $(AM_LDADD)
test_keynames_LDADD = $(AM_LDADD)
test_attribute_LDADD = $(AM_LDADD)
test_lookuptable_LDADD = $(AM_LDADD)
test_global_engine_LDADD = $(AM_LDADD)

# gen enum types
ibusenumtypes.h: stamp-ibusenumtypes.h
	@true
stamp-ibusenumtypes.h: $(ibus_public_h_sources) ibusenumtypes.h.template
	@( cd $(srcdir) && $(GLIB_MKENUMS) --template ibusenumtypes.h.template \
	$(ibus_public_h_sources) ) | sed 's/i_bus_/ibus_/g' | sed 's/I_TYPE_BUS_/IBUS_TYPE_/g' >> xgen-geth \
	&& (cmp -s xgen-geth ibusenumtypes.h || cp xgen-geth ibusenumtypes.h ) \
	&& rm -f xgen-geth \
	&& echo timestamp > $(@F)
ibusenumtypes.c: $(ibus_public_h_sources) ibusenumtypes.c.template
	$(AM_V_GEN) \
	( cd $(srcdir) && $(GLIB_MKENUMS) --template ibusenumtypes.c.template \
	$(ibus_public_h_sources) ) | sed 's/i_bus_/ibus_/g' | sed 's/I_TYPE_BUS_/IBUS_TYPE_/g' > xgen-getc \
	&& cp xgen-getc ibusenumtypes.c  \
	&& rm -f xgen-getc

# gen marshal
ibusmarshalers.h: stamp-ibusmarshalers.h
	@true
stamp-ibusmarshalers.h: ibusmarshalers.list
	@$(GLIB_GENMARSHAL) --prefix=ibus_marshal $(srcdir)/ibusmarshalers.list --header >> xgen-gmh \
	&& (cmp -s xgen-gmh ibusmarshalers.h || cp xgen-gmh ibusmarshalers.h) \
	&& rm -f xgen-gmh \
	&& echo timestamp > $(@F)
ibusmarshalers.c: ibusmarshalers.list
	$(AM_V_GEN) \
	(echo "#include \"ibusmarshalers.h\""; \
	 $(GLIB_GENMARSHAL) --prefix=ibus_marshal $(srcdir)/ibusmarshalers.list --body; \
	 echo ) >> xgen-gmc \
	&& cp xgen-gmc ibusmarshalers.c \
	&& rm -f xgen-gmc

EXTRA_DIST = \
	ibusversion.h.in \
	ibusmarshalers.list \
	ibusenumtypes.h.template \
	ibusenumtypes.c.template \
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES) \
	stamp-ibusmarshalers.h \
	stamp-ibusenumtypes.h \
	$(NULL)

DISTCLEANFILES = \
	ibusversion.h \
	$(NULL)
