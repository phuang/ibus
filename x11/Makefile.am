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

SUBDIRS = \
	IMdkit \
	$(NULL)

xim_PYTHON = \
	engine.py \
	factory.py \
	main.py \
	tables.py \
	test.py \
	$(NULL)

ximdir = $(datadir)/ibus/xim

IMdkit_DATA = \
	IMdkit.py \
	$(NULL)

IMdkit_LTLIBRARIES = _IMdkit.la

IMdkitdir = @pyexecdir@

_IMdkit_la_SOURCES = \
	$(NULL)

nodist__IMdkit_la_SOURCES = \
	IMdkit_wrap.c \
	$(NULL)

_IMdkit_la_CFLAGS = \
	@X11_CFLAGS@ \
	@GDK2_CFLAGS@ \
	@PYGOBJECT2_CFLAGS@ \
	@PYGTK2_CFLAGS@ \
	@GTK2_CFLAGS@ \
	@PYTHON_CFLAGS@ \
	-IIMdkit \
	$(NULL)

_IMdkit_la_LDFLAGS = \
	@X11_LIBS@ \
	@GDK2_LIBS@ \
	@PYGOBJECT2_LIBS@ \
	@PYGTK2_LIBS@ \
	@GTK2_LIBS@ \
	@PYTHON_LIBS@ \
	-rpath $(IMdkitdir) \
	-avoid-version \
	-module \
	IMdkit/libIMdkit.la \
	$(NULL)

#libexec_SCRIPTS = ibus-xim

IMdkit.py IMdkit_wrap.c: IMdkit.i
	$(SWIG) -python -I/usr/include -o IMdkit_wrap.c $(srcdir)/IMdkit.i

test: all
	$(ENV) PYTHONPATH=$(builddir)/.libs python test.py

test-ipython: all
	$(ENV) PYTHONPATH=$(builddir)/.libs ipython

EXTRA_DIST = \
	IMdkit.i \
	ibus-xim.in \
	$(NULL)

CLEANFILES = \
	IMdkit.py \
	IMdkit_wrap.* \
	*.pyc \
	$(MULL)

DISTCLEANFILES = \
	$(MULL)
