/*
 * Copyright (c) 2002, 2003 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#ifdef HAVE_VASPRINTF
/* If we have vasprintf, we need to define this before we include stdio.h. */
#define _GNU_SOURCE
#endif

#include <sys/types.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "event.h"
#include "config.h"
#include "evutil.h"
#include "itrace.h"

struct evbuffer *
evbuffer_new(void)
{
	struct evbuffer *buffer;
	IT_IT("evbuffer_new");
	
	buffer = calloc(1, sizeof(struct evbuffer));

	IT_EXIT;
	return (buffer);
}

void
evbuffer_free(struct evbuffer *buffer)
{
	IT_IT("evbuffer_free");
	if (buffer->orig_buffer != NULL)
		free(buffer->orig_buffer);
	free(buffer);

	IT_EXIT;
}

/* 
 * This is a destructive add.  The data from one buffer moves into
 * the other buffer.
 */

#define SWAP(x,y) do { \
	(x)->buffer = (y)->buffer; \
	(x)->orig_buffer = (y)->orig_buffer; \
	(x)->misalign = (y)->misalign; \
	(x)->totallen = (y)->totallen; \
	(x)->off = (y)->off; \
} while (0)

int
evbuffer_add_buffer(struct evbuffer *outbuf, struct evbuffer *inbuf)
{
	int res;
	IT_IT("evbuffer_add_buffer");

	/* Short cut for better performance */
	if (outbuf->off == 0) {
		struct evbuffer tmp;
		size_t oldoff = inbuf->off;

		/* Swap them directly */
		SWAP(&tmp, outbuf);
		SWAP(outbuf, inbuf);
		SWAP(inbuf, &tmp);

		/* 
		 * Optimization comes with a price; we need to notify the
		 * buffer if necessary of the changes. oldoff is the amount
		 * of data that we transfered from inbuf to outbuf
		 */
		if (inbuf->off != oldoff && inbuf->cb != NULL)
			(*inbuf->cb)(inbuf, oldoff, inbuf->off, inbuf->cbarg);
		if (oldoff && outbuf->cb != NULL)
			(*outbuf->cb)(outbuf, 0, oldoff, outbuf->cbarg);
		
		IT_EXIT;
		return (0);
	}

	res = evbuffer_add(outbuf, inbuf->buffer, inbuf->off);
	if (res == 0) {
		/* We drain the input buffer on success */
		evbuffer_drain(inbuf, inbuf->off);
	}

	IT_EXIT;
	return (res);
}

int
evbuffer_add_vprintf(struct evbuffer *buf, const char *fmt, va_list ap)
{
	char *buffer;
	size_t space;
	size_t oldoff = buf->off;
	int sz;
	va_list aq;
	IT_IT("evbuffer_add_vprintf");

	/* make sure that at least some space is available */
	evbuffer_expand(buf, 64);
	for (;;) {
		size_t used = buf->misalign + buf->off;
		buffer = (char *)buf->buffer + buf->off;
		assert(buf->totallen >= used);
		space = buf->totallen - used;

#ifndef va_copy
#define	va_copy(dst, src)	memcpy(&(dst), &(src), sizeof(va_list))
#endif
		va_copy(aq, ap);

		sz = evutil_vsnprintf(buffer, space, fmt, aq);

		va_end(aq);

		if (sz < 0)
		{
			IT_EXIT;
			return (-1);
		}
		if ((size_t)sz < space) {
			buf->off += sz;
			if (buf->cb != NULL)
				(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);
			IT_EXIT;
			return (sz);
		}
		if (evbuffer_expand(buf, sz + 1) == -1)
		{
			IT_EXIT;
			return (-1);
		}

	}
	/* NOTREACHED */

	IT_EXIT;
}

int
evbuffer_add_printf(struct evbuffer *buf, const char *fmt, ...)
{
	int res = -1;
	va_list ap;

	va_start(ap, fmt);
	res = evbuffer_add_vprintf(buf, fmt, ap);
	va_end(ap);
	return (res);
}

/* Reads data from an event buffer and drains the bytes read */

int
evbuffer_remove(struct evbuffer *buf, void *data, size_t datlen)
{
	size_t nread = datlen;
	IT_IT("evbuffer_remove");

	if (nread >= buf->off)
		nread = buf->off;

	memcpy(data, buf->buffer, nread);
	evbuffer_drain(buf, nread);
	
	IT_EXIT;
	return (nread);
}

