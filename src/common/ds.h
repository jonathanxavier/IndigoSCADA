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

/* Last modified: Mon Dec 24 11:06:28 EST 2001
 *                Thu Dec 27 15:39:13 EST 2001
 */

# ifndef _LIBDS_H_
# define _LIBDS_H_

# define DS_LIB_VERSION_MAJOR	2
# define DS_LIB_VERSION_MINOR	1
# define DS_LIB_VERSION_STATUS	

# ifdef sun
# define const
# endif

typedef void*	DSKEY;
typedef int 	(*DSKEYCMPFUN)(const DSKEY,const DSKEY);

/*----------------------------------------------------------------------*
 * Tree									*
 *----------------------------------------------------------------------*/
# define AVLTreeNoCopyKeys      0
# define AVLTreeCopyKeys        1
# define AVLTreeNoKeySize       0

# define AVLTreeNoCompFun       ((void*)0)
# define AVLTreeNullData        ((void*)0)

typedef DSKEYCMPFUN		AVLKEYCOMPFUN;

typedef struct avltree	AvlTree;
typedef AvlTree*	AVLTREE;
typedef struct avlnode	AvlNode;
typedef AvlNode*	AVLNODE;


/* Default tree making routine for nodes with string keys */
# define avlNewTreeStrKeys()	\
	avlNewTree((AVLKEYCOMPFUN)strcmp,AVLTreeCopyKeys,AVLTreeNoKeySize)
# define avlMake()		avlNewTree((void*)0,1,0)
# define avlAdd(t,k,d)		avlInsert((t),(const DSKEY)(k),(void*)(d))
# define avlRemove(t,k)		avlRemoveByKey((t),(k))
# define avlFirstNode(tr)	avlMinimumNode((tr))
# define avlLastNode(tr)	avlMaximumNode((tr))
# define avlFirst(tr)		avlMinimum((tr))
# define avlLast(tr)		avlMaximum((tr))
# define avlSize(tr)		avlTotalNodes((tr))

# define AVLPreWalk		0
# define AVLInWalk		1
# define AVLPostWalk		2

/*----------------------------------------------------------------------*
 * Queue								*
 *----------------------------------------------------------------------*/

typedef struct QElem	QElem;
typedef QElem*		QELEM;
typedef struct Queue	Queue;
typedef Queue*		QUEUE;

# define qAdd(q,e)	qEnque((q),(void*)(e))
# define qRemove(q)	qDeque((q))
# define qSize(q)	qLength((q))

/*----------------------------------------------------------------------*
 * Heap									*
 *----------------------------------------------------------------------*/
typedef struct Heap	Heap;
typedef Heap*		HEAP;

# define HEAP_MAXIMIZE		0
# define HEAP_MINIMIZE		1

typedef DSKEYCMPFUN	HEAPCMPFUNC;

typedef void (*HEAPCHGFUNC)(void*,int);
typedef void (*HEAPWALKFUNC)(int,const DSKEY,void*);
typedef void (*HEAPPRINTFUNC)(int,void*,void*);

# define heapAdd(h,k,d)	heapInsert((h),(k),(d))

/*----------------------------------------------------------------------*
 * Hashing								*
 *----------------------------------------------------------------------*/

typedef unsigned (*HASHFUNC)(DSKEY);
typedef DSKEYCMPFUN	HASHCMPFUNC;

typedef struct hashtable	HashTable;
typedef HashTable*		HASHTABLE;

typedef struct Stack	Stack;
typedef Stack*		STACK;
typedef struct PArray	PArray;
typedef PArray*		PARRAY;

/*----------------------------------------------------------------------*
 * Set									*
 *----------------------------------------------------------------------*/
typedef struct _set	Set;
typedef Set*		SET;
typedef Set*		BAG;
typedef int (*SETCMPFUN)(void*,void*);

# define setAppend(s1,s2)	setUnion(s1,s2)

