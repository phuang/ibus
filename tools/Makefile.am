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

NULL =

libibus = $(top_builddir)/src/libibus-@IBUS_API_VERSION@.la

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	$(NULL)

# force include config.h before gi18n.h.
AM_CPPFLAGS = -include $(CONFIG_HEADER)

AM_CFLAGS = \
	@GLIB2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@GTHREAD2_CFLAGS@ \
	$(INCLUDES) \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DIBUS_DISABLE_DEPRECATED \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable \
	-Wno-unused-function \
	$(NULL)

AM_LDADD = \
	@GOBJECT2_LIBS@ \
	@GLIB2_LIBS@ \
	@GIO2_LIBS@ \
	@GTHREAD2_LIBS@ \
	$(libibus) \
	$(NULL)

AM_VALAFLAGS = \
	--vapidir=$(top_builddir)/bindings/vala \
	--vapidir=$(top_srcdir)/bindings/vala \
	--pkg=ibus-1.0 \
	--pkg=posix \
	--pkg=config \
	$(NULL)

bin_PROGRAMS = ibus

ibus_SOURCES = \
	main.vala \
	$(NULL)

ibus_LDADD = \
	$(AM_LDADD) \
	$(NULL)

bash_completion_DATA= \
	ibus.bash \
	$(NULL)
bash_completiondir=@datadir@/bash-completion/completions

man_one_in_files = ibus.1.in
man_one_files = $(man_one_in_files:.1.in=.1)
man_one_DATA =$(man_one_files:.1=.1.gz) 
man_onedir = $(datadir)/man/man1
%.1: %.1.in
	$(AM_V_GEN) sed \
		-e 's|@VERSION[@]|$(VERSION)|g' $< > $@.tmp && \
		mv $@.tmp $@
%.1.gz: %.1
	$(AM_V_GEN) gzip -c $< > $@.tmp && mv $@.tmp $@

EXTRA_DIST = \
	$(man_one_in_files) \
	ibus.bash \
	$(NULL)

CLEANFILES = \
	$(man_one_DATA) \
	$(man_one_files) \
	$(NULL)

-include $(top_srcdir)/git.mk
