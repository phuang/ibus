# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2012 Daiki Ueno <ueno@unixuser.org>
# Copyright (c) 2011 Peng Huang <shawn.p.huang@gmail.com>
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
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA  02110-1301  USA

NULL =

overridesdir = $(pyoverridesdir)
overrides_PYTHON =				\
	gi/overrides/IBus.py			\
	$(NULL)

TESTS =

if ENABLE_TESTS
TESTS += test-override-ibus.py
endif

TESTS_ENVIRONMENT = \
	PYTHONPATH=$(top_srcdir)/tests:$${PYTHONPATH:+:$$PYTHONPATH} \
	LD_LIBRARY_PATH=$(top_builddir)/src/.libs:$$LD_LIBRARY_PATH \
	GI_TYPELIB_PATH=$(top_builddir)/src:$$GI_TYPELIB_PATH \
	$(PYTHON) -B \
	$(NULL)

EXTRA_DIST =					\
	gi/__init__.py				\
	gi/overrides/__init__.py		\
	test-override-ibus.py                   \
	$(NULL)

-include $(top_srcdir)/git.mk
