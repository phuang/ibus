# vim:set noet ts=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2013 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (c) 2015-2017 Takao Fujiwara <takao.fujiwara1@gmail.com>
# Copyright (c) 2007-2017 Red Hat, Inc.
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

libibus = $(top_builddir)/src/libibus-@IBUS_API_VERSION@.la
libibus_emoji_dialog = \
    $(top_builddir)/ui/gtk3/libibus-emoji-dialog-@IBUS_API_VERSION@.la

# force include config.h before gi18n.h.
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-include $(CONFIG_HEADER) \
	$(NULL)

AM_CFLAGS = \
	@GLIB2_CFLAGS@ \
	@GIO2_CFLAGS@ \
	@GTHREAD2_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
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
	--target-glib="$(VALA_TARGET_GLIB_VERSION)" \
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
man_onedir = $(mandir)/man1
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

if ENABLE_EMOJI_DICT
if ENABLE_UI
AM_VALAFLAGS += \
    --define=EMOJI_DICT \
    $(NULL)
endif
endif

-include $(top_srcdir)/git.mk
