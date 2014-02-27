#! /bin/sh
aclocal -I m4
autoheader
automake --foreign --add-missing --copy
autoconf
