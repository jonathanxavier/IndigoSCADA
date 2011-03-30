#-------------------------------------------------------------
#
# $Id: Makefile,v 1.7 2002/10/25 06:48:34 kingofgib Exp $
#
# Makefile for LibDS 2.0
#

CC		=  cc
CCOPTFLAGS	= -O2

CCEXTRAFLAGS	= -DStandAlone

OUTPUTDIR       = .
INCLUDEDIR      = . 

INCLUDEFLAGS    = -I$(INCLUDEDIR) -I. -I..

.SUFFIXES:	.e .o


LIBTARGET 	= libds
LIBHEADER 	= ds.h
HEADERS 	= 
OBJ 		=  avltree.o hashtbl.o heap.o parray.o queue.o stack.o set.o

LINC	= -I../include -I../../include
ARLIB	= $(LIBTARGET).a
SHARED	= 
SOLIB   = $(LIBTARGET).so
TARGET	= $(ARLIB)

all:
	@echo "Use: make gcc on systems with GCC";\
	echo "Use: make sun on Solaris using UCBCC";\
	echo "Use: make unix on other Unix types"; \
	echo "See README for make instructions on the AIX and MSWindows"
release:
	@-$(MAKE) CCFLAGS="$(CCOPTFLAGS) $(CCWARNFLAGS) $(CCEXTRAFLAGS)" \
	SHARED="$(SHARED)" $(TARGET)
debug:
	@-$(MAKE) CCFLAGS="$(CCDEBUGFLAGS) $(CCWARNFLAGS) $(CCEXTRAFLAGS)" \
	$(TARGET)

gcc:
	@-$(MAKE) CC="gcc -pipe" CCWARNFLAGS="-Wall -W -Wshadow \
	-Wnested-externs -Wwrite-strings -Wpointer-arith \
	-Wmissing-declarations -Wredundant-decls -Winline \
	-Wstrict-prototypes" SHARED="-shared" release
sun:
	@-$(MAKE) CCOPTFLAGS="-Xa -v" CCWARNFLAGS="" SHARED="-Bdynamic -G" release

unix:
	@-$(MAKE) CCOPTFLAGS="-O" release

$(ARLIB):   $(OBJ)
	ar r $(ARLIB) $(OBJ); ranlib $(ARLIB);\
	ld $(SHARED) -o $(SOLIB) $(OBJ) -lc 

install-dummy:
	install -m 664 -D libds.so /usr/local/lib/libds.so.2.0; \
	ln -s /usr/local/lib/libds.so.2.0 /usr/local/lib/libds.so; \
	install -m 644 -D libds.a /usr/local/lib/libds.a; \
	install -m 644 -D ds.h /usr/local/include/ds.h

.c.o:
	$(CC) $(INCLUDEFLAGS) $(LINC) $(CPPFLAGS) $(CCFLAGS) -c $*.c
.c.e:
	$(CC) -E $(INCLUDEFLAGS) $(LINC) $(CPPFLAGS) $(CCFLAGS) $*.c > $@

.PHONY: clean

clean:
	@-echo "Cleaning directory"
	@-rm -f *.core core *.e *.o *.obj *.ilk *.ncb *.pdb *.pch; \
	rm -f *~ $(SOLIB) $(ARLIB)

avltree.o: ./ds.h ./local.h

heap.o: ./ds.h ./local.h

parray.o: ./ds.h ./local.h

queue.o: ./ds.h ./local.h

stack.o: ./ds.h ./local.h

hashtbl.o: ./ds.h ./local.h

set.o: ./ds.h ./local.h
