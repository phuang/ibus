#!/bin/sh
set -e
set -x

autopoint  --force
libtoolize --automake --copy --force
gtkdocize  --copy #--flavour=no-tmpl
aclocal -I m4 --force
autoheader --force
automake --add-missing --copy --force
autoconf --force
export CFLAGS="-Wall -g -O0 -Wl,--no-undefined"
export CXXFLAGS="$CFLAGS"
./configure --enable-maintainer-mode $*
