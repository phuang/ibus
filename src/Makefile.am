# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
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
	ibusattribute.h \
	ibusproperty.h \
	ibuslookuptable.h \
	ibusinputcontext.h \
	ibusconfig.h \
	ibusconfigservice.h \
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
	$(NULL)
ibus_h_sources = \
	ibusinternal.h \
	ibusconfigprivate.h \
	keyname-table.h \
	$(ibus_public_h_sources) \
	$(ibus_built_public_h_sources) \
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
	ibusattribute.c \
	ibusproperty.c \
	ibuslookuptable.c \
	ibusinputcontext.c \
	ibusconfig.c \
	ibusconfigservice.c \
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
	$(ibus_built_c_sources) \
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
libibus_la_CFLAGS = \
	@X11_CFLAGS@ \
	@GLIB2_CFLAGS@ \
	@GOBJECT2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@DBUS_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	$(NULL)
libibus_la_LDFLAGS = \
	@X11_LIBS@ \
	@GLIB2_LIBS@ \
	@GOBJECT2_LIBS@ \
	@GIO2_LIBS@ \
	@DBUS_LIBS@ \
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
AM_LDFLAGS = \
	$(DEPS) \
	$(NULL)

TESTS = \
	test-text \
	test-keynames \
	test-attribute \
	test-lookuptable \
	$(NULL)
noinst_PROGRAMS = $(TESTS)
test_text_DEPENDENCIES = $(DEPS)
test_keynames_DEPENDENCIES = $(DEPS)
test_attribute_DEPENDENCIES = $(DEPS)
test_lookuptable_DEPENDENCIES = $(DEPS)

# gen enum types
ibusenumtypes.h: stamp-ibusenumtypes.h
	@true
stamp-ibusenumtypes.h: @REBUILD@ $(ibus_public_h_sources) ibusenumtypes.h.template
	( cd $(srcdir) && $(GLIB_MKENUMS) --template ibusenumtypes.h.template \
	$(ibus_public_h_sources) ) | sed 's/i_bus_/ibus_/g' | sed 's/I_TYPE_BUS_/IBUS_TYPE_/g' >> xgen-geth \
	&& (cmp -s xgen-geth ibusenumtypes.h || cp xgen-geth ibusenumtypes.h ) \
	&& rm -f xgen-geth \
	&& echo timestamp > $(@F)
ibusenumtypes.c: @REBUILD@ $(ibus_public_h_sources) ibusenumtypes.c.template
	( cd $(srcdir) && $(GLIB_MKENUMS) --template ibusenumtypes.c.template \
	$(ibus_public_h_sources) ) | sed 's/i_bus_/ibus_/g' | sed 's/I_TYPE_BUS_/IBUS_TYPE_/g' > xgen-getc \
	&& cp xgen-getc ibusenumtypes.c  \
	&& rm -f xgen-getc

# gen marshal
ibusmarshalers.h: stamp-ibusmarshalers.h
	@true
stamp-ibusmarshalers.h: ibusmarshalers.list
	$(GLIB_GENMARSHAL) --prefix=ibus_marshal $(srcdir)/ibusmarshalers.list --header >> xgen-gmh \
	&& (cmp -s xgen-gmh ibusmarshalers.h || cp xgen-gmh ibusmarshalers.h) \
	&& rm -f xgen-gmh \
	&& echo timestamp > $(@F)
ibusmarshalers.c: ibusmarshalers.list
	(echo "#include \"ibusmarshalers.h\""; \
	 $(GLIB_GENMARSHAL) --prefix=ibus_marshal $(srcdir)/ibusmarshalers.list --body; \
	 echo ) >> xgen-gmc \
	&& cp xgen-gmc ibusmarshalers.c \
	&& rm -f xgen-gmc

EXTRA_DIST = \
	ibusmarshalers.list \
	ibusenumtypes.h.template \
	ibusenumtypes.c.template \
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES) \
	stamp-ibusmarshalers.h \
	stamp-ibusenumtypes.h \
	$(NULL)
