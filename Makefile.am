# Makefile config
AUTOMAKE_OPTIONS = subdir-objects
#ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CPPFLAGS = -Os -g -std=c++11 -Werror -Iinclude
AM_CFLAGS = -Os -g -Werror -Iinclude
LIBS = -lcrypt -lsqlite3 -ldl -lpthread

#CFLAGS=-Wall -I/usr/local/include -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
#LDFLAGS=-L/usr/local/lib -lX11 -lXtst -lxosd

minibar_core_source = \
src/cgi.cpp \
src/configure.cpp \
src/database.cpp \
src/jsoncpp.cpp \
src/minibar.cpp \
src/router.cpp \
src/utils.cpp \
include/cgi.h \
include/configure.h \
include/database.h \
include/json/json.h \
include/jsoncpp.h \
include/minibar.h \
include/param.h \
include/router.h \
include/utils.h

minibar_database_source = \
src/htpasswd.cpp \
include/htpasswd.h \
src/sqlite3db.cpp \
include/sqlite3db.h

minibar_fastcgi_source = \
src/fastcgi.cpp

minibar_test_source = \
src/test/main.cpp \
src/test/utils.cpp \
src/test/cgi.cpp \
src/test/configure.cpp \
src/test/htpasswd.cpp \
src/test/database.cpp \
src/test/minibar.cpp \
src/test/router.cpp

sbin_PROGRAMS = minibar-fastcgi
noinst_PROGRAMS = minibar-test

minibar_fastcgi_SOURCES = $(minibar_core_source) $(minibar_database_source) $(minibar_fastcgi_source)
minibar_fastcgi_LDADD = -lfcgi

minibar_test_SOURCES = $(minibar_core_source) $(minibar_database_source) $(minibar_test_source)
minibar_test_CXXFLAGS = -DUNITTEST
minibar_test_LDADD = -lgtest



unittest: minibar-test resources/test.db
	./minibar-test

# Sqlite3 SQL compilation support
%.db %.db: %.sql
	cat $< | sqlite3 $@

clean-local:
	rm -f resources/test.db
