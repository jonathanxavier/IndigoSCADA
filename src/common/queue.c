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
 * Note: this "queue" implementation is actually also a linked list. Why
 * bother with making two things that are essentially the same as far as
 * implementation is concerned?
 *
 * Last modified: Fri Dec  7 10:19:00 EST 2001
 *		  Mon Dec 24 10:36:38 EST 2001
 */
# include "local.h"

# define QNULL		((Queue*) 0)
# define QELEMNULL	((QElem*) 0)

struct QElem
{
    void *	qeData;
    QElem *	qePrev;
    QElem *	qeNext;
};

struct Queue
{
    int		qTotal;
    QElem *	qHead;
    QElem *	qTail;
    QElem * 	qCurr;		/* For currencies */
};

# define TOTAL(q)	((Queue*)q)->qTotal
# define TAIL(q)	((Queue*)q)->qTail
# define HEAD(q)	((Queue*)q)->qHead
# define CURR(q)	((Queue*)q)->qCurr

static QElem*
qelem_new(void *data)
{
    QElem *qelem;

    STDMALLOC(qelem,sizeof(QElem),NULL);

    qelem->qeData = data;
    qelem->qePrev = QELEMNULL;
    qelem->qeNext = QELEMNULL;

    return qelem;
}
static int
qelem_enque(QUEUE q,QELEM elem)
{
    QElem *	qelem = (QElem*) elem;

    if (!HEAD(q))
    {
    	CURR(q) = TAIL(q) = HEAD(q) = qelem;
    }
    else
    {
	qelem->qePrev   = TAIL(q);
	CURR(q) = TAIL(q)->qeNext = qelem;
	TAIL(q)		= TAIL(q)->qeNext;
    }

    ++TOTAL(q);

    return 0;
}
/*----------------------------------------------------------------------*
 * Exported 								*
 *----------------------------------------------------------------------*/
/*
 * Standard Queue functions
 */
QUEUE
qMake(void)
{
    Queue *	q;

    STDMALLOC(q,sizeof(Queue),QNULL);

    q->qHead = QELEMNULL;
    q->qTail = QELEMNULL;
    q->qCurr = QELEMNULL;
    q->qTotal= 0;

    return q;
}
void
qClose(QUEUE q)
{
    qCloseWithFunction(q,NULL);
}
void
qCloseWithFunction(QUEUE queue,void (*f)(void*))
{
    if (queue)
    {
	/* Loop through all elements and free them. */
	QElem * tmp = HEAD(queue);

	while (tmp)
	{
	    QElem * t = tmp;
	    if (f)
	    	f(tmp->qeData);
	    tmp = tmp->qeNext;
	    STDFREE(t);
	}
    }

    STDFREE(queue);
}
QELEM
qEnque(QUEUE q,void *data)
{
    QElem *	qelem;

    if (!q)
	return NULL;

    /* Allocate a new QElement to put in the queue */

    if (!(qelem = qelem_new(data)))
	return NULL;

    qelem_enque(q,(QELEM)qelem);

    return (QELEM) qelem;
}
void*
qDeque(QUEUE q)
{
    QElem *	first;
    void *	data = NULL;

    if (!q)
    	return NULL;

    if (HEAD(q))
    {
    	first = HEAD(q);

	/* Last element found, set TAIL(q) to NIL */

	if (TAIL(q) == HEAD(q))
	    TAIL(q) = QELEMNULL;

	HEAD(q) = HEAD(q)->qeNext;

	if (HEAD(q))
	    HEAD(q)->qePrev = QELEMNULL;

	/* Remember the data  ... */

	data = first->qeData;

	/* ... and free the element */

	first->qeNext = QELEMNULL;
	STDFREE(first);

	--TOTAL(q);
    }
    return data;
}

int
qWalk(QUEUE q,void (*fun)(void*))
{
    QElem *	tmp;

    if (!q)
	return -1;

    tmp = HEAD(q);

    while (tmp)
    {
	(void)fun(tmp->qeData);
	tmp = tmp->qeNext;
    }

    return 0;
}
int
qWalkAscending(QUEUE q,void (*fun)(void*))
{
    return qWalk(q,fun);
}
int
qWalkDescending(QUEUE q,void (*fun)(void*))
{
    QElem *	tmp;

    if (!q)
    	return -1;

    tmp = TAIL(q);

    while (tmp)
    {
	fun(tmp->qeData);
	tmp = tmp->qePrev;
    }
    return 0;
}
int qLength(QUEUE q) { return ((Queue*)q)->qTotal; }
int qEmpty( QUEUE q) { return ! HEAD(q); }

