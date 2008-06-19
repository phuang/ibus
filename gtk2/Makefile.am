# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
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
#
# $Id: $
#
INCLUDES =	\
	-I$(top_srcdir)

im_ibus_sources = \
	ibusim.c \
	ibusimclient.c \
	ibusimcontext.c \
	$(NULL)

im_ibus_private_headers = \
	ibusimcontext.h \
	ibusimclient.h \
	$(NULL)

immoduledir = $(libdir)/gtk-2.0/immodules
immodule_LTLIBRARIES = im-ibus.la

im_ibus_la_SOURCES = \
	$(im_ibus_sources) \
	$(im_ibus_private_headers) \
	$(NULL)

im_ibus_la_CFLAGS = 			\
	@GTK2_CFLAGS@ 			\
	@DBUS_CFLAGS@ 			\
	-DG_LOG_DOMAIN=\"IBUS\"	\
	$(NULL)

im_ibus_la_LDFLAGS = \
	@GTK2_LIBS@ 	\
	@DBUS_LIBS@ 	\
	-avoid-version	\
	-module			\
	$(NULL)

EXTRA_DIST =