/*
 * Reads a line terminated by either '\r\n', '\n\r' or '\r' or '\n'.
 * The returned buffer needs to be freed by the called.
 */

char *
evbuffer_readline(struct evbuffer *buffer)
{
	u_char *data = EVBUFFER_DATA(buffer);
	size_t len = EVBUFFER_LENGTH(buffer);
	char *line;
	unsigned int i;
	IT_IT("evbuffer_readline");

	for (i = 0; i < len; i++) {
		if (data[i] == '\r' || data[i] == '\n')
			break;
	}

	if (i == len)
	{	
		IT_EXIT;
		return (NULL);
	}

	if ((line = malloc(i + 1)) == NULL) {
		fprintf(stderr, "%s: out of memory\n", __func__);
		IT_EXIT;
		return (NULL);
	}

	memcpy(line, data, i);
	line[i] = '\0';

	/*
	 * Some protocols terminate a line with '\r\n', so check for
	 * that, too.
	 */
	if ( i < len - 1 ) {
		char fch = data[i], sch = data[i+1];

		/* Drain one more character if needed */
		if ( (sch == '\r' || sch == '\n') && sch != fch )
			i += 1;
	}

	evbuffer_drain(buffer, i + 1);

	IT_EXIT;
	return (line);
}

/* Adds data to an event buffer */

static void
evbuffer_align(struct evbuffer *buf)
{
	IT_IT("evbuffer_align");

	memmove(buf->orig_buffer, buf->buffer, buf->off);
	buf->buffer = buf->orig_buffer;
	buf->misalign = 0;

	IT_EXIT;
}

/* Expands the available space in the event buffer to at least datlen */

int
evbuffer_expand(struct evbuffer *buf, size_t datlen)
{
	size_t need = buf->misalign + buf->off + datlen;
	IT_IT("evbuffer_expand");

	/* If we can fit all the data, then we don't have to do anything */
	if (buf->totallen >= need)
	{
		IT_EXIT;
		return (0);
	}

	/*
	 * If the misalignment fulfills our data needs, we just force an
	 * alignment to happen.  Afterwards, we have enough space.
	 */
	if (buf->misalign >= datlen) {
		evbuffer_align(buf);
	} else {
		void *newbuf;
		size_t length = buf->totallen;

		if (length < 256)
			length = 256;
		while (length < need)
			length <<= 1;

		if (buf->orig_buffer != buf->buffer)
			evbuffer_align(buf);
		if ((newbuf = realloc(buf->buffer, length)) == NULL)
		{
			IT_EXIT;
			return (-1);
		}

		buf->orig_buffer = buf->buffer = newbuf;
		buf->totallen = length;
	}

	IT_EXIT;
	return (0);
}

int
evbuffer_add(struct evbuffer *buf, const void *data, size_t datlen)
{
	size_t need = buf->misalign + buf->off + datlen;
	size_t oldoff = buf->off;
	IT_IT("evbuffer_add");

	if (buf->totallen < need) {
		if (evbuffer_expand(buf, datlen) == -1)
		{
			IT_EXIT;
			return (-1);
		}
	}

	memcpy(buf->buffer + buf->off, data, datlen);
	buf->off += datlen;

	if (datlen && buf->cb != NULL)
		(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);

	IT_EXIT;
	return (0);
}

void
evbuffer_drain(struct evbuffer *buf, size_t len)
{
	size_t oldoff = buf->off;
	IT_IT("evbuffer_drain");

	if (len >= buf->off) {
		buf->off = 0;
		buf->buffer = buf->orig_buffer;
		buf->misalign = 0;
		goto done;
	}

	buf->buffer += len;
	buf->misalign += len;

	buf->off -= len;

 done:
	/* Tell someone about changes in this buffer */
	if (buf->off != oldoff && buf->cb != NULL)
		(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);

	IT_EXIT;
}

/*
 * Reads data from a file descriptor into a buffer.
 */

#define EVBUFFER_MAX_READ	4096

//#define DEBUG_TCP_PROTOCOL //apa added 18-01-10

