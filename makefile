# Copyright (c) 2013, Eric Anderton
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met: 
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution. 
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies, 
# either expressed or implied, of the FreeBSD Project.


IDIR =
CC=gcc
CPPC=g++

# note: use -s to strip symbolic information for smaller size
CFLAGS := $(CFLAGS) -Os -g -Isrc
CPPFLAGS := $(CPPFLAGS) -Os -g -std=c++11 -Isrc -Werror
LFLAGS := -Os -lstdc++ -ldl

SDIR := src
ODIR := obj
DIST := dist


CORE_SRC := cgi.cpp router.cpp database.cpp config.cpp utils.cpp jsoncpp.cpp minibar.cpp \
sqlite3.c sql.cpp

FASTCGI_SRC := fastcgi.cpp

TESTING_SRC := unittest.cpp


TARGETNAME = minibar
TARGET = $(DIST)/$(TARGETNAME)
GDBFILE = $(ODIR)/$(TARGETNAME).gdb

PACKAGEFILES := $(TARGET) 

# refinement

CORE_OBJ := $(CORE_SRC:%.c=%.o)
CORE_OBJ := $(CORE_OBJ:%.cpp=%.o)
CORE_OBJ := $(CORE_OBJ:%=$(ODIR)/%)
CORE_SRC := $(CORE_SRC:%=$(SDIR)/%)

FASTCGI_OBJ := $(FASTCGI_SRC:%.c=%.o)
FASTCGI_OBJ := $(FASTCGI_OBJ:%.cpp=%.o)
FASTCGI_OBJ := $(FASTCGI_OBJ:%=$(ODIR)/%)
FASTCGI_SRC := $(FASTCGI_SRC:%=$(SDIR)/%)

TESTING_OBJ := $(TESTING_SRC:%.c=%.o)
TESTING_OBJ := $(TESTING_OBJ:%.cpp=%.o)
TESTING_OBJ := $(TESTING_OBJ:%=$(ODIR)/%)
TESTING_SRC := $(TESTING_SRC:%=$(SDIR)/%)

ALL_SRC := $(CORE_SRC) $(FASTCGI_SRC) $(TESTING_SRC)
FASTCGI_SRC := $(FASTCGI_SRC) $(CORE_SRC)
TESTING_SRC := $(TESTING_SRC) $(CORE_SRC)
GDB_OBJ := $(CORE_OBJ:%.o=%.gdb)

# header dependencies
DEPEND = $(ODIR)/deps

$(DEPEND): $(ALL_SRC)
	-rm -rf $@
	$(CPPC) $(CPPFLAGS) -MM $(filter %.cpp,$(ALL_SRC)) >> $@
	$(CC) $(CFLAGS) -MM $(filter %.c,$(ALL_SRC)) >> $@

depend: $(DEPEND)
include $(DEPEND)

## compilation
$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CPPC) -c -o $@ $(CPPFLAGS) $<

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $(CFLAGS) $<


#$(DIST)/%.db: $(SDIR)/%.sql
#	cat $< | sqlite3 $@

$(ODIR)/%.gdb: $(SDIR)/%.c
	grep -nHi '//@breakpoint' $< | sed -n 's/^\(.*:.*\):.*/break \1/p' > $@

$(ODIR)/%.gdb: $(SDIR)/%.cpp
	grep -nHi '//@breakpoint' $< | sed -n 's/^\(.*:.*\):.*/break \1/p' > $@

## build the GDB script to use for the target
$(GDBFILE): $(DEBUG_TARGET) $(GDB_OBJ)
	-rm -f $@
	for f in $<; do cat $$f >> $@; done
	echo "symbol $(DEBUG_TARGET)" >> $@
	echo "continue" >> $@

## binary targets
$(TARGET)-fastcgi: depend $(CORE_OBJ) $(FASTCGI_OBJ)
	$(CC) -o $@ $(filter %.o,$^) $(LFLAGS) -lpthread -lfcgi

$(TARGET)-test: depend $(CORE_OBJ) $(TESTING_OBJ)
	$(CC) -o $@ $(filter %.o,$^) $(LFLAGS) -lpthread -lgtest

## packaging
compile: $(TARGET)-fastcgi

package: compile

# user targets
install: package stop
	sudo cp $(DIST)/minibar-fastcgi /usr/local/bin
	make restart

reinstall: uninstall install

stop:
	sudo service lighttpd stop

restart:
	sudo service lighttpd restart

tail:
	#sudo tail -f /var/log/lighttpd/minibar.log
	sudo tail -f /var/log/lighttpd/error.log

GETPID := "$$(ps aux | grep minibar-fastcgi | awk '{print $$2}' | head -1)"
attach: $(GDBFILE)
	sudo gdb --pid=$(GETPID) -x $(GDBFILE)

debug: $(GDBFILE) reinstall attach

clean:
	-rm -rf $(ODIR)/* $(DIST)/*

## FastCGI test target
test: CPPFLAGS += -DUNITTEST
test: CPP_SRC += unittest.cpp
test: $(TARGET)-test $(GDBFILE)
	cat resources/test.sql | sqlite3 dist/test.db
	jsonlint -s -v resources/test.mini
	cp resources/test.mini dist/test.mini
	gdb $(TARGET)-test -x $(GDBFILE)



all: package

.PHONY: depend compile package uninstall install reinstall tail attach debug clean test all

