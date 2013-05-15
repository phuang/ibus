# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2013 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2007-2013 Red Hat, Inc.
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

ibussetup_PYTHON = \
	main.py \
	i18n.py \
	icon.py \
	enginecombobox.py \
	enginetreeview.py \
	engineabout.py \
	keyboardshortcut.py \
	$(NULL)

ibussetup_DATA = \
	setup.ui \
	$(NULL)

bin_SCRIPTS = ibus-setup
ibussetupdir = $(pkgdatadir)/setup

desktop_in_files = ibus-setup.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
desktopdir = $(datadir)/applications
@INTLTOOL_DESKTOP_RULE@

man_one_in_files = ibus-setup.1.in
man_one_files = $(man_one_in_files:.1.in=.1)
man_one_DATA =$(man_one_files:.1=.1.gz) 
man_onedir = $(datadir)/man/man1
%.1: %.1.in
	$(AM_V_GEN) sed \
		-e 's|@VERSION[@]|$(VERSION)|g' $< > $@.tmp && \
		mv $@.tmp $@
%.1.gz: %.1
	$(AM_V_GEN) gzip -c $< > $@.tmp && mv $@.tmp $@

CLEANFILES = \
	$(man_one_DATA) \
	$(man_one_files) \
	*.pyc \
	ibus-setup \
	$(NULL)

EXTRA_DIST = \
	$(man_one_in_files) \
	ibus-setup.in \
	setup.ui \
	ibus-setup.desktop.in \
	$(NULL)

DISTCLEANFILES = \
	$(desktop_DATA) \
	$(NULL)

test:
	$(ENV_IBUS_TEST) \
		PYTHONPATH=$(top_srcdir) \
		IBUS_PREFIX="@prefix@" \
		IBUS_LOCALEDIR="@localedir@" \
		$(PYTHON) \
		$(srcdir)/main.py

-include $(top_srcdir)/git.mk
