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
 * Implementation of sets and bags.
 * A Set is a data structure that holds any number of unique elements --- 
 * i.e. no duplicates are allowed. A Bag can hold any number of elements,
 * including duplicates.
 * 
 * This particular implementation uses a QUEUE to represent sets.
 *
 * Created: Mon Jan  7 10:38:41 EST 2002
 * Updates: Sat Jun 22 12:29:23 EST 2002
 */

# include "local.h"

struct _set
{
    int 	ord;		/* Elements have an order flag */
    int		ed;		/* Ensure disjoint flag */
    union {
	QUEUE	un_qelts;
	AVLTREE un_telts;
    } un_data;
    SETCMPFUN	cmp;
};

# define SET_IsSET(s)		( (s)->ed != 0 )
# define SET_IsBAG(s)		( (s)->ed == 0 )

# define SET_Ord(s)		( (s)->ord != 0 )

# define qelts		un_data.un_qelts
# define telts		un_data.un_telts

SET
setMake(void)
{
    return setNew(NULL,0,0);
}

SET
setNew(SETCMPFUN f,int ensure_disjoint,int ord)
{
    Set * set;

    STDMALLOC(set,sizeof(Set),NULL);

    set->ed   = ensure_disjoint != 0;

    if (ensure_disjoint && ord)
    {
	/* Elements can be ordered, this means, use more efficient AVL tree */
	set->telts = avlNewTree(f,0,0);
	set->ord   = 1;
    }
    else
    {
	/* Elements cannot be ordered, use conventional QUEUE */
	set->qelts= qMake();
	set->ord  = 0;
    }
    set->cmp  = f;

    return set;
}
void
setClose(SET set)
{
    if (SET_Ord(set))
	avlClose(set->telts);
    else
	qClose(set->qelts);

    STDFREE(set);
}
void
setCloseWithFunction(SET set,void (*fun)(void*))
{
    if (SET_Ord(set))
	avlCloseWithFunction(set->telts,fun);
    else
	qCloseWithFunction(set->qelts,fun);
    STDFREE(set);
}
/*----------------------------------------------------------------------*
 * Add/Remove								*
 *----------------------------------------------------------------------*/
int
setAdd(SET set,void *data)
{
    /* An ordered set uses the avl tree routines. */
    if (SET_Ord(set))
	return avlInsert(set->telts,data,data);

    /* A non-ordered set uses the disjoint flag to check if it needs to 
     * exclude double elements */
    if (SET_IsSET(set) && setContains(set,data))
	return -1;

    /* Else, or if BAG, add the element to the queue */
    return - !qAdd(set->qelts,data);
}
void*
setRemove(SET set,void * key)
{
    void *e;

    if (SET_Ord(set))
	return avlRemove(set->telts,key);

    for (e = qFirst(set->qelts); e; e = qNext(set->qelts))
    {
	if (set->cmp ? set->cmp(e,key) == 0 : e == key)
	{
	    qElemRemove(set->qelts,qElemCurr(set->qelts));
	    return e;
	}
    }

    return NULL;
}
void*
setFirst(SET set)
{
    if (SET_Ord(set))
	return avlFirst(set->telts);

    return qFirst(set->qelts);
}
void*
setNext(SET set)
{
    if (SET_Ord(set))
	return avlNext(set->telts);

    return qNext(set->qelts);
}

int
setSize(SET set)
{
    if (SET_Ord(set))
	return avlSize(set->telts);

    return qSize(set->qelts);
}
int
setEmpty(SET set)
{
    return setSize(set) == 0;
}
int
setContains(SET set,void *elt)
{
    return setFind(set,elt) != NULL;
}
/*----------------------------------------------------------------------*/
/* Search for an element in a SET/BAG. If a set is passed, we simply 	*/
/* use the avltree find routines to locate it. If a BAG is passed, we need*/ 
/* to inspect all elements prior to locating the one we are looking for.*/
/* If no compare routine is given, we resort to comparing pointers. What*/
/* else?								*/
/*----------------------------------------------------------------------*/
void*
setFind(SET set,void *elt)
{
    void *e;

    /* A orderable SET: fetch element trough avlTree routines. */
    if (SET_Ord(set))
	return avlFind(set->telts,elt);

    for (e = qFirst(set->qelts); e; e = qNext(set->qelts))
    {
	if (set->cmp ? set->cmp(e,elt) == 0 : e == elt)
	    return e;
    }

    return NULL;
}

