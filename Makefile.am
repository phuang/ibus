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
	daemon \
	gconf \
	ibus \
	launcher \
	ui \
	lib \
	util \
	client \
	setup \
	src \
	bus \
	icons \
	m4 \
	po \
	$(NULL)

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = \
	autogen.sh \
	ibus.spec.in \
	python-config.py \
	$(NULL)

noinst_DIST = \
	$(NULL)


DISTCLEANFILES = \
	po/stamp-it \
	$(NULL)

install-data-hook:
	$(MKDIR_P) $(DESTDIR)${pkgdatadir}/engine

DISTCHECK_CONFIGURE_FLAGS = \
	--enable-pygconf \
	$(NULL)

rpm: dist @PACKAGE_NAME@.spec
	rpmbuild -bb \
			--define "_sourcedir `pwd`" \
			--define "_builddir `pwd`" \
			--define "_srcrpmdir `pwd`" \
			--define "_rpmdir `pwd`" \
			--define "_specdir `pwd`" \
			@PACKAGE_NAME@.spec

srpm: dist @PACKAGE_NAME@.spec
	rpmbuild -bs \
			--define "_sourcedir `pwd`" \
			--define "_builddir `pwd`" \
			--define "_srcrpmdir `pwd`" \
			--define "_rpmdir `pwd`" \
			--define "_specdir `pwd`" \
			@PACKAGE_NAME@.spec

clean-rpm:
	$(RM) -r "`uname -i`"

clean-local: clean-rpm
