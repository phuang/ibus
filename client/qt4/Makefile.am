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

EXTRA_DIST = \
	ibus-client.cpp \
	ibus-client.h \
	ibus-input-context.cpp \
	ibus-input-context.h \
	ibus.pro \
	im-ibus-qt.cpp \
	$(NULL)

if IBUS_BUILD_QT4
Makefile.qmake: ibus.pro
	$(QMAKE) -makefile -o Makefile.qmake $(srcdir)/ibus.pro

all-local: Makefile.qmake
	$(MAKE) -f Makefile.qmake $(AM_MAKEFLAGS) top_builddir=$(top_builddir) all

check-local: Makefile.qmake

clean-local: Makefile.qmake
	$(MAKE) -f Makefile.qmake $(AM_MAKEFLAGS) top_builddir=$(top_builddir) clean

distclean-local: Makefile.qmake
	$(MAKE) -f Makefile.qmake $(AM_MAKEFLAGS) top_builddir=$(top_builddir) distclean

install-exec-local: Makefile.qmake
	$(MAKE) -f Makefile.qmake $(AM_MAKEFLAGS) top_builddir=$(top_builddir) INSTALL_ROOT=$(DESTDIR) install

uninstall-local: Makefile.qmake
	$(MAKE) -f Makefile.qmake $(AM_MAKEFLAGS) top_builddir=$(top_builddir) INSTALL_ROOT=$(DESTDIR) uninstall

test: all
	QT_IM_MODULE=ibus kwrite
endif

