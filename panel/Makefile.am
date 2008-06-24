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

panel_PYTHON = \
	candidatepanel.py \
	handle.py \
	image.py \
	lang.py \
	languagebar.py \
	main.py \
	menu.py \
	panel.py \
	toolitem.py \
	$(NULL)

paneldir = $(pkgdatadir)/panel

libexec_SCRIPTS = ibus-panel

CLEANFILES = \
	*.pyc \
	$(NULL)

EXTRA_DIST = \
	ibus-panel.in \
	$(NULL)

test:
	$(ENV) PYTHONPATH=$(top_srcdir) $(PYTHON) $(srcdir)/main.py
