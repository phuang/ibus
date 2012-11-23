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

if ENABLE_TESTS
TESTS_DIR = tests
endif

SUBDIRS = . $(TESTS_DIR)

# libibus = libibus-@IBUS_API_VERSION@.la
libibus = libibus-1.0.la

# gobject introspection
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_SCANNER_ARGS =
INTROSPECTION_COMPILER_ARGS = \
    --includedir=$(srcdir)    \
    --includedir=.            \
    $(NULL)
INTROSPECTION_GIRS =
CLEANFILES =

# C preprocessor flags
AM_CPPFLAGS =                                   \
    -DG_LOG_DOMAIN=\"IBUS\"                     \
    @GLIB2_CFLAGS@                              \
    @GOBJECT2_CFLAGS@                           \
    @GIO2_CFLAGS@                               \
    -DIBUS_DATA_DIR=\"$(pkgdatadir)\"           \
    -DIBUS_COMPILATION                          \
    -DISOCODES_PREFIX=\"$(ISOCODES_PREFIX)\"    \
    $(NULL)

# ibus library
lib_LTLIBRARIES = $(libibus)

libibus_1_0_la_LIBADD =     \
    @GLIB2_LIBS@            \
    @GOBJECT2_LIBS@         \
    @GIO2_LIBS@             \
    $(NULL)
libibus_1_0_la_LDFLAGS =            \
    -no-undefined                   \
    -export-symbols-regex "ibus_.*" \
    -version-info @LT_VERSION_INFO@ \
    $(NULL)

ibus_sources =              \
    ibusshare.c             \
    ibusobject.c            \
    ibusserializable.c      \
    ibusproxy.c             \
    ibusservice.c           \
    ibusfactory.c           \
    ibusengine.c            \
    ibuserror.c             \
    ibustext.c              \
    ibuskeymap.c            \
    ibusattribute.c         \
    ibusattrlist.c          \
    ibusproperty.c          \
    ibusproplist.c          \
    ibuslookuptable.c       \
    ibusinputcontext.c      \
    ibusconfig.c            \
    ibusconfigservice.c     \
    ibuspanelservice.c      \
    ibusbus.c               \
    ibuskeynames.c          \
    ibuskeyuni.c            \
    ibushotkey.c            \
    ibusxml.c               \
    ibusenginedesc.c        \
    ibusobservedpath.c      \
    ibuscomponent.c         \
    ibusutil.c              \
    ibusenginesimple.c      \
    $(NULL)
libibus_1_0_la_SOURCES =    \
    ibusmarshalers.c        \
    ibusenumtypes.c         \
    $(ibus_sources)         \
    $(NULL)
ibus_marshalers_sources =   \
    ibusmarshalers.h        \
    ibusmarshalers.c        \
    $(NULL)
ibus_enumtypes_sources =    \
    ibusenumtypes.h         \
    ibusenumtypes.c         \
    $(NULL)
ibus_headers =              \
    ibus.h                  \
    ibusshare.h             \
    ibusdebug.h             \
    ibusobject.h            \
    ibusserializable.h      \
    ibusproxy.h             \
    ibusservice.h           \
    ibusfactory.h           \
    ibusengine.h            \
    ibuserror.h             \
    ibustext.h              \
    ibuskeymap.h            \
    ibusattribute.h         \
    ibusattrlist.h          \
    ibusproperty.h          \
    ibusproplist.h          \
    ibuslookuptable.h       \
    ibusinputcontext.h      \
    ibusconfig.h            \
    ibusconfigservice.h     \
    ibuspanelservice.h      \
    ibuskeysyms.h           \
    ibuskeysyms-compat.h    \
    ibuskeys.h              \
    ibustypes.h             \
    ibusbus.h               \
    ibushotkey.h            \
    ibusxml.h               \
    ibusenginedesc.h        \
    ibusobservedpath.h      \
    ibuscomponent.h         \
    ibusutil.h              \
    ibusenginesimple.h      \
    $(NULL)