void*
qCurrent(QUEUE q)
{
    if (!q)
	return NULL;

    return CURR(q) ? CURR(q)->qeData : NULL;
}
void
qClearCurrent(QUEUE q)
{
    qSetCurrent(q,NULL);
}
void
qSetCurrent(QUEUE q,QELEM el)
{
    if (!q)
	return;
    CURR(q) = el;
}
void*
qFirst(QUEUE q)
{
    QElem * first;

    first = qElemFirst(q);
    CURR(q) = first;
    return first ? first->qeData : NULL;
}
void*
qLast(QUEUE q)
{
    QElem * last;

    last = qElemLast(q);
    CURR(q) = last;
    return last ? last->qeData : NULL;
}
void*
qNext(QUEUE q)
{
    QElem *next;

    if (!CURR(q))
	return qFirst(q);
    next = qElemNext(CURR(q));

    CURR(q) = next;

    return next ? next->qeData : NULL;
}
void*
qPrev(QUEUE q)
{
    QElem *prev;

    if (!CURR(q))
	return qLast(q);
    prev = qElemPrev(CURR(q));
    CURR(q) = prev;

    return prev ? prev->qeData : NULL;
}
/*----------------------------------------------------------------------*
 * More sophisticated QELEM functionality 				*
 *----------------------------------------------------------------------*/
void*
qElemData(QELEM qelem)
{
    if (!qelem)
	return NULL;
    return ((QElem*)qelem)->qeData;
}

/*
 * Insert a QELEM in the queue. The new qelem is placed before the one
 * given. If the given element is NULL, the qelem put at the end of the
 * queue.
 */
int
qElemInsert(QUEUE q,QELEM pBefore,QELEM pElem)
{
    QElem *	elem = (QElem*)pElem;
    QElem *	before= (QElem*)pBefore;

    if (!before)
	return qelem_enque(q,elem);

    elem->qeNext = before;
    elem->qePrev = before->qePrev;
    if (before->qePrev)
	before->qePrev->qeNext = elem;
    before->qePrev = elem;
    return 0;
}
/*
 * Attach an existing QELEM to the given QUEUE. This is the same
 * as actually enqueing the element, but does not create a new
 * QELEM.
 */
int
qElemAttach(QUEUE q,QELEM elem)
{
    return qelem_enque(q,elem);
}
/*
 * Detach the given queue element from the queue.
 * It can now be added to another queue, or freed using qElemFree().
 */
int
qElemDetach(QUEUE q,QELEM elem)
{
    QElem *curr = (QElem*)elem;
    QElem *prev = curr->qePrev;
    QElem *next = curr->qeNext;

    /* Detach curr from the queue */
    if (prev)
        prev->qeNext = next;
    else /* This is the head */
	HEAD(q) = next;

    if (next)
        next->qePrev = prev;
    else /* This is the tail element */
	TAIL(q) = prev;

    curr->qePrev = curr->qeNext = NULL;

    if (CURR(q) == curr)
	CURR(q) = NULL;

    --TOTAL(q);
    return 0;
}

void
qElemFree(QELEM elem)
{
    STDFREE(elem);
}

/*
 * Returns a pointer to the first element in the queue. To get the
 * actual data, call qElemData(QELEM qelem).
 */
QELEM
qElemFirst(QUEUE q)
{
    if (!q || !HEAD(q))
    	return NULL;
    return HEAD(q);
}
/*
 * Returns a pointer to the last element of the queue.
 */
QELEM
qElemLast(QUEUE q)
{
    if (!q || !TAIL(q))
    	return NULL;
    return TAIL(q);
}
/*
 * Given a current qElem, return the next one. 
 */
QELEM
qElemNext(QELEM curr)
{
    if (!curr)
	return NULL;

    return ((QElem*)curr)->qeNext;
}
/*
 * Given a current element, return the previous.
 */
QELEM
qElemPrev(QELEM curr)
{
    if (!curr)
	return NULL;

    return ((QElem*)curr)->qePrev;
}
QELEM
qElemCurr(QUEUE q)
{
    return CURR(q);
}
/*
 * Given a current qElem, remove it from the queue.
 * The data stored with the element is returned.
 * The element itself is free()'d.
 *
 * Returns NULL if it fails, or if you stored a NULL pointer with this
 * element.
 */
void*
qElemRemove(QUEUE q,QELEM elem)
{
    void * data;
    if (!elem)
	return 0;

    /* Give the data back */
    data = ((QElem*)elem)->qeData;

    qElemDetach(q,elem);
    STDFREE(elem);
    return data;
}
