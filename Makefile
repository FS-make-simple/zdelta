# Makefile for libzd 
# Original zlib Makefile file, modified to match libzd.
# Dimitre Trendafilov (2003)


# Makefile for zlib
# Copyright (C) 1995-1998 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h


# To compile and test, type:
# make test
#
# To compile the command line delta compressor, type:
# make zdc 
#
# To compile the command line delta decompressor, type:
# make zdu 
#
# For multiple reference file support compile with REFNUM=N option
# where N is the desired number (1..4) of referemce files. The default
# value is 1. 
# NOTE: The number of reference files comes at a price! Based on
# the selected number of reference files the compression may degrade, 
# and/or the memory usage may be increase. Do NOT add support for more 
# reference files than you need!!!
#
# To suppress the zdelta header and checksum compile with -DNO_ERROR_CHECK

# To install /usr/local/lib/libzd.*, /usr/local/include/zdlib.h, and
# /usr/local/include/zdconf.h type:
#    make install
# To install in $HOME instead of /usr/local, use:
#    make install prefix=$HOME

CC=gcc

CFLAGS= -O2 -W -Wall -pedantic -ansi -g -DREFNUM=2

LDSHARED=$(CC)
CPP=$(CC) -E

VER=2.1
PNAME=zd
PROGS=$(PNAME)elta
STATICLIB=lib$(PNAME).a
SHAREDLIB=lib$(PNAME).so.0
PLIBS=$(STATICLIB) $(SHAREDLIB)

AR=ar rc
RANLIB=ranlib
TAR=tar
SHELL=/bin/sh

prefix = /usr
dirbin = ${prefix}/bin
dirlib = ${prefix}/lib
dirinclude = ${prefix}/include
SRCS = src

OBJS = $(SRCS)/adler32.o $(SRCS)/deflate.o $(SRCS)/infblock.o \
       $(SRCS)/infcodes.o $(SRCS)/inffast.o $(SRCS)/inflate.o \
       $(SRCS)/inftrees.o $(SRCS)/infutil.o $(SRCS)/zd_mem.o \
       $(SRCS)/trees.o $(SRCS)/zdelta.o $(SRCS)/zutil.o

all: $(PLIBS) $(PROGS)

test: all
	@LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) ; export LD_LIBRARY_PATH; \
	echo 'libzd test, using Makefile as reference file';\
	echo '*** libzd test OK ***' | ./zdc Makefile | ./zdu Makefile

libzd: $(STATICLIB) $(SHAREDLIB)

$(STATICLIB): $(OBJS)
	$(AR) $@ $^
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

$(SHAREDLIB): $(OBJS)
	$(LDSHARED) -shared -Wl,-soname,$@ -o $@ $^

$(PROGS): $(SRCS)/zd.o $(SHAREDLIB)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -fv $(OBJS) *~ $(SRCS)/$(PNAME).o $(PROGS) $(PLIBS) so_locations

tags:	
	etags *.[ch]

depend:
	makedepend -- $(CFLAGS) -- *.[ch]

# DO NOT DELETE THIS LINE -- make depend depends on it.

$(SRCS)/adler32.o: $(SRCS)/zdconf.h $(SRCS)/zdlib.h
$(SRCS)/deflate.o: $(SRCS)/deflate.h $(SRCS)/zutil.h $(SRCS)/zdconf.h $(SRCS)/zdlib.h
$(SRCS)/infblock.o: $(SRCS)/infblock.h $(SRCS)/inftrees.h $(SRCS)/infcodes.h $(SRCS)/infutil.h $(SRCS)/zutil.h $(SRCS)/zdconf.h $(SRCS)/zdlib.h
$(SRCS)/infcodes.o: $(SRCS)/zutil.h $(SRCS)/zdconf.h $(SRCS)/zdlib.h $(SRCS)/inftrees.h $(SRCS)/infblock.h $(SRCS)/infcodes.h $(SRCS)/infutil.h $(SRCS)/inffast.h $(SRCS)/zdlib.h
$(SRCS)/inffast.o: $(SRCS)/zutil.h $(SRCS)/zdconf.h $(SRCS)/inftrees.h $(SRCS)/zdlib.h $(SRCS)/infblock.h $(SRCS)/infcodes.h $(SRCS)/infutil.h $(SRCS)/inffast.h $(SRCS)/zdlib.h
$(SRCS)/inflate.o: $(SRCS)/zutil.h $(SRCS)/zdconf.h $(SRCS)/infblock.h $(SRCS)/zdlib.h
$(SRCS)/inftrees.o: $(SRCS)/zutil.h $(SRCS)/zdconf.h $(SRCS)/inftrees.h $(SRCS)/zdlib.h
$(SRCS)/infutil.o: $(SRCS)/zutil.h $(SRCS)/zdconf.h $(SRCS)/infblock.h $(SRCS)/inftrees.h $(SRCS)/infcodes.h $(SRCS)/infutil.h $(SRCS)/zdlib.h
$(SRCS)/trees.o: $(SRCS)/deflate.h $(SRCS)/zutil.h  $(SRCS)/zdconf.h $(SRCS)/trees.h $(SRCS)/zdlib.h
$(SRCS)/zutil.o: $(SRCS)/zutil.h $(SRCS)/zdconf.h $(SRCS)/zdlib.h
$(SRCS)/zdelta.o: $(SRCS)/zutil.h $(SRCS)/tailor.h $(SRCS)/zdconf.h $(SRCS)/zdlib.h
$(SRCS)/zd_mem.o: $(SRCS)/zd_mem.h
$(SRCS)/zd.o: $(SRCS)/zd_mem.h $(SRCS)/zdlib.h

install: $(PROGS)
	install -d $(dirinclude)
	install -d $(dirlib)
	install -d $(dirbin)
	install -m 0644 $(SRCS)/zdlib.h $(dirinclude)
	install -m 0644 $(SRCS)/zdconf.h $(dirinclude)
	install -m 0644 $(STATICLIB) $(dirlib)
	install -m 0644 $(SHAREDLIB) $(dirlib)/$(SHAREDLIB).$(VER)
	install -m 0755 $(PROGS) $(dirbin)
	ln -s $(SHAREDLIB).$(VER) $(dirlib)/$(SHAREDLIB)

uninstall:
	rm -fv $(dirinclude)/zdlib.h $(dirinclude)/zdconf.h
	rm -fv $(dirlib)/$(STATICLIB) $(dirlib)/$(SHAREDLIB).$(VER) $(dirlib)/$(SHAREDLIB)
	rm -fv $(dirbin)/$(PROGS)
