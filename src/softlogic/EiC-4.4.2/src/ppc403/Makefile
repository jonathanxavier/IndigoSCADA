CC = powerpc-eabi-gcc
AR = powerpc-eabi-ar
AS = powerpc-eabi-as
LD = powerpc-eabi-ld

LIB =  libppc.a

PLATFORM = POWERPC 
DEFINES = -D$(PLATFORM) -D_SEIC_ 

CFLAGS =  -mno-eabi -mcpu=403 -mbig -fsigned-char -Wall $(DEFINES) \
	  -I /ppc/include -I ..
	  
libOBJS =	$(LIB)(close.o)\
		$(LIB)(fstat.o)\
		$(LIB)(getpid.o)\
		$(LIB)(isatty.o)\
		$(LIB)(kill.o)\
		$(LIB)(lseek.o)\
		$(LIB)(open.o)\
		$(LIB)(print.o)\
		$(LIB)(putnum.o)\
		$(LIB)(read.o)\
		$(LIB)(sbrk.o)\
		$(LIB)(sleep.o)\
		$(LIB)(stat.o)\
		$(LIB)(unlink.o)\
		$(LIB)(write.o)\
		$(LIB)(gets.o)\
		$(LIB)(ppclib.o)


all:	libppc.a \
	io.o

clean:	
	rm libppc.a
	rm io.o

$(LIB):	$(libOBJS)
