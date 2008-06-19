#!/bin/sh
set -e
set -x

autopoint
libtoolize --automake --copy
aclocal -I m4
autoheader
automake --add-missing --copy
autoconf
./configure --enable-maintainer-mode $*
