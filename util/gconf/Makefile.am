INCLUDES = $(PYTHON_INCLUDES) $(PYGTK_CFLAGS)

defsdir = $(datadir)/pygtk/2.0/defs

if IBUS_BUILD_PYGCONF
# defs_DATA = gconf.defs
endif

common_ldflags = -module -avoid-version $(PYTHON_LDFLAGS)

# pkgpythondir = $(pyexecdir)/gtk-2.0
# pkgpyexecdir = $(pyexecdir)/gtk-2.0

# gconf binding

if IBUS_BUILD_PYGCONF
pkgpyexec_LTLIBRARIES = gconf.la
endif
gconf_la_CFLAGS = $(GCONF_CFLAGS)
gconf_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initgconf
gconf_la_LIBADD = $(PYGTK_LIBS) $(GCONF_LIBS) $(PYTHON_LIBS)
gconf_la_SOURCES = \
	gconfmodule.c \
	gconf-fixes.c \
	gconf-fixes.h \
	gconf-types.c \
	gconf-types.h
nodist_gconf_la_SOURCES = gconf.c
CLEANFILES = gconf.c
EXTRA_DIST = gconf.defs gconf.override gconf-arg-types.py
gconf.c: gconf.defs gconf.override gconf-arg-types.py

argtypesdir = $(datadir)/pygtk/2.0/argtypes

if IBUS_BUILD_PYGCONF
# argtypes_PYTHON = gconf-arg-types.py
endif

.defs.c:
	(cd $(srcdir) \
	 && $(PYGTK_CODEGEN) \
	    --py_ssize_t-clean \
	    --load-types gconf-arg-types.py \
	    --override $*.override \
	    --prefix py$* $*.defs) > gen-$*.c \
	&& cp gen-$*.c $*.c \
	&& rm -f gen-$*.c


EXTRA_DIST += wscript