int
evbuffer_read(struct evbuffer *buf, int fd, int howmuch)
{
	u_char *p;
	size_t oldoff = buf->off;
	int n = EVBUFFER_MAX_READ;
	int i;

#if defined(FIONREAD)
#ifdef WIN32
	long lng = n;
	
	IT_IT("evbuffer_read");

	if (ioctlsocket(fd, FIONREAD, &lng) == -1 || (n=lng) <= 0) 
	{
#else
	IT_IT("evbuffer_read");
	if (ioctl(fd, FIONREAD, &n) == -1 || n <= 0) 
	{
#endif
		n = EVBUFFER_MAX_READ;
	} 
	else if (n > EVBUFFER_MAX_READ && n > howmuch) 
	{
		/*
		 * It's possible that a lot of data is available for
		 * reading.  We do not want to exhaust resources
		 * before the reader has a chance to do something
		 * about it.  If the reader does not tell us how much
		 * data we should read, we artifically limit it.
		 */
		if ((size_t)n > buf->totallen << 2)
			n = buf->totallen << 2;
		if (n < EVBUFFER_MAX_READ)
			n = EVBUFFER_MAX_READ;
	}
#endif

	//printf("lng %d\n", lng);

	if (howmuch < 0 || howmuch > n)
		howmuch = n;

	/* If we don't have FIONREAD, we might waste some space here */
	if (evbuffer_expand(buf, howmuch) == -1)
	{
		IT_EXIT;
		return (-1);
	}

	/* We can append new data at this point */
	p = buf->buffer + buf->off;

#ifndef WIN32
	n = read(fd, p, howmuch);
#else
	n = recv(fd, p, howmuch, 0);

	#ifdef DEBUG_TCP_PROTOCOL
	printf("Rec %d\n", n);
	#endif

	for(i = 0;i < n; i++)
	{
		unsigned char c = *((unsigned char*)p + i);
		//printf("rx <--- 0x%02x-\n", c);
		IT_COMMENT1("rx <--- 0x%02x", c);
	}

#endif
	if (n == -1)
	{
		IT_EXIT;
		return (-1);
	}

	if (n == 0)
	{
		IT_EXIT;
		return (0);
	}

	buf->off += n;

	/* Tell someone about changes in this buffer */
	if (buf->off != oldoff && buf->cb != NULL)
		(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);

	IT_EXIT;
	return (n);
}

int
evbuffer_write(struct evbuffer *buffer, int fd)
{
	int n;
//	int i;
	IT_IT("evbuffer_write");

#ifndef WIN32
	n = write(fd, buffer->buffer, buffer->off);
#else

	/*
	TODO: 09-01-2010 
	Discover when the driver start to create fragments
	in the output, I think that may be when the write buffer overtake a certain 
	amount, may be that select can say us when fragments begins to form
	{
		int rc;
		fd_set wri_err;
		struct timeval tm;
		FD_ZERO(&wri_err);
		FD_SET(fd, &wri_err);

		tm.tv_sec = 0;
        tm.tv_usec = 1000;

		rc = select((int)fd+1, NULL, NULL, &wri_err, &tm);

		//if (rc <= 0) 
		//{ 
		//	assert(0);
		//}

		if(FD_ISSET(fd,&wri_err))
		{
			IT_EXIT;
			printf("Error in write!\n");
			return (0);
		}

		//FD_CLR(fd,&wri_err);
	}
	*/
	
	n = send(fd, buffer->buffer, buffer->off, 0);


	#ifdef DEBUG_TCP_PROTOCOL
	printf("Send %d\n", n);
	#endif

//	for(i = 0;i < n; i++)
//	{
//		unsigned char c = *((unsigned char*)buffer->buffer + i);
		//printf("tx ---> 0x%02x\n", c);
//		IT_COMMENT1("tx ---> 0x%02x", c);
//	}
#endif
	if (n == -1)
	{
		IT_EXIT;
		return (-1);
	}
	if (n == 0)
	{
		IT_EXIT;
		return (0);
	}
	evbuffer_drain(buffer, n);

	IT_EXIT;
	return (n);
}

u_char *
evbuffer_find(struct evbuffer *buffer, const u_char *what, size_t len)
{
	u_char *search = buffer->buffer, *end = search + buffer->off;
	u_char *p;
	IT_IT("evbuffer_find");

	while (search < end &&
	    (p = memchr(search, *what, end - search)) != NULL) {
		if (p + len > end)
			break;
		if (memcmp(p, what, len) == 0)
		{
			IT_EXIT;
			return (p);
		}
		search = p + 1;
	}

	IT_EXIT;
	return (NULL);
}

void evbuffer_setcb(struct evbuffer *buffer,
    void (*cb)(struct evbuffer *, size_t, size_t, void *),
    void *cbarg)
{
	IT_IT("evbuffer_setcb");

	buffer->cb = cb;
	buffer->cbarg = cbarg;

	IT_EXIT;
}
