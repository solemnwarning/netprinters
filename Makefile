# NetPrinters - Makefile
# Copyright (C) 2008 Daniel Collins <solemnwarning@solemnwarning.net>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#	* Redistributions of source code must retain the above copyright
#	  notice, this list of conditions and the following disclaimer.
#
#	* Redistributions in binary form must reproduce the above copyright
#	  notice, this list of conditions and the following disclaimer in the
#	  documentation and/or other materials provided with the distribution.
#
#	* Neither the name of the author nor the names of its contributors may
#	  be used to endorse or promote products derived from this software
#	  without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

CC := gcc
DLLTOOL := dlltool
CFLAGS ?= -Wall -DWINVER=0x0500
INCLUDES ?= -I./src/
LIBS ?= -L./src/ -lwinspool -lws2_32
VPATH := ./src/

ifdef HOST
	CC := $(HOST)-$(CC)
	DLLTOOL := $(HOST)-$(DLLTOOL)
endif

.PHONY: all
all: netprinters.exe

.PHONY: clean
clean:
	rm -f src/*.o
	rm -f src/libwinspool.a
	rm -f netprinters.exe

netprinters.exe: libwinspool.a netprinters.o printer.o compare.o
	$(CC) $(CFLAGS) -o netprinters.exe src/netprinters.o src/compare.o $(LIBS)

netprinters.o: netprinters.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o src/netprinters.o src/netprinters.c

compare.o: compare.c compare.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o src/compare.o src/compare.c

libwinspool.a: winspool.def winspool.h
	$(DLLTOOL) -k -d src/winspool.def -l src/libwinspool.a

printer.o: printer.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o src/printer.o src/printer.c
