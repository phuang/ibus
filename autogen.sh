#!/bin/sh
set -e
set -x

autopoint
libtoolize --automake --copy
aclocal -I m4
autoheader
gtkdocize --copy
automake --add-missing --copy
autoconf
./configure $*
