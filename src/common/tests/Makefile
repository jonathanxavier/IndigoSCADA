# $Id: Makefile,v 1.5 2002/10/25 06:48:35 kingofgib Exp $
CC              =  cc -DStandAlone
CCOPTFLAGS      = -O2
CCDEBUGFLAGS    = -g
CCWARNFLAGS     = 

INCLUDEDIR      = ../
LIBDIR		= ..

CFLAGS		= $(CCOPTFLAGS) $(CCWARNFLAGS) -I$(INCLUDEDIR) -L..
LLIBS		= ds

OBJ 	=  avltree.o hashtbl.o heap.o parray.o queue.o stack.o set.o
PROGS	=  avltree hashtbl heap parray queue stack set

.SUFFIXES:	.o

all:
	@echo "Use: make gcc on systems with GCC";\
        echo "Use: make sun on Solaris using UCBCC";\
        echo "Use: make unix on other Unix flavors (UNSUPPORTED)";\
        echo "See README for make instructions on the AIX and MSWindows"

gcc:
	@-$(MAKE) CC="gcc -pipe -DStandAlone" \
	CCWARNFLAGS="-Wall -pedantic -W -Wshadow \
	-Wnested-externs -Wwrite-strings -Wpointer-arith \
	-Wmissing-declarations -Wredundant-decls -Winline \
	-Wstrict-prototypes" $(PROGS)
sun unix:
	@-$(MAKE) CCOPTFLAGS="-O " $(PROGS)

avltree: avltree.o nullstr.o ../ds.h ../libds.a
	$(CC) $(CFLAGS) $@.o nullstr.o -l$(LLIBS) -o $@
hashtbl: hashtbl.o nullstr.o ../ds.h ../libds.a
	$(CC) $(CFLAGS) $@.o nullstr.o -l$(LLIBS) -o $@
heap:	heap.o nullstr.o ../ds.h ../libds.a
	$(CC) $(CFLAGS) $@.o nullstr.o -l$(LLIBS) -o $@
queue:	queue.o nullstr.o ../ds.h ../libds.a
	$(CC) $(CFLAGS) $@.o nullstr.o -l$(LLIBS) -o $@
stack:	stack.o nullstr.o ../ds.h ../libds.a
	$(CC) $(CFLAGS) $@.o nullstr.o -l$(LLIBS) -o $@
parray:	parray.o nullstr.o ../ds.h ../libds.a
	$(CC) $(CFLAGS) $@.o nullstr.o -l$(LLIBS) -o $@
set:	set.o nullstr.o ../ds.h ../libds.a
	$(CC) $(CFLAGS) $@.o nullstr.o -l$(LLIBS) -o $@

%.o:	%.c
	$(CC) $(CFLAGS) -c $*.c

.PHONY: clean

clean:
	@-echo "Cleaning directory "
	@-rm -f core *.e *.o *.obj *.ilk *.ncb *.pdb *.pch; \
	rm -f *~ $(PROGS) $(OBJ)
