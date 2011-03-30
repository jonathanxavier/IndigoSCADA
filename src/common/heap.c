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

/*
 * A binary heap implementation. 
 * 
 * See the documentation that comes with this library for usage.
 *
 * Copyright (C) 2001, Peter Bozarov
 * 
 * This file is version 1.0.
 * Last modified: Thu Nov  8 20:54:23 EST 2001
 *		  Fri Nov 30 00:01:12 CET 2001
 *		  Fri Dec  7 21:09:15 EST 2001
 *		  Mon Dec 24 10:36:10 EST 2001
 *		  Mon Dec 31 09:41:31 CET 2001
 *
 */

# include "local.h" 

# define HLEFT(i)	(((i)<<1) + 1)
# define HRIGHT(i)	(HLEFT(i)+1)
# define HPARENT(i)	((i-1)>>1)


typedef struct HeapElement
{
    DSKEY 	heKey;
    void *	heData;
} HeapElement;

struct Heap
{
    int		   hpMode;	/* Minimize or Maximize */
    int		   hpSize;
    int 	   hpFilled;
    int		   hpGrowBy;
    HeapElement**  hpArray;
    HEAPCMPFUNC    hpCmpFunc;
    HEAPCHGFUNC	   hpChgFunc;
};

typedef int (*HeapInternCmp)(Heap*,HeapElement*,HeapElement*);

# define HSIZE(h)	 (h)->hpFilled
# define HARRAY(h,i)	 (h)->hpArray[i]
# define NEEDS2GROW(h)	((h)->hpFilled == (h)->hpSize)
# define NEEDS2SHRINK(h) ((h)->hpFilled < (h)->hpSize - (h)->hpGrowBy)
# define LAST_INDEX(h)	((h)->hpFilled - 1)
# define HLAST(h)	((h)->hpArray[LAST_INDEX(h)])

# define HSWAP(h,i,j) \
    { \
	HeapElement * tmp = HARRAY(h,i); \
	HARRAY(h,i) = HARRAY(h,j); \
	HARRAY(h,j) = tmp; \
    }

/*----------------------------- (  15 lines) --------------------------*/
static int heap_grow(Heap *);
static int heap_shrink(Heap *);
static int heap_int_compare(void *,void *);
static int heap_double_compare(void *,void *);
static int heap_float_compare(void *,void *);
static int heap_larger(Heap *,HeapElement *,HeapElement *);
static int heap_smaller(Heap *,HeapElement *,HeapElement *);
static void heap_swap(Heap *,int,int);
static int heap_heapify(Heap *,int);
static HeapElement * heap_delete(Heap *,int);
static HeapElement * heap_new_element(DSKEY,void *);
static void print_tree(Heap *,int,int,HEAPPRINTFUNC);
static void heap_walk(Heap *,int,int,HEAPWALKFUNC);
static int check_values(Heap *,int,int);

# ifndef StandAlone
    static int check_values( Heap *, int, int );
# endif

