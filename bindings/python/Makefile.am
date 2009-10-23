# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2009 Peng Huang <shawn.p.huang@gmail.com>
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

# pyexec_PYTHON =
# pyexec_LTLIBRARIES = ibus.la
#
# INCLUDES = 					\
# 	-I$(top_srcdir)/src		\
# 	-I$(top_builddir)/src	\
# 	$(NULL)
#
# ibus_la_SOURCES = ibus.c ibusmodule.c
# ibus_la_CFLAGS = 		\
# 	@PYTHON_CFLAGS@ 	\
# 	@GLIB2_CFLAGS@ 		\
# 	@GOBJECT2_CFLAGS@ 	\
# 	@PYGOBJECT2_CFLAGS@	\
# 	@DBUS_CFLAGS@		\
# 	$(NULL)
# ibus_la_LDFLAGS = 	\
# 	@PYTHON_LIBS@ 	\
# 	@GLIB2_LIBS@ 	\
# 	@GOBJECT2_LIBS@ \
# 	@DBUS_LIBS@		\
# 	$(top_builddir)/src/libibus.la	\
# 	-avoid-version	\
# 	-module			\
# 	-export-symbols-regex "initibus"	\
# 	$(NULL)
#
# ibus-codegen.defs:
# 	(cd $(srcdir); \
# 	 ./gendefs.sh; ) > ibus-codegen.defs
#
# IBUS_DEFS = 					\
# 	ibus.defs					\
# 	ibus-types.defs				\
# 	ibus-func.defs				\
# 	ibus-virtual-methods.defs	\
# 	ibus.override				\
# 	$(NULL)
#
# ibus.c: $(IBUS_DEFS) ibus.override ibus-codegen.defs
#
# CLEANFILES = ibus.c ibus-codegen.defs
# EXTRA_DIST = 					\
# 	gendefs.sh					\
# 	common.h					\
# 	$(GIK_DEFS)					\
# 	$(NULL)
#
# PYGOBJECT_CODEGEN = pygobject-codegen-2.0
#
# .defs.c:
# 	(cd $(srcdir)\
# 	&& $(PYGOBJECT_CODEGEN) \
# 	-o $*.override \
# 	-I $(abs_builddir) \
# 	-p py$* $*.defs) > gen-$*.c \
# 	&& cp gen-$*.c $*.c \
# 	&& rm -f gen-$*.c
#
