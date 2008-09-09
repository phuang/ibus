#!/bin/sh
set -e
set -x

autopoint
libtoolize --automake --copy
aclocal -I m4
autoheader
automake --add-missing --copy
autoconf
export CFLAGS="-Wall -g -O0 -Wl,--no-undefined"
export CXXFLAGS="$CFLAGS"
./configure --enable-maintainer-mode $*