static int
heap_grow(Heap *heap)
{
    unsigned int	new_size;

    new_size = sizeof(HeapElement*) * (heap->hpSize + heap->hpGrowBy);

    heap->hpSize += heap->hpGrowBy;

    STDREALLOC(heap->hpArray,new_size,-1);

    return 0;
}
static int
heap_shrink(Heap *heap)
{
    unsigned int new_size;

    new_size = sizeof(HeapElement*)*(heap->hpSize - heap->hpGrowBy);

    heap->hpSize -= heap->hpGrowBy;

    heap->hpArray = (void*) realloc(heap->hpArray,new_size);

    if (!heap->hpArray)
	{ XLOG(heap->hpArray); return -1; }

    return 0;
}
static int
heap_int_compare(void *pOne,void *pTwo)
{
    int one = *(int*)pOne;
    int two = *(int*)pTwo;

    return one - two;
}
static int
heap_double_compare(void *pOne,void *pTwo)
{
    double one = *(double*)pOne;
    double two = *(double*)pTwo;

    if (one < two)
    	return -1;
    if (one > two)
    	return  1;
    return 0;
}
static int
heap_float_compare(void *pOne,void *pTwo)
{
    float one = *(float*)pOne;
    float two = *(float*)pTwo;

    if (one < two)
    	return -1;
    if (one > two)
    	return  1;
    return 0;
}
static int
heap_larger(Heap *h,HeapElement *one,HeapElement *two)
{
    if (!one || !two || !h->hpCmpFunc)
	return -1;
    return h->hpCmpFunc(one->heKey,two->heKey) > 0;
}
static int
heap_smaller(Heap *h,HeapElement *one,HeapElement *two)
{
    DBG(debug("heap_smaller(h=%p,one=%p,two=%p)\n",
    	h,one,two));

    if (!one || !two || !h->hpCmpFunc)
	{ XLOG(h->hpCmpFunc); return -1; }

    return h->hpCmpFunc(one->heKey,two->heKey) < 0;
}
static void
heap_swap(Heap *h,int idx1,int idx2)
{
    DBG(debug("heap_swap(h=%p,idx1=%d,idx2=%d)\n",
    	h,idx1,idx2));

    if (h->hpChgFunc)
    {
	h->hpChgFunc(HARRAY(h,idx1)->heData,idx2);
	h->hpChgFunc(HARRAY(h,idx2)->heData,idx1);
    }
    HSWAP(h,idx1,idx2);
}
static int
heap_heapify(Heap *h,int idx)
{
    int 		l,r,largest;
    HeapInternCmp 	cmp_func;
    DBG(debug("heap_heapify(h=%p,idx=%d)\n",h,idx));

    l = HLEFT(idx);
    r = HRIGHT(idx);

    LLOG(l); LLOG(r);

    if (h->hpMode == HEAP_MAXIMIZE)
	cmp_func = heap_larger;
    else 
    	cmp_func = heap_smaller;

    if (l <= HSIZE(h) && cmp_func(h,HARRAY(h,l),HARRAY(h,idx)))
	largest = l; 
    else 
	largest = idx;

    if (r <= HSIZE(h) && cmp_func(h,HARRAY(h,r),HARRAY(h,largest)))
	largest = r;

    if (largest != idx)
    {
	heap_swap(h,idx,largest);
	return heap_heapify(h,largest);
    }

    return 0;
}
/*
 * Return an element from the heap, located at the given index
 */
static HeapElement *
heap_delete(Heap * h,int idx)
{
    HeapElement *he,*helast;
    int		 pidx;
    HeapInternCmp cmp_func;

    DBG(debug("heap_delete(h=%p,idx=%d)\n",h,idx));

    LLOG(HSIZE(h));

    if (idx < 0 || idx >= HSIZE(h))
	{ LLOG(idx); return NULL; }

    if (h->hpMode == HEAP_MAXIMIZE)
	cmp_func = heap_larger;
    else 
    	cmp_func = heap_smaller;

    /* Remember the current element */
    he = HARRAY(h,idx);
    /* Remember the last element */
    helast = HLAST(h);

    h->hpFilled--;

    if (idx == HSIZE(h))
    	/* This is the last element */
	goto end_label;

    /* Put the last element in the position of the current */

    HARRAY(h,idx) = helast;

    if (h->hpChgFunc)
	h->hpChgFunc(HARRAY(h,idx)->heData,idx);

    /* Heapify the new subtree. Then if the root of the subtree 
     * is larger than its parent, propagate it up the tree until
     * it finds is proper location */
    heap_heapify(h,idx);

    pidx = HPARENT(idx);
    LLOG(pidx);

    if (idx > 0)
    {
	while (cmp_func(h,HARRAY(h,idx),HARRAY(h,pidx)))
	{
	    heap_swap(h,idx,pidx);
	    idx = pidx;
	    if (idx == 0)
		break;
	    pidx = HPARENT(idx);
	}
    }

end_label:


    if (NEEDS2SHRINK(h))
    	heap_shrink(h);

    return he;
}
static HeapElement*
heap_new_element(DSKEY key,void *data)
{
    HeapElement * he;

    STDMALLOC(he,sizeof(HeapElement),NULL);
    he->heKey = key;
    he->heData = data;
    return he;
}
static void
print_tree(Heap * h,int idx,int depth,
	   HEAPPRINTFUNC func)
{
    HeapElement *he;

    DBG(debug("print_tree(h=%p,idx=%d,depth=%d,func=%p)\n",
    	h,idx,depth,func));

    if (idx < 0 || idx >= HSIZE(h))
	return;
    he = HARRAY(h,idx);

    func(depth,he->heKey,he->heData);

    print_tree(h,HLEFT(idx),depth+1,func);
    print_tree(h,HRIGHT(idx),depth+1,func);
}

static void
heap_walk(Heap *h,int idx,int depth,HEAPWALKFUNC func)
{
    HeapElement *he;

    if (idx < 0 || idx >= HSIZE(h))
	return;

    he = HARRAY(h,idx);

    func(depth,he->heKey,he->heData);

    heap_walk(h,HLEFT(idx),depth + 1,func);
    heap_walk(h,HRIGHT(idx),depth + 1,func);
}
    
