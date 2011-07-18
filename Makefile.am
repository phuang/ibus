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

if ENABLE_PYTHON
PYTHON_DIRS =       \
	ibus            \
	ui              \
	setup           \
	$(NULL)
endif

if ENABLE_GCONF
GCONF_DIRS =        \
	gconf           \
	$(NULL)
endif

if ENABLE_DAEMON
DAEMON_DIRS =       \
	bus             \
	$(NULL)
endif

if ENABLE_MEMCONF
MEMCONF_DIRS = \
	memconf \
	$(NULL)
endif

if ENABLE_DCONF
DCONF_DIRS = \
	dconf \
	$(NULL)
endif

SUBDIRS =           \
	src             \
	util            \
	client          \
	data            \
	m4              \
	po              \
	docs            \
	bindings        \
	$(DAEMON_DIRS)  \
	$(PYTHON_DIRS)  \
	$(GCONF_DIRS)   \
	$(MEMCONF_DIRS) \
	$(DCONF_DIRS)   \
	$(NULL)

ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ibus-@IBUS_API_VERSION@.pc

ibus_pc_in = ibus-@IBUS_API_VERSION@.pc.in
EXTRA_DIST = \
	autogen.sh \
	$(ibus_pc_in) \
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
	--enable-gtk-doc \
	--disable-schemas-install \
	--disable-introspection \
	$(NULL)

dist-hook:
	if test -d .git ; then \
		git log --name-status --date=iso > $(distdir)/ChangeLog ; \
	fi

rpm: dist @PACKAGE_NAME@.spec
	rpmbuild -bb \
			--define "_sourcedir `pwd`" \
			--define "_builddir `pwd`/rpm" \
			--define "_srcrpmdir `pwd`/rpm" \
			--define "_rpmdir `pwd`"/rpm \
			--define "_specdir `pwd`/rpm" \
			@PACKAGE_NAME@.spec

srpm: dist @PACKAGE_NAME@.spec
	rpmbuild -bs \
			--define "_sourcedir `pwd`" \
			--define "_builddir `pwd`/rpm" \
			--define "_srcrpmdir `pwd`/rpm" \
			--define "_rpmdir `pwd`/rpm" \
			--define "_specdir `pwd`/rpm" \
			@PACKAGE_NAME@.spec

.PHONY: debian/changelog
debian/changelog:
	$(AM_V_GEN) \
	( \
		. /etc/lsb-release; \
		date=`date -R`; \
		version=@VERSION@; \
		serie=$(serie); \
		if test -z "$$serie"; then \
		  serie=$$DISTRIB_CODENAME; \
		fi; \
		if test -z "$$release"; then \
		  release=1; \
		fi; \
		s=`cat debian/changelog.in`; \
		eval "echo \"$${s}\""; \
	) > $@

ppa: dist debian/changelog
	$(AM_V_GEN) \
	( \
		mkdir ppa; \
		cd ppa; \
		tar zxvf ../$(distdir).tar.gz ; \
		cd $(distdir); \
		cp -a ../../debian . ; \
		cd debian; \
		debuild -S -sa ; \
	)

dpkg: dist debian/changelog
	$(AM_V_GEN) \
	( \
		mkdir ppa; \
		cd ppa; \
		tar zxvf ../$(distdir).tar.gz ; \
		cd $(distdir); \
		cp -a ../../debian . ; \
		cd debian; \
		debuild -b -us -uc; \
	)

clean-rpm:
	$(RM) -r "`uname -i`"

clean-local: clean-rpm

git-tag:
	git tag -s @PACKAGE_VERSION@

git-clean-tree:
	git clean -d -f -x

-include $(top_srcdir)/git.mk
