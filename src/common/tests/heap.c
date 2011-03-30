/*
 * Copyright (c) 2000, 2001 Peter Bozarov.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Peter Bozarov.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
# include <ds.h>

# include <stdio.h>
# include <stdlib.h>

# define PRINT_ARRAY	0
# define PRINT_TREE	1

static int printing = PRINT_ARRAY;

static int
compare(void *one,void *two)
{
    double key_one = *(double*)one;
    double key_two = *(double*)two;
    int		ret = 0;

    if (key_one < key_two)
	ret = -1;
    if (key_one > key_two)
	ret =  1;

    return ret;
}
static void
printfunc(int depth,void *pKey,void *pData)
{
    printf("%*c%.5f ",depth<<1,' ',*((double*)pKey));

    if (printing == PRINT_TREE)
	putchar('\n');
}
static void
chg_func(void* data,int idx)
{
    double key = *(double*)data;
}
double
random_num(double max)
{
    double n;

    n = rand()/((double)RAND_MAX) * max;
    return n;
}
int
random_int(int max)
{
    return (int)random_num((double)max);
}
int
main(int argc,char **argv)
{
    int		i;
    HEAP	heap;
    double *	keys;
    int		size;

    if (argc == 1)
    {
	printf("%s: usage: %s <heap_size>\n",argv[0],argv[0]);
	return 0;
    }
    size = atoi(argv[1]);
    if (size == 0)
	size = 100;

    printf("Test program for HEAP\n");
    
    printf("Making heap of size %d\n",size);
    heap = heapNew(compare,size,10,HEAP_MINIMIZE);

    if (!heap)
    {
	printf("Can't create heap\n");
	return -1;
    }
    heapSetChgFunc(heap,chg_func);
    if ( !( keys = malloc( sizeof(double)*size ) ) )
    {
	printf("Can't allocate key array\n");
	return -1;
    }

    for (i = 0; i < size; i++)
    {
	keys[i] = random_num(100);

	/* printf("adding element with key %f\n",keys[i]); */

	heapInsert(heap,&keys[i],&keys[i]);
	if (heapCheck(heap))
	{
	    printf("Heap error while inserting %d = %f\n",i,keys[i]);
	    printing = PRINT_TREE;
	    heapPrintTree(heap,printfunc);
	    exit(1);
	}
    }

    printf("Starting random deletion of elements...");
    fflush(stdout);

    for (i = 0; i < size; i++)
    {
	int idx = random_int(size - i);

	/* printf("deleting element at %d, elements left %d\n",idx,size - i); */

	heapDelete(heap,idx);
	if (heapCheck(heap))
	{
	    printf("Heap error while deleting\n");
	    printing = PRINT_TREE;
	    heapPrintTree(heap,printfunc);
	    exit(1);
	}
    }
    printf("done.\nIf you see this, then the heap implementation is OK.\n");

    return 0;
}
