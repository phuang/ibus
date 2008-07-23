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

libexec_PROGRAMS = ibus-x11

ibus_x11_SOURCES = \
	main.c \
	gdk-private.c \
	$(NULL)

noinst_HEADERS = \
	gdk-private.h \
	$(NULL)

IMdkit = $(builddir)/IMdkit/libIMdkit.la

ibus_x11_LDADD = \
	@GTK2_LIBS@ \
	$(top_builddir)/gtk2/libibus-gtk.la \
	$(IMdkit) \
	$(NULL)
ibus_x11_CFLAGS = \
	@GTK2_CFLAGS@ \
	-I$(srcdir)/IMdkit \
	-I$(top_srcdir)/gtk2 \
	$(NULL)

SUBDIRS = IMdkit 
