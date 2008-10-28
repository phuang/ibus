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

libibus = $(top_builddir)/src/libibus.la

INCLUDES = \
	-I$(top_srcdir) \
	$(NULL)

noinst_PROGRAMS = ibus-daemon
ibus_daemon_DEPENDENCIES = \
	$(libibus) \
	$(NULL)
ibus_daemon_SOURCES = \
	main.c \
	dbusimpl.c \
	dbusimpl.h \
	ibusimpl.c \
	ibusimpl.h \
	inputcontext.c \
	inputcontext.h \
	engineproxy.c \
	engineproxy.h \
	panelproxy.c \
	panelproxy.h \
	configproxy.c \
	configproxy.h \
	factoryproxy.c \
	factoryproxy.h \
	server.c \
	server.h \
	connection.c \
	connection.h \
	matchrule.c \
	matchrule.h \
	$(NULL)
ibus_daemon_CFLAGS = \
	@X11_CFLAGS@ \
	@GLIB2_CFLAGS@ \
	@DBUS_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-I$(top_srcdir)/src \
	$(NULL)
ibus_daemon_LDADD = \
	@X11_LIBS@ \
	@GLIB2_LIBS@ \
	@DBUS_LIBS@ \
	$(libibus) \
	$(NULL)

$(libibus):
	$(MAKE) -C $(top_builddir)/src
