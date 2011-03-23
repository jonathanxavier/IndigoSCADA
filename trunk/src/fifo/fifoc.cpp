/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "fifo.h"
#include "fifoc.h"

SHMEM_DLL_ENTRY fifo_h fifo_open(char const* name, size_t max_size)
{
    fifo_obj* fifo = new fifo_obj();

    if(fifo->open(name, max_size))
	{
        return (fifo_h)fifo;
    }

	fprintf(stderr, "Failed to create queue buffer\n");
	fflush(stderr);
    delete fifo;
    return NULL;
}
        
SHMEM_DLL_ENTRY void fifo_put(fifo_h hnd, char* message, int length)
{
   ((fifo_obj*)hnd)->put(message, length);
}

SHMEM_DLL_ENTRY int fifo_get(fifo_h hnd, char* buf, int buf_size, unsigned msec)
{
    return ((fifo_obj*)hnd)->get(buf, buf_size, msec);
}

SHMEM_DLL_ENTRY void fifo_close(fifo_h hnd)
{
    ((fifo_obj*)hnd)->close();
    delete (fifo_obj*)hnd;
}
