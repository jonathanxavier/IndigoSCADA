OBJS = crypt_file.o md5.o arc4.o cryptlib.o filters.o misc.o mqueue.o queue.o
INCS = rc4/arc4.h rc4/config.h rc4/cryptlib.h rc4/misc.h rc4/mqueue.h rc4/pch.h rc4/queue.h rc4/words.h rc4/filters.h md5/md5.h

CC = g++
TFLAGS=-pthread
CFLAGS = -I. -I.. -c -Wall -O0 -g $(TFLAGS)
AR = ar
ARFLAGS = -cru
LD = $(CC)
LDFLAGS = -g $(TFLAGS)

ifneq (,$(findstring freebsd,$(OSTYPE)))
RANLIB = ranlib
else
RANLIB = true
endif

GB_LIB=../libgigabase_r.a

libcryptgb.a: $(OBJS)
	rm -f libcryptgb.a
	$(AR) $(ARFLAGS) libcryptgb.a $(OBJS)
	$(RANLIB) libcryptgb.a

crypt_file.o: crypt_file.cpp $(INCS)
	$(CC) $(CFLAGS) crypt_file.cpp   

md5.o: md5/md5.cpp $(INCS)
	$(CC) $(CFLAGS) md5/md5.cpp

arc4.o: rc4/arc4.cpp $(INCS)
	$(CC) $(CFLAGS) rc4/arc4.cpp

cryptlib.o: rc4/cryptlib.cpp $(INCS)
	$(CC) $(CFLAGS) rc4/cryptlib.cpp

filters.o: rc4/filters.cpp $(INCS)
	$(CC) $(CFLAGS) rc4/filters.cpp

misc.o: rc4/misc.cpp $(INCS)
	$(CC) $(CFLAGS) rc4/misc.cpp

mqueue.o: rc4/mqueue.cpp $(INCS)
	$(CC) $(CFLAGS) rc4/mqueue.cpp

queue.o: rc4/queue.cpp $(INCS)
	$(CC) $(CFLAGS) rc4/queue.cpp


testperf.o: ../testperf.cpp $(INCS)
	$(CC) $(CFLAGS) -DENCRYPTED ../testperf.cpp

testperf: testperf.o libcryptgb.a $(GB_LIB)
	$(LD) $(LDFLAGS) -o testperf testperf.o libcryptgb.a $(GB_LIB) $(STDLIBS)

clean:  
	rm -f *.o *.a testperf
