#-------------------------------------------------------------
#       
#
#CC              =  gcc -pipe
#CCOPTFLAGS      = -O2
#CCDEBUGFLAGS    = -ggdb
#CCWARNFLAGS     = -Wall -pedantic -W -Wshadow -Wnested-externs \
#                  -Wwrite-strings -Wpointer-arith \
#                  -Wmissing-declarations -Wredundant-decls -Winline \
#		  -Wstrict-prototypes

CC		=  cc
CCOPTFLAGS	= -O2

CCEXTRAFLAGS	= -DStandAlone

OUTPUTDIR       = .
INCLUDEDIR      = . 

INCLUDEFLAGS    = -I $(INCLUDEDIR) -I . -I ..

# Default compile is release
CCFLAGS         = $(CCOPTFLAGS) $(CCWARNFLAGS) $(CCEXTRAFLAGS)

#CCFLAGS        = $(CCDEBUGFLAGS) $(CCWARNFLAGS) $(CCEXTRAFLAGS)

.SUFFIXES:	.e .o


LIBTARGET 	= libds
LIBHEADER 	= ds.h
HEADERS 	= 
OBJ 		=  avltree.o hashtbl.o heap.o parray.o queue.o stack.o set.o

LINC	= -I ../include -I ../../include
ARLIB	= $(LIBTARGET).a
SOLIB   = $(LIBTARGET).so
TARGET	= $(ARLIB)

all:
	$(MAKE) release
release:
	@-$(MAKE) CCFLAGS="$(CCOPTFLAGS) $(CCWARNFLAGS) $(CCEXTRAFLAGS)" \
	$(TARGET)
debug:
	@-$(MAKE) CCFLAGS="$(CCDEBUGFLAGS) $(CCWARNFLAGS) $(CCEXTRAFLAGS)" \
	$(TARGET)
linux:
	$(MAKE) CC="gcc -pipe" CCWARNFLAGS="-Wall"
$(ARLIB):   $(OBJ)
	@-ar r $(ARLIB) $(OBJ); ranlib $(ARLIB);\
	ld -shared -o $(SOLIB) $(OBJ) -lc 

install:
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
	@-rm -f core *.e *.o *.obj *.ilk *.ncb *.pdb *.pch; \
	rm -f *~ $(SOLIB) $(ARLIB)

avltree.o: ./ds.h ./local.h

heap.o: ./ds.h ./local.h

parray.o: ./ds.h ./local.h

queue.o: ./ds.h ./local.h

stack.o: ./ds.h ./local.h

hashtbl.o: ./ds.h ./local.h

set.o: ./ds.h ./local.h