/*----------------------------------------------------------------------*/
/* Unions and Intersections of SETs and BAGs				*/
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* setUnion(set1,set2): elements of set2 are simply added to set1. Set2 */
/* remains unchanged. set1 is modified.					*/
/* Return value is the new version of set1.				*/
/*----------------------------------------------------------------------*/

SET
setUnion(SET set1,SET set2)
{
    void *e;

    /* Add all elements of set2 onto set1 */
    for (e = setFirst(set2); e; e = setNext(set2))
	setAdd(set1,e);
 
    return set1;
}

/*----------------------------------------------------------------------*/
/* setUnion1(set1,set2): here, a completely new set is created, and the */
/* union of set1 and set2 is stored in it. set1 and set2 remain 	*/
/* unmodified.								*/
/* Returns a new set that inherits all parameters of set1.		*/
/*----------------------------------------------------------------------*/

SET
setUnion1(SET s1,SET s2)
{
    SET set;
    void *e;

    set = setNew(s1->cmp,s1->ed,s1->ord);

    if (!set)
	{ XLOG(set); return NULL; }

    for (e = setFirst(s1); e; e = setNext(s1))
	setAdd(set,e);	/* Blind copy of s1 */

    for (e = setFirst(s2); e; e = setNext(s2))
	setAdd(set,e);	/* Call setAdd() to weed out duplicates */

    return set;
}
/*----------------------------------------------------------------------*/
/* setIntersect(set1,set2): a new set is created that contains the 	*/
/* intersection of the elements contained in set1 and set2.		*/
/* This measn A /\ B							*/
/*----------------------------------------------------------------------*/

SET
setIntersect(SET s1,SET s2)
{
    SET set;
    void *e;

    /* At least one of the sets must have a compare function */
    if ( ! (s1->cmp || s2->cmp) )
	return NULL;

    /* Create a disjoint set */
    set = setNew(s1->cmp ? s1->cmp : s2->cmp,1,s1->ord);

    if (!set)
	{ XLOG(set); return NULL; }

    for (e = setFirst(s1); e; e = setNext(s1))
    {
	if (setContains(s2,e))
	    setAdd(set,e);
    }

    return set;
}
/*----------------------------------------------------------------------*/
/* The difference of two sets: the result of A - B is all elements that */
/* belong to A only. This means A - B = A /\ B - B			*/
/*----------------------------------------------------------------------*/

SET
setDifference(SET A,SET B)
{
    SET set;
    void *e;

    if ( ! B->cmp)
	return NULL;		/* B needs a compare function */

    /* Create the resulting set */
    set = setNew(A->cmp,A->ed,A->ord);

    if (!set)
	{ XLOG(set); return NULL; }

    for (e = setFirst(A); e; e = setNext(A))
	if (!setContains(B,e))
	    setAdd(set,e);

    return set;
}
/*----------------------------------------------------------------------*/
/* setXIntersect(set1,set2): a new set is created, containing those	*/
/* elements of set1 and set2 that are not contained in the intersection */
/* of set1 and set2.							*/
/* Note: mathematically, this is the difference A \/ B - A /\ B		*/
/*----------------------------------------------------------------------*/

SET
setXIntersect(SET A,SET B)
{
    SET set;
    void *e;

    if (! (A->cmp && B->cmp) )
	return NULL;		/* Both need a compare function */

    /* Create new set */
    if (! (set = setNew(A->cmp,A->ed,A->ord)))
	{ XLOG(set); return NULL; }

    /* Add elements of A only */
    for (e = setFirst(A); e; e = setNext(A))
	if (!setContains(B,e))
	    setAdd(set,e);

    /* Add elements of B only */
    for (e = setFirst(B); e; e = setNext(B))
	if (!setContains(A,e))
	    setAdd(set,e);

    return set;
}
