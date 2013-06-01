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
CPPFLAGS := $(CPPFLAGS) -Os -g -std=c++11 -Isrc -Werror -DUNITTEST
CFLAGS := $(CFLAGS) -Os -g -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION
LFLAGS := -Os -lstdc++ -lfcgi

SDIR := src
ODIR := obj
DIST := dist

C_SRC := sqlite3.c
CPP_SRC := router.cpp config.cpp minibar.cpp unittest.cpp utils.cpp main.cpp jsoncpp.cpp sql.cpp
SRC := $(C_SRC) $(CPP_SRC)

TARGETNAME := minibar
TARGET := $(DIST)/$(TARGETNAME)
GDBFILE := $(ODIR)/$(TARGETNAME).gdb

PACKAGEFILES := $(TARGET) 

# refinement
SRCS := $(patsubst %,$(SDIR)/%,$(SRC))
C_SRCS := $(patsubst %,$(SDIR)/%,$(C_SRC))
CPP_SRCS := $(patsubst %,$(SDIR)/%,$(CPP_SRC))
OBJ := $(patsubst %,$(ODIR)/%,$(SRC)) 
OBJ := $(patsubst %.c,%.o,$(OBJ))
OBJ := $(patsubst %.cpp,%.o,$(OBJ))
BRK := $(patsubst %.o,%.brk,$(OBJ))

# header dependencies
DEPEND = $(ODIR)/deps

$(DEPEND): $(SRCS)
	-rm -rf $@
	$(CPPC) $(CPPFLAGS) -MM $(CPP_SRCS) >> $@
	$(CC) $(CFLAGS) -MM $(C_SRCS) >> $@

depend: $(DEPEND)
include $(DEPEND)

# c/cpp compilation
$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CPPC) -c -o $@ $(CPPFLAGS) $<

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $(CFLAGS) $<

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $^ $(LFLAGS)

$(DIST)/%.db: $(SDIR)/%.sql
	cat $< | sqlite3 $@

# gather breakpoint information for GDB
$(ODIR)/%.brk: $(SDIR)/%.c
	grep -nHi '//@breakpoint' $< | sed -n 's/^\(.*:.*\):.*/break \1/p' > $@

$(ODIR)/%.brk: $(SDIR)/%.cpp
	grep -nHi '//@breakpoint' $< | sed -n 's/^\(.*:.*\):.*/break \1/p' > $@

# build the GDB script to use for the target
$(GDBFILE): $(BRK)
	-rm -f $@
	for f in $(BRK); do cat $$f >> $@; done
	echo "symbol $(TARGET)" >> $@
	echo "continue" >> $@

# packaging
compile: depend $(OBJS) $(PACKAGEFILES)

package: compile

# user targets
install: package stop
#	sudo mkdir -p /var/opt/minibar
	sudo cp $(DIST)/minibar /usr/local/bin
#	sudo chown -R www-data:www-data /var/opt/minibar
	make restart

reinstall: uninstall install

stop:
	sudo service lighttpd stop

restart:
	sudo service lighttpd restart

tail:
	#sudo tail -f /var/log/lighttpd/minibar.log
	sudo tail -f /var/log/lighttpd/error.log

GETPID := "$$(ps aux | grep minibar | awk '{print $$2}' | head -1)"
attach:
	sudo gdb --pid=$(GETPID) -x $(GDBFILE)

debug: $(GDBFILE) reinstall attach

clean:
	-rm -rf $(ODIR)/* $(DIST)/*

test: $(GDBFILE) package
	#gdb $(TARGET) -x $(GDBFILE)
	$(TARGET)

all: package

.PHONY: depend compile package uninstall install reinstall tail attach debug clean test all