/*----------------------------------------------------------------------*
 * Exported functions.							*
 *----------------------------------------------------------------------*/
HEAP
heapMake(void)
{
    return heapNew((HEAPCMPFUNC)strcmp,10,10,HEAP_MINIMIZE);
}
HEAP
heapNew(int (*cmp)(DSKEY,DSKEY),int size,int grow_by,int mode)
{
    Heap *	heap;
    DBG(debug("heapNew(cmp=%p,size=%d,grow_by=%d)\n",(void*)cmp,size,grow_by));
    DBG(debug("heap mode = %s\n",mode == HEAP_MINIMIZE ? "minimize" :
    				 mode == HEAP_MAXIMIZE ? "maximize" : 
				 "unknown?"));
    STDMALLOC(heap,sizeof(Heap),NULL);
    STDMALLOC(heap->hpArray,sizeof(HeapElement*) * size,NULL);

    heap->hpMode   = mode;
    heap->hpSize   = size;
    heap->hpFilled = 0;
    heap->hpGrowBy = grow_by;
    heap->hpCmpFunc= cmp;
    heap->hpChgFunc= NULL;

    return (HEAP)heap;
}
void
heapCloseWithFunction(HEAP pHeap,void (*fun)(void*))
{
    Heap * h = (Heap*)pHeap;
    int i = h->hpFilled; 

    while (i--)
    {
	if (h->hpArray[i] && fun)
	    fun(h->hpArray[i]->heData);
	STDFREE(h->hpArray[i]);
    }
    STDFREE(h->hpArray);
    STDFREE(h);
}
void
heapClose(HEAP pHeap)
{
    heapCloseWithFunction(pHeap,NULL);
}
HEAP
heapMakeIntKeys(int size,int grow_by,int mode)
{
    return heapNew(heap_int_compare,size,grow_by,mode);
}
HEAP
heapMakeDoubleKeys(int size,int grow_by,int mode)
{
    return heapNew(heap_double_compare,size,grow_by,mode);
}
HEAP
heapMakeFloatKeys(int size,int grow_by,int mode)
{
    return heapNew(heap_float_compare,size,grow_by,mode);
}
HEAP
heapMakeStringKeys(int size,int grow_by,int mode)
{
    return heapNew((HEAPCMPFUNC)strcmp,size,grow_by,mode);
}
int
heapSetChgFunc(HEAP heap,HEAPCHGFUNC func)
{
    DBG(debug("heapSetChgFunc(heap=%p,func=%p)\n",heap,func));

    if (!heap)
	{ XLOG(heap); return -1; }

    ((Heap*)heap)->hpChgFunc = func;

    return 0;
}
/*
 * Returns the index into the heap array 
 */
int
heapInsert(HEAP heap,const DSKEY key,void *data)
{
    Heap	* 	h = (Heap*)heap;
    HeapElement	* 	he;
    int			i,pidx;
    HeapInternCmp 	cmp_func;

    DBG(debug("heapInsert(heap=%p,key=%p,data=%p)\n",
    	heap,key,data));

    if (!h)
	{ XLOG(h); return -1; }

    if (h->hpMode == HEAP_MINIMIZE)
	cmp_func = heap_larger;
    else 
    	cmp_func = heap_smaller;

    /* 1. Make a new element */
    he = heap_new_element(key,data);

    /* 2. Grow the heap if needed */

    if (NEEDS2GROW(h) && heap_grow(h))
	{ LLOG(-1); return -1; }

    /* 3. Insert the new element into the heap */

    i = HSIZE(h);
    pidx = HPARENT(i);

    while (i > 0 && cmp_func(h,HARRAY(h,pidx),he))
    {
	if (h->hpChgFunc)
	    h->hpChgFunc(HARRAY(h,pidx)->heData,i);

	HARRAY(h,i) = HARRAY(h,pidx);

	i = pidx;
	pidx = HPARENT(i);
    }
    HARRAY(h,i) = he;

    HSIZE(h)++;
    return i;
}
/*
 * Delete an element from the heap, at the given index
 */
