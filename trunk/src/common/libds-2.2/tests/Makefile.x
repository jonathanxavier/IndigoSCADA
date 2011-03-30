CC              =  gcc -pipe -DStandAlone -ggdb
CCOPTFLAGS      = -O2
CCDEBUGFLAGS    = -ggdb
CCWARNFLAGS     = -Wall -pedantic -W -Wshadow -Wnested-externs \
                  -Wwrite-strings -Wpointer-arith \
                  -Wmissing-declarations -Wredundant-decls -Winline \
		  -Wstrict-prototypes

INCLUDEDIR      = ../
LIBDIR		= ..

CFLAGS		= $(CCOPTFLAGS) $(CCWARNFLAGS) -I $(INCLUDEDIR)
LLIBS		= $(LIBDIR)/libds.a

OBJ 	=  avltree.o hashtbl.o heap.o parray.o queue.o stack.o set.o
PROGS	=  avltree hashtbl heap parray queue stack set

.SUFFIXES:	.o

all:
	$(MAKE) $(PROGS)
linux:
	@-$(MAKE) CC="gcc -pipe -DStandAlone" \
	CCWARNFLAGS="-Wall -pedantic -W -Wshadow \
	-Wnested-externs -Wwrite-strings -Wpointer-arith \
	-Wmissing-declarations -Wredundant-decls -Winline \
	-Wstrict-prototypes" $(PROGS)

avltree: avltree.o nullstr.o ../ds.h $(LLIBS)
	$(CC) $(CFLAGS) $@.o nullstr.o $(LLIBS) -o $@
hashtbl: hashtbl.o nullstr.o ../ds.h $(LLIBS)
	$(CC) $(CFLAGS) $@.o nullstr.o $(LLIBS) -o $@
heap:	heap.o nullstr.o ../ds.h $(LLIBS)
	$(CC) $(CFLAGS) $@.o nullstr.o $(LLIBS) -o $@
queue:	queue.o nullstr.o ../ds.h $(LLIBS)
	$(CC) $(CFLAGS) $@.o nullstr.o $(LLIBS) -o $@
stack:	stack.o nullstr.o ../ds.h $(LLIBS)
	$(CC) $(CFLAGS) $@.o nullstr.o $(LLIBS) -o $@
parray:	parray.o nullstr.o ../ds.h $(LLIBS)
	$(CC) $(CFLAGS) $@.o nullstr.o $(LLIBS) -o $@
set:	set.o nullstr.o ../ds.h $(LLIBS)
	$(CC) $(CFLAGS) $@.o nullstr.o $(LLIBS) -o $@

%.o:	%.c
	$(CC) $(CFLAGS) -c $*.c

.PHONY: clean

clean:
	@-echo "Cleaning directory "
	@-rm -f core *.e *.o *.obj *.ilk *.ncb *.pdb *.pch; \
	rm -f *~ $(PROGS) $(OBJ)
