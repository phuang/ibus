anthy_PYTHON = \
	anthy.py \
	anthyengine.py \
	convertor.py \
	conv_tables.py \
	anthy_panel.py \
	__init__.py \
	$(NULL)

anthydir = $(datadir)/gik/ime/anthy

module_LTLIBRARIES = _anthy.la
moduledir = $(datadir)/gik/ime/anthy
_anthy_la_SOURCES = anthy_wrap.c
_anthy_la_CFLAGS = \
	@ANTHY_CFLAGS@ \
	`python-config --cflags`
_anthy_la_LDFLAGS = \
	@ANTHY_LIBS@ \
	`python-config --ldflags` \
	-avoid-version -module

anthy.py anthy_wrap.c: anthy.i
	swig -python $<

CLEANFILES = \
	anthy.py \
	anthy.pyc \
	anthy.pyo \
	anthy_wrap.c \
	$(NULL)
EXTRA_DIST = \
	anthy.i \
	test.py \
	$(NULL)