/*----------------------------- ( 139 lines) --------------------------*/
extern void * avlNodeKey(AVLNODE);
extern char * avlNodeKeyAsString(void *);
extern AVLTREE avlNewTree(int(*)(DSKEY,DSKEY),int,int);
extern void avlClose(AVLTREE);
extern void avlCloseWithFunction(AVLTREE,void(*)(void *));
extern void avlWalk(AVLTREE,void(*)(void *),int);
extern void avlWalkAscending(AVLTREE,void(*)(void *));
extern void avlWalkDescending(AVLTREE,void(*)(void *));
extern int avlHeight(AVLTREE);
extern int avlInsert(AVLTREE,const DSKEY,void *);
extern AVLNODE avlFindNode(AVLTREE,DSKEY);
extern void * avlFind(AVLTREE,DSKEY);
extern AVLNODE avlMinimumNode(AVLTREE);
extern void * avlMinimum(AVLTREE);
extern AVLNODE avlMaximumNode(AVLTREE);
extern void * avlMaximum(AVLTREE);
extern AVLNODE avlNextNode(AVLTREE,AVLNODE);
extern AVLNODE avlNextNodeByKey(AVLTREE,DSKEY);
extern AVLNODE avlPrevNode(AVLTREE,AVLNODE);
extern AVLNODE avlPrevNodeByKey(AVLTREE,DSKEY);
extern int avlGetData(AVLNODE,void **);
extern void * avlNodeData(AVLNODE);
extern void * avlNodeUpdateData(AVLNODE,void *);
extern void * avlUpdateData(AVLTREE,DSKEY,void *);
extern void * avlRemoveByKey(AVLTREE,DSKEY);
extern int avlRemoveNode(AVLTREE,AVLNODE);
extern int avlSetCurrent(AVLTREE,DSKEY);
extern int avlClearCurrent(AVLTREE);
extern void * avlCurrent(AVLTREE);
extern void * avlPrev(AVLTREE);
extern void * avlNext(AVLTREE);
extern void * avlCut(AVLTREE);
extern void avlFreeNode(void *);
extern int avlTotalNodes(AVLTREE);
extern AVLNODE avlRootNode(AVLTREE);
extern AVLNODE avlLeftNode(AVLNODE);
extern AVLNODE avlRightNode(AVLNODE);
extern int avlNodeHeight(AVLNODE);
extern int avlCheck(AVLTREE);
extern HASHTABLE htMake(int);
extern HASHTABLE htMakeHashTable(int,HASHFUNC,HASHCMPFUNC);
extern void htClose(HASHTABLE);
extern void htCloseWithFunction(HASHTABLE,void(*)(void*));
extern int htAdd(HASHTABLE,DSKEY,void *);
extern void * htFind(HASHTABLE,DSKEY);
extern void * htRemove(HASHTABLE,DSKEY);
extern int htSize(HASHTABLE);
extern int htItems(HASHTABLE);
extern int htEmpty(HASHTABLE);
extern int htConflicts(HASHTABLE);
extern void htWalk(HASHTABLE,int,void(*)(DSKEY,void *,int));
extern HEAP heapMake(void);
extern HEAP heapNew(int(*)(DSKEY,DSKEY),int,int,int);
extern void heapCloseWithFunction(HEAP,void(*)(void *));
extern void heapClose(HEAP);
extern HEAP heapMakeIntKeys(int,int,int);
extern HEAP heapMakeDoubleKeys(int,int,int);
extern HEAP heapMakeFloatKeys(int,int,int);
extern HEAP heapMakeStringKeys(int,int,int);
extern int heapSetChgFunc(HEAP,HEAPCHGFUNC);
extern int heapInsert(HEAP,const DSKEY,void *);
extern void * heapDelete(HEAP,int);
extern void * heapFirst(HEAP);
extern void * heapPeekFirst(HEAP);
extern void * heapElementAt(HEAP,int);
extern int heapSize(HEAP);
extern int heapEmpty(HEAP);
extern void heapWalk(HEAP,HEAPWALKFUNC);
extern void heapPrintArray(HEAP,HEAPPRINTFUNC);
extern void heapPrintTree(HEAP,HEAPPRINTFUNC);
extern int heapCheck(HEAP);
extern PARRAY paMake(int,int);
extern void paCloseWithFunction(PARRAY,void(*)(void *));
extern void paClose(PARRAY);
extern int paAdd(PARRAY,void *);
extern void * paRemove(PARRAY,int);
extern int paSize(PARRAY);
extern void * paReplace(PARRAY,int,void *);
extern int paContains(PARRAY,int(*)(void *,void *),void *);
extern void * paElementAt(PARRAY,int);
extern void * paCurrent(PARRAY);
extern void paSetCurrent(PARRAY,int);
extern void paClearCurrent(PARRAY);
extern void * paFirst(PARRAY);
extern void * paLast(PARRAY);
extern void * paNext(PARRAY);
extern void * paPrev(PARRAY);
extern QUEUE qMake(void);
extern void qClose(QUEUE);
extern void qCloseWithFunction(QUEUE,void(*)(void *));
extern QELEM qEnque(QUEUE,void *);
extern void * qDeque(QUEUE);
extern int qWalk(QUEUE,void(*)(void *));
extern int qWalkAscending(QUEUE,void(*)(void *));
extern int qWalkDescending(QUEUE,void(*)(void *));
extern int qLength(QUEUE);
extern int qEmpty(QUEUE);
extern void * qCurrent(QUEUE);
extern void qClearCurrent(QUEUE);
extern void qSetCurrent(QUEUE,QELEM);
extern void * qFirst(QUEUE);
extern void * qLast(QUEUE);
extern void * qNext(QUEUE);
extern void * qPrev(QUEUE);
extern void * qElemData(QELEM);
extern int qElemInsert(QUEUE,QELEM,QELEM);
extern int qElemAttach(QUEUE,QELEM);
extern int qElemDetach(QUEUE,QELEM);
extern void qElemFree(QELEM);
extern QELEM qElemFirst(QUEUE);
extern QELEM qElemLast(QUEUE);
extern QELEM qElemNext(QELEM);
extern QELEM qElemPrev(QELEM);
extern QELEM qElemCurr(QUEUE);
extern void * qElemRemove(QUEUE,QELEM);
extern SET setMake(void);
extern SET setNew(SETCMPFUN,int,int);
extern void setClose(SET);
extern void setCloseWithFunction(SET,void(*)(void *));
extern int setAdd(SET,void *);
extern void * setRemove(SET,void *);
extern void * setFirst(SET);
extern void * setNext(SET);
extern int setSize(SET);
extern int setEmpty(SET);
extern int setContains(SET,void *);
extern void * setFind(SET,void *);
extern SET setUnion(SET,SET);
extern SET setUnion1(SET,SET);
extern SET setIntersect(SET,SET);
extern SET setDifference(SET,SET);
extern SET setXIntersect(SET,SET);
extern STACK stkMake(void);
extern void stkCloseWithFunction(STACK,void(*)(void *));
extern void stkClose(STACK);
extern int stkPush(STACK,void *);
extern void * stkPop(STACK);
extern void * stkPeek(STACK);
extern int stkSize(STACK);
extern int stkEmpty(STACK);

# endif /* ! _LIBDS_H_ */
