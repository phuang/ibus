bin_PROGRAMS = ibus-x11

ibus_x11_SOURCES = \
	main.c \
	gdk-private.c \
	$(NULL)

noinst_HEADERS = \
	gdk-private.h \
	$(NULL)

IMdkit = $(builddir)/IMdkit/libIMdkit.la

ibus_x11_LDADD = \
	@GTK2_LIBS@ \
	$(top_builddir)/gtk2/libibus-gtk.la \
	$(IMdkit) \
	$(NULL)
ibus_x11_CFLAGS = \
	@GTK2_CFLAGS@ \
	-I$(srcdir)/IMdkit \
	-I$(top_srcdir)/gtk2 \
	$(NULL)

SUBDIRS = IMdkit 