ibusincludedir = $(includedir)/ibus-@IBUS_API_VERSION@
ibus_public_headers =       \
    $(ibus_headers)         \
    ibusenumtypes.h         \
    ibusversion.h           \
    $(NULL)
ibusinclude_HEADERS =       \
    $(ibus_public_headers)  \
    $(NULL)
ibus_privite_headers =       \
    ibusinternal.h           \
    keyname-table.h          \
    gtkimcontextsimpleseqs.h \
    $(NULL)
noinst_HEADERS =            \
    $(ibus_privite_headers) \
    $(NULL)

BUILT_SOURCES =                 \
    $(ibus_marshalers_sources)  \
    $(ibus_enumtypes_sources)   \
    $(NULL)

if HAVE_INTROSPECTION
introspection_sources =         \
    $(ibus_headers)             \
    $(ibus_sources)             \
    $(NULL)
built_introspection_sources =   \
    $(ibus_enumtypes_sources)   \
    ibusversion.h               \
    $(NULL)
IBus-1.0.gir: $(libibus) Makefile
IBus_1_0_gir_SCANNERFLAGS = --pkg-export=ibus-1.0 $(IBUS_GIR_SCANNERFLAGS)
IBus_1_0_gir_INCLUDES = GLib-2.0 GObject-2.0 Gio-2.0
IBus_1_0_gir_LIBS = $(libibus)
IBus_1_0_gir_FILES =                                            \
    $(addprefix $(srcdir)/,$(introspection_sources))            \
    $(addprefix $(builddir)/,$(built_introspection_sources))    \
    $(NULL)
IBus_1_0_gir_CFLAGS = -DIBUS_COMPILATION -I$(srcdir) -I$(builddir)
INTROSPECTION_GIRS += IBus-1.0.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibsdir = $(libdir)/girepository-1.0
typelibs_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelibs_DATA)
endif

# gen enum types
ibusenumtypes.h: $(ibus_headers) ibusenumtypes.h.template
	$(AM_V_GEN) ( top_builddir=`cd $(top_builddir) && pwd`; \
	cd $(srcdir) && $(GLIB_MKENUMS) --template ibusenumtypes.h.template $(ibus_headers) | \
		sed 's/i_bus_/ibus_/g' | \
		sed 's/I_TYPE_BUS_/IBUS_TYPE_/g') > \
	ibusenumtypes.h.tmp && mv ibusenumtypes.h.tmp ibusenumtypes.h

ibusenumtypes.c: $(ibus_headers) ibusenumtypes.c.template
	$(AM_V_GEN) ( top_builddir=`cd $(top_builddir) && pwd`; \
	cd $(srcdir) && $(GLIB_MKENUMS) --template ibusenumtypes.c.template $(ibus_headers) | \
		sed 's/i_bus_/ibus_/g' | \
		sed 's/I_TYPE_BUS_/IBUS_TYPE_/g') > \
	ibusenumtypes.c.tmp && mv ibusenumtypes.c.tmp ibusenumtypes.c

# gen marshal
ibusmarshalers.h: ibusmarshalers.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=_ibus_marshal $(srcdir)/ibusmarshalers.list --header --internal > $@.tmp && \
	mv $@.tmp $@

ibusmarshalers.c: ibusmarshalers.h ibusmarshalers.list
	$(AM_V_GEN) (echo "#include \"ibusmarshalers.h\""; \
	$(GLIB_GENMARSHAL) --prefix=_ibus_marshal $(srcdir)/ibusmarshalers.list --body --internal) > $@.tmp && \
	mv $@.tmp $@

EXTRA_DIST =                    \
    ibusversion.h.in            \
    ibusmarshalers.list         \
    ibusenumtypes.h.template    \
    ibusenumtypes.c.template    \
    $(NULL)

CLEANFILES +=                   \
    $(BUILT_SOURCES)            \
    stamp-ibusmarshalers.h      \
    stamp-ibusenumtypes.h       \
    $(NULL)

DISTCLEANFILES =                \
    ibusversion.h               \
    $(NULL)

-include $(top_srcdir)/git.mk
