#/usr/bin/sh

touch NEWS README AUTHORS ChangeLog
autoreconf --force --install
./configure
