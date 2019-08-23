# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2012 Daiki Ueno <ueno@unixuser.org>
# Copyright (c) 2014-2016 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2018-2019 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2012-2018 Red Hat, Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
# USA

NULL =

if ENABLE_PYTHON2
py2_compile = PYTHON=$(PYTHON2) $(SHELL) $(py_compile)
overrides2dir = $(py2overridesdir)
overrides2_DATA =				\
	gi/overrides/IBus.py			\
	$(NULL)
endif

overridesdir = $(pyoverridesdir)
overrides_PYTHON =				\
	gi/overrides/IBus.py			\
	$(NULL)

TESTS =

if ENABLE_TESTS
TESTS += test-override-ibus.py
endif

# IBus.Keymap() accesses keymap files
TESTS_ENVIRONMENT = \
	PYTHONPATH=$(top_srcdir)/tests:$${PYTHONPATH:+:$$PYTHONPATH} \
	LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$$LD_LIBRARY_PATH \
	GI_TYPELIB_PATH=$(top_builddir)/src:$$GI_TYPELIB_PATH \
	IBUS_KEYMAP_PATH=$(top_srcdir)/data/keymaps \
	$(NULL)

LOG_COMPILER = $(PYTHON) -B

EXTRA_DIST =					\
	gi/__init__.py				\
	gi/overrides/__init__.py		\
	test-override-ibus.py                   \
	$(NULL)

install-data-hook:
	@$(NORMAL_INSTALL)
if ENABLE_PYTHON2
	@for data in $(overrides2_DATA); do \
	    file=`echo $$data | sed -e 's|^.*/||'`; \
	    dlist="$$dlist $$file"; \
	done; \
	$(py2_compile) --destdir "$(DESTDIR)" \
	               --basedir "$(overrides2dir)" \
	               $$dlist
endif
	$(NULL)
uninstall-hook:
	@$(NORMAL_UNINSTALL)
if ENABLE_PYTHON2
	@list='$(overrides2_DATA)'; test -n "$(overrides2dir)" || list=; \
	py_files=`for p in $$list; do echo $$p; done | sed -e 's|^.*/||'`; \
	test -n "$$py_files" || exit 0; \
	dir='$(DESTDIR)$(overrides2dir)'; \
	pyc_files=`echo "$$py_files" | sed 's|$$|c|'`; \
	pyo_files=`echo "$$py_files" | sed 's|$$|o|'`; \
	for files in \
	    "$$py_files" \
	    "$$pyc_files" \
	    "$$pyo_files" \
	; do \
	    test -z "$$files" || rm -f "$$dir/$$files" || st=$$?; \
	done; \
	exit $$st
endif
	$(NULL)

-include $(top_srcdir)/git.mk
