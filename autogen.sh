#!/bin/sh
set -e

aclocal --install -I m4
autoconf
automake --add-missing --copy
./configure "$@"
