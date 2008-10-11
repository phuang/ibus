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

BUILT_SOURCES = \
	ibusmarshalers.h \
	ibusmarshalers.c \
	$(NULL)

# ibus library
lib_LTLIBRARIES = libibus.la

libibus_la_SOURCES = \
	ibus.h \
	ibusdebug.h \
	ibusinternel.h \
	ibusobject.c \
	ibusobject.h \
	ibusconnection.c \
	ibusconnection.h \
	ibusproxy.c \
	ibusproxy.h \
	ibusservice.c \
	ibusservice.h \
	ibusfactory.c \
	ibusfactory.h \
	ibusengine.c \
	ibusengine.h \
	ibusattribute.c \
	ibusattribute.h \
	ibuslookuptable.c \
	ibuslookuptable.h \
	ibuskeysyms.h \
	ibustypes.h \
	ibusmarshalers.h \
	ibusmarshalers.c \
	$(NULL)
libibus_la_CFLAGS = \
	@X11_CFLAGS@ \
	@GLIB2_CFLAGS@ \
	@DBUS_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	$(NULL)
libibus_la_LDFLAGS = \
	@X11_LIBS@ \
	@GLIB2_LIBS@ \
	@DBUS_LIBS@ \
	$(NULL)

# ibus programs
bin_PROGRAMS = ibus-test
ibus_test_SOURCES = \
	test.c \
	$(NULL)
ibus_test_CFLAGS = \
	@X11_CFLAGS@ \
	@GLIB2_CFLAGS@ \
	@DBUS_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	$(NULL)
ibus_test_LDFLAGS = \
	libibus.la \
	$(NULL)


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
	$(NULL)

CLEANFILES = $(BUILT_SOURCES) stamp-ibusmarshalers.h