void*
heapDelete(HEAP heap,int idx)
{
    HeapElement * he;
    void*	  data;

    DBG(debug("heapDelete(heap=%p,idx=%d)\n",heap,idx));

    /* Remove the element from the heap */
    he = heap_delete((Heap*)heap,idx);

    XLOG(he);
    data = he ? he->heData : NULL;

    if (he) { STDFREE(he); }

    LLOG(((Heap*)heap)->hpFilled);

    return data;
}
void*
heapFirst(HEAP heap)
{
    DBG(debug("heapFirst(heap=%p)\n",heap));

    return heapDelete(heap,0);
}
void*
heapPeekFirst(HEAP heap)
{
    Heap * h = (Heap*)heap;

    DBG(debug("heapPeekFirst(heap=%p)\n",heap));

    if (!h)
	{ XLOG(h); return NULL; }

    return h->hpFilled ? HARRAY(h,0)->heData : NULL;
}
void*
heapElementAt(HEAP heap,int idx)
{
    Heap * h = (Heap*)heap;

    DBG(debug("heapElementAt(heap=%p,idx=%d)\n",heap,idx));

    if (!h)
    	{ XLOG(h); return NULL; }

    if (idx < 0 || idx >= HSIZE(h))
	{ LLOG(idx); LLOG(HSIZE(h)); return NULL; }

    return HARRAY(h,idx)->heData;
}
int
heapSize(HEAP heap)
{
    return ((Heap*)heap)->hpFilled;
}
int
heapEmpty(HEAP heap)
{
    return heapSize(heap) == 0;
}

void
heapWalk(HEAP heap,HEAPWALKFUNC func)
{
    heap_walk((Heap*)heap,0,0,func);
}

/*
 * Print routines, can be useful....
 */
void
heapPrintArray(HEAP heap,HEAPPRINTFUNC printfunc)
{
    Heap *h = (Heap*)heap;
    int i; 

    for (i = 0; i < HSIZE(h); i++)
    {
	HeapElement * he = HARRAY(h,i);
	printfunc(-1,he->heKey,he->heData);
    }
    printf("\n");
}
void
heapPrintTree(HEAP heap,HEAPPRINTFUNC printfunc)
{
    Heap *h = (Heap*)heap;

    if (HSIZE(h) == 0)
	{ XLOG(0); return; }

    print_tree(h,0,0,printfunc);
}
/*
 * Check the heap. All children must be smaller/larger, or equal to their
 * parents; also, all the leaves should be spread over two levels,
 * with the lower level exclusively to the left of the higher level.
 * (in fact, now that I look at the code, I can't help notice that this
 * particular kind of check is not implemented. You wanna do it?...
 */

# define Key2Int(he)	(*(int*)((he)->heKey))
# define Key2Double(he)	(*(double*)((he)->heKey))
static int
check_values(Heap *h,int idx,int depth)
{
    HeapElement *he_l = NULL;
    HeapElement *he_r = NULL;
    HeapElement *he_p = NULL;
    HeapInternCmp cmp_func;

    if (idx < 0 || idx >= HSIZE(h))
	return 0;

    if (h->hpMode == HEAP_MINIMIZE)
	cmp_func = heap_larger;
    else 
	cmp_func = heap_smaller;

    he_p = HARRAY(h,idx);

    if (HLEFT(idx) >= HSIZE(h))	/* No left child */
    	return 0;

    if (HRIGHT(idx) < HSIZE(h))	/* Has right child */
	he_r = HARRAY(h,HRIGHT(idx));
	
    he_l = HARRAY(h,HLEFT(idx));

    if ( cmp_func(h,he_p,he_l))
    {
	printf("*** Heap violates parent-lchild property.\n");
	printf("*** Left child (%d) is %s than parent (%d)\n",
		HLEFT(idx),
		h->hpMode == HEAP_MINIMIZE ? "smaller" : "larger",
		idx);
	printf("*** Depth %d\n",depth);
	printf("%.8f - %.8f = %.8f\n",Key2Double(he_l),Key2Double(he_p),
		Key2Double(he_l) - Key2Double(he_p));

	return -1;
    }

    if (he_r &&  cmp_func(h,he_p,he_r))
    {
	printf("*** Heap violates parent-rchild property.\n");
	printf("*** Right child (%d) is %s than parent (%d)\n",
		HRIGHT(idx),h->hpMode == HEAP_MINIMIZE ? "smaller" : "larger",
		idx);
	printf("*** Depth %d\n",depth);
	printf("%.8f - %.8f = %.8f\n",Key2Double(he_r),Key2Double(he_p),
		Key2Double(he_r) - Key2Double(he_p));

	return -1;
    }

    if (check_values(h,HLEFT(idx),depth+1))
	return -1;

    if (he_r)
	return check_values(h,HRIGHT(idx),depth+1);
    return 0;
}

int
heapCheck(HEAP h)
{
    return check_values((Heap*)h,0,0);
}
