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

/*----------------------------------------------------------------------*
 * Implementation of AVL trees 						*
 *									*
 * The main idea was taken from:					*
 * "Data structures and program design in C" by Robert L. Kruse, 	*
 * Bruce P. Leung, and Clovis L. Tondo, 1991 Prentice-Hall, Inc,	*
 * ISBN 0-13-726-332-5.							*
 * The implementation of delete was not given in the book, so any	*
 * problems with it are mine not the book's. It seems all those books	*
 * never wanna bother with the really interesting stuff...		*
 *----------------------------------------------------------------------*/

/* Mon Feb 21 16:29:17 CET 2000 */
/* Fri Dec  7 21:08:34 EST 2001 */
/* Sun Dec 30 10:13:49 CET 2001 */

# include "local.h"

# ifndef StandAlone
# 	include <memtraceoff.h>
# endif 

# define AVLNULL		(void*)0
# define  LeftHigh		    -1
# define EqualHigh		     0
# define RightHigh		     1

# define I_ERROR()	fprintf(stderr,"AVL TREE INTERNAL ERROR: %s,%d\n",\
			__FILE__,__LINE__)

struct avlnode
{
    DSKEY		an_key;
    int			an_high;	    /* One of left, equal, right */
    void *	        an_data;	    /* What is stored in the tree */
    struct avlnode *	an_parent;
    struct avlnode *	an_left;
    struct avlnode *	an_right;
};

struct avltree
{
    int			at_nodes;	/* number of nodes in the tree	*/
    int			at_copy_keys;	/* should we copy each key ?	*/
    int			at_key_size;	/* if yes, what is the key size */
    AVLKEYCOMPFUN	at_key_comp;	/* how do we compare keys?	*/
    struct avlnode *	root;
    struct avlnode *	current;	/* to keep track of currencies  */
};

# define NODE(t)	((AvlNode*)(t))		    
# define PARENT(t)	(t)->an_parent
# define LEFT(t)	(t)->an_left
# define RIGHT(t)	(t)->an_right

# define LEAF(n)	(!LEFT((n)) && !RIGHT((n)))
# define ORPHAN(n)	(!PARENT((n)))

# define CUTOFF(n)	(n)->an_right=(n)->an_left=(n)->an_parent=NULL
# define TREE(t)	((AvlTree*)(t))
# define TREEROOT(t)	(((AvlTree*)(t))->root)
# define CURR(t)	(((AvlTree*)(t))->current)

# define LEFTCHILD	-1
# define RIGHTCHILD	 1

# define NodeIsLeftChild(x)  ((x)&&PARENT((x))&&(x)==LEFT(PARENT((x))))
# define NodeIsRightChild(x) ((x)&&PARENT((x))&&(x)==RIGHT(PARENT((x))))

/*----------------------------- (  19 lines) --------------------------*/
static AvlNode * avl_new_node(void);
static void avl_free_node(AvlNode *);
static AvlNode * avl_left_rotate(AvlNode *);
static AvlNode * avl_right_rotate(AvlNode *);
static AvlNode * avl_right_balance(AvlNode *);
static AvlNode * avl_left_balance(AvlNode *);
static AvlNode * avl_insert(AvlTree *,AvlNode *,DSKEY,void *,int *);
static AvlNode * avl_find_node(AvlTree *,AvlNode *,DSKEY);
static AvlNode * avl_minimum_node(AvlNode *);
static AvlNode * avl_maximum_node(AvlNode *);
static int avl_copy_nodes(AvlTree *,AvlNode *,AvlNode *);
static int avl_remove_node(AvlTree *,AvlNode *);
static AvlNode * avl_rem_fix_up_right(AvlNode *);
static AvlNode * avl_rem_fix_up_left(AvlNode *);
static int avlRealTreeHeight(AvlNode *);
static void avl_close_node(AVLNODE,void(*)(void *),int);
static void avl_visit_node(AVLNODE,void(*)(void *),int);
static int avl_check_node(AvlNode *,int);

/*----------------------------------------------------------------------*
 * Allocate/Free memory							*
 *----------------------------------------------------------------------*/
static AvlNode*
avl_new_node(void)
{
    AvlNode* node;

    node = (AvlNode*) malloc(sizeof(AvlNode));

    if (!node)
	return AVLNULL;

    node->an_high   = EqualHigh;
    node->an_key    = (DSKEY) 0;
    node->an_parent = AVLNULL;
    node->an_left   = AVLNULL;
    node->an_right  = AVLNULL;
    node->an_data   = NULL;

    return node;
}
static void
avl_free_node(AvlNode * node)
{
    free( (void*)node);
}
/*----------------------------------------------------------------------*
 * Left rotation. It takes a node, and left rotates it in such a way 	*
 * that the node's right child (if present) becomes the new root.	*
 * The original node becomes the right child's left child (draw a 	*
 * picture....)								*
 *----------------------------------------------------------------------*/
static AvlNode*
avl_left_rotate(AvlNode* root)
{
    AvlNode * node;

    if (!root || !RIGHT(root))
    { 
	DBG(fprintf(stderr,
	    "avl_left_rotate(): root or RIGHT(root) = NULL\n"));
	return AVLNULL;
    }

    node = RIGHT(root);

    RIGHT(root) = LEFT(node);

    if (LEFT(node))
	PARENT(LEFT(node)) = root;

    LEFT(node) = root;

    PARENT(node) = PARENT(root);
    PARENT(root) = node;

    /* The new root is root's right pointer */

    return node;
}
/*----------------------------------------------------------------------*
 * Right rotation. The root's left child becomes the new root.		*
 *----------------------------------------------------------------------*/
static AvlNode*
avl_right_rotate(AvlNode* root)
{
    AvlNode * node;

    if (!root || ! LEFT(root))
	{ XLOG(root); return AVLNULL; }

    node = LEFT(root);

    LEFT(root) = RIGHT(node);

    if (RIGHT(node))
	PARENT(RIGHT(node)) = root;

    RIGHT(node) = root;

    PARENT(node) = PARENT(root);
    PARENT(root) = node;

    return node;
}
static AvlNode*
avl_right_balance(AvlNode* root)
{
    AvlNode * right;

    if (!root || !RIGHT(root))
	{ XLOG(root); return AVLNULL; }

    right = RIGHT(root);

    if (right->an_high == RightHigh)
    {
	root->an_high = EqualHigh;
	right->an_high= EqualHigh;

	return avl_left_rotate(root);
    }
    else if (right->an_high == LeftHigh)
    {
	AvlNode * w = LEFT(right);

	if (w->an_high    == LeftHigh)
	{
	    w->an_high    = EqualHigh;
	    root->an_high = EqualHigh;
	    right->an_high= RightHigh;
	}
	else if (w->an_high == RightHigh)
	{
	    w->an_high    = EqualHigh;
	    root->an_high = LeftHigh;
	    right->an_high= EqualHigh;
	}
	else
	{
	    root->an_high = EqualHigh;
	    right->an_high= EqualHigh;
	}

	RIGHT(root) = avl_right_rotate(right);
	w = avl_left_rotate(root);

	return w;
    }

    return root;
}
static AvlNode* 
avl_left_balance(AvlNode* root)
{
    AvlNode* left;

    if (!root || !LEFT(root))
	{ XLOG(root); return AVLNULL; }

    left = LEFT(root);

    if (left->an_high == LeftHigh)
    {
	left->an_high = EqualHigh;
	root->an_high = EqualHigh;

	return avl_right_rotate(root);
    }
    else if (left->an_high == RightHigh)
    {
	AvlNode * w = RIGHT(left);

	if (w->an_high    == LeftHigh)
	{
	    w->an_high    = EqualHigh;
	    root->an_high = RightHigh;
	    left->an_high = EqualHigh;
	}
	else if (w->an_high == RightHigh)
	{
	    w->an_high    = EqualHigh;
	    root->an_high = EqualHigh;
	    left->an_high = LeftHigh;
	}
	else
	{
	    root->an_high = EqualHigh;
	    left->an_high = EqualHigh;
	}

	LEFT(root) = avl_left_rotate(left);
	w = avl_right_rotate(root);

	/* w has become the new root */
	return w;
    }

    return root;
}
static AvlNode*
avl_insert(AvlTree * tree,AvlNode *root,DSKEY key,void *data,int *grew)
{
    AvlNode * tmp;
    int	      comp;

    if (!root)
    {
	if (! (root = avl_new_node()) )
	    { XLOG(root); return AVLNULL; }

	/*--------------------------------------------------------------*
	 * if we are instructed to copy the keys, then we proceed as 	*
	 * follows: if tree->at_key_size is zero, we assume that keys	*
	 * are simple strings, and do a strcpy. If tree->at_key_size	*
	 * is larger than zero, then we do an appropriate malloc() and	*
	 * a memcpy().							*
	 *--------------------------------------------------------------*/
	  
	if (tree->at_copy_keys)
	{
	    if (tree->at_key_size)
	    {
		if (! (root->an_key = 
		       (DSKEY) malloc((size_t)tree->at_key_size)) )
		{
		    XLOG(root);
		    avl_free_node(root);
		    return AVLNULL;
		}

		memcpy(root->an_key,key,(size_t)tree->at_key_size);
	    }
	    else
	    {
		/* Assume strings as keys */
		if (! (root->an_key  = (DSKEY) strdup((char*)key)) )
		{
		    XLOG(root);
		    avl_free_node(root);
		    return AVLNULL;
		}
	    }
	}
	else
	{
	    root->an_key = key;
	}

	root->an_data = data;
	root->an_high = EqualHigh;
	root->an_left = root->an_right = AVLNULL;
	root->an_parent = AVLNULL;

	*grew = 1;

	tree->current = root;
	return root;
    }

    comp = tree->at_key_comp(key,root->an_key);

    if (comp < 0)	    /* Node key is smaller than root */
    {
	tmp = avl_insert(tree,LEFT(root),key,data,grew);

	if (!tmp)
	    { XLOG(tmp); return AVLNULL; }

	/* Mark the parent of the new left node */
	LEFT(root) = tmp;
	PARENT(LEFT(root)) = root;

	if (*grew)	   /* Left tree grew in size */
	{
	    if (root->an_high == LeftHigh)
	    { 
		/* Root needs to be left-balanced */
		if (! (root = avl_left_balance(root)) )
		    { XLOG(root); return AVLNULL; }

		*grew = 0;

	    }
	    else if (root->an_high == EqualHigh)
		root->an_high = LeftHigh;
	    else
	    {
		/* Root was RightHigh, since insertion happened in the
		 * left subtree this means this tree did not grow in
		 * size (of course the left subtree did grow in size)
		 * */

		root->an_high = EqualHigh;
		*grew = 0;
	    }
	}

	return root;
    }
    else if (comp > 0)
    {
	tmp = avl_insert(tree,RIGHT(root),key,data,grew);

	if (!tmp)
	    { XLOG(tmp); return AVLNULL; }

	RIGHT(root) = tmp;

	PARENT(RIGHT(root)) = root;

	if (*grew)
	{
	    if (root->an_high == LeftHigh)
	    {
		root->an_high = EqualHigh;
		*grew = 0;
	    }
	    else if (root->an_high == EqualHigh)
		root->an_high = RightHigh;
	    else
	    {
		if (! (root = avl_right_balance(root)) )
		    { XLOG(root); return AVLNULL; }

		*grew = 0;
	    }
	}

	return root;
    }

    /* Duplicate key */
    DBG(fprintf(stderr,"avl_insert(): duplicate key\n"));

    return AVLNULL;
}
/*----------------------------------------------------------------------*
 * Find nodes in the tree						*
 *----------------------------------------------------------------------*/
static AvlNode *
avl_find_node(AvlTree * tree,AvlNode* node,DSKEY key)
{
    int cmp;
    
    if (!tree)
	{ XLOG(tree); return AVLNULL; }
    if (!node)
	{ XLOG(node); return AVLNULL; }

    cmp = tree->at_key_comp((void*)key,(void*)node->an_key);

    return cmp <  0 ? avl_find_node(tree, LEFT(node),key) :
	   cmp == 0 ? node :
	   cmp >  0 ? avl_find_node(tree,RIGHT(node),key) : NULL;
}
/* 
 * Find the minimum/maximum node starting from a given node.
 */
static AvlNode*
avl_minimum_node(AvlNode * node)
{
    if (!node)
	{ XLOG(node); return AVLNULL; }

    while (LEFT(node))
	node = LEFT(node);

    return node;
}
static AvlNode*
avl_maximum_node(AvlNode* node)
{
    if (!node)
	{ XLOG(node); return AVLNULL; }

    while (RIGHT(node))
	node = RIGHT(node);

    return node;
}

static int
avl_copy_nodes(AvlTree * tree,AvlNode * dest,AvlNode * src)
{
    dest->an_data = src->an_data;

    /* Copy the node fields properly */

    if (!tree->at_copy_keys)
	dest->an_key = src->an_key;
    else
    {
	free( (void*) dest->an_key);
	
	if (tree->at_key_size)
	    memcpy(dest->an_key,src->an_key,(size_t)tree->at_key_size);
	else
	{
	    dest->an_key = malloc(strlen((char*) dest->an_key) + 1);
	    
	    if (!dest->an_key)
		{ XLOG(dest); return -1; }
	    
	    strcpy(dest->an_key,src->an_key);
	}
    }
    return 0;
}
/*----------------------------------------------------------------------*
 * Remove a node from the tree						*
 * All kinds of administrative stuff needs to be done, check out the 	*
 * code itself.								*
 *----------------------------------------------------------------------*/
static int
avl_remove_node(AvlTree * tree,AvlNode * node)
{
    AvlNode * root;
    AvlNode * parent;
    int	      l_child,r_child;

    if (!tree || !node)
	{ XLOG(tree); XLOG(node); return -1; }

# define BIND(node) 	\
	if (l_child) \
	{\
	    LEFT(parent) = (node);\
	    avl_rem_fix_up_right(parent);\
	}\
	else if (r_child)\
	{\
	    RIGHT(parent)= (node);\
	    parent = avl_rem_fix_up_left(parent);\
	}\
	else			/* Node has no parent nor children */\
	{\
	    DBG(printf("DEL: root node, line: %d\n",__LINE__));\
	    root = (node);\
	}

    root    = tree->root;

    /* Case 0: node has both a LEFT and a RIGHT child */
    if (LEFT(node) && RIGHT(node))
    {
	AvlNode * pred = avlPrevNode(tree,node);
	
	if (!pred)
	    { I_ERROR(); return -1; }

	if (avl_copy_nodes(tree,node,pred))
	    { XLOG(node); return -1; }

	/* Replace node's data by prev's data in the tree */
	node = pred;
    }

    l_child = NodeIsLeftChild(node);
    r_child = NodeIsRightChild(node);
    parent  = PARENT(node);

    /* Case 1: node is a LEAF */
    if (LEAF(node))
    {
	BIND(AVLNULL);
	CUTOFF(node);
    }
    else if (LEFT(node) && !RIGHT(node))
    {	
	/* Case 2: node has only a LEFT child */

	PARENT(LEFT(node)) = parent;
	BIND(  LEFT(node));
	CUTOFF(node);
    }
    else if (RIGHT(node) && !LEFT(node))
    {
        /* Case 3: node has only a RIGHT child */
	PARENT(RIGHT(node)) = parent;
	BIND(  RIGHT(node));
	CUTOFF(node);
    }
    else
	{ I_ERROR(); return -1; }

    /* if avlRemoveFix...() went all the way up to the root, and
     * rotated it out of position, then it will have a parent. Go up
     * the tree until we find the new real root and return it */
    
    if (root)				/* If any nodes left in tree	*/
	while (PARENT(root))
	    root = PARENT(root);
    
    tree->root = root;
    
    return 0;
    
}
/*----------------------------------------------------------------------*
 * avl_rem_fix_up_right(): called when a node was deleted from p's left	*
 * subtree so that the right subtree might need fixing up.		* 
 * Shorter indicates upon return if the tree rooted at p itself		*
 * is shorter as a result of the deletion of the node.			*
 *----------------------------------------------------------------------*/
static AvlNode*
avl_rem_fix_up_right(AvlNode * p)
{
    AvlNode * 	pp;
    int		l_child,r_child;

    if (!p)
	{ XLOG(p); return p; }

    pp 	    = PARENT(p);
    l_child = NodeIsLeftChild(p);
    r_child = NodeIsRightChild(p);

    DBG(printf("avl_rem_fix_up_right()\n")); XLOG(p);

    switch (p->an_high)
    {
    case LeftHigh:		/* Case 1 : shorter */

	DBG(printf("Case 1\n"));
	
	p->an_high = EqualHigh;

	if (l_child) 	  return avl_rem_fix_up_right(pp);
	else if (r_child) return avl_rem_fix_up_left (pp);

	return p;
	
    case EqualHigh:		/* Case 2 : not shorter */

	DBG(printf("Case 2\n"));

	p->an_high = RightHigh;
	
	DBG(printf("new root: '%s'\n",avlNodeKeyAsString(p)));

	return p;
	
    case RightHigh:		/* Case 3 */
      {
	AvlNode * q = RIGHT(p);
	
	if (!q) { I_ERROR(); return AVLNULL; }

	if (q->an_high == EqualHigh) /* Case 3a : not shorter */
	{
	    DBG(printf("Case 3a\n"));

	    p->an_high = RightHigh;
	    q->an_high =  LeftHigh;
	    
	    p = avl_left_rotate(p);
	    
	    if (l_child)	LEFT(pp) = p;
	    else if (r_child)	RIGHT(pp)= p;

	    DBG(printf("new root: '%s'\n",avlNodeKeyAsString(p)));

	    return p;
	}
	else if (q->an_high == RightHigh) /* Case 3b : shorter */
	{
	    DBG(printf("Case 3b\n"));

	    p->an_high = EqualHigh;
	    q->an_high = EqualHigh;

	    p = avl_left_rotate(p);

	    if (l_child)
	    {
		LEFT(pp) = p;
		return avl_rem_fix_up_right(pp);
	    }
	    else if (r_child)
	    {
		RIGHT(pp)= p;
		return avl_rem_fix_up_left (pp);
	    }

	    DBG(printf("new root: '%s'\n",avlNodeKeyAsString(p)));

	    return p;
	}
	else		/* Case 3c : shorter */
	{
	    AvlNode * r = LEFT(q);

	    DBG(printf("Case 3c\n"));

	    if (!r) { I_ERROR(); return AVLNULL; }

	    switch (r->an_high)
	    {
	    case LeftHigh:
		p->an_high = EqualHigh;
		q->an_high = RightHigh;
		break;

	    case EqualHigh:
		p->an_high = EqualHigh;
		q->an_high = EqualHigh;
		break;

	    case RightHigh:
		p->an_high =  LeftHigh;
		q->an_high = EqualHigh;
		break;
	    }
	    
	    r->an_high = EqualHigh;
	    
	    r = avl_right_rotate(q);
	    RIGHT(p) = r;
	    r =  avl_left_rotate(p);

	    if (l_child)	LEFT(pp) = r;
	    else if (r_child)	RIGHT(pp)= r;

	    if (NodeIsLeftChild(r))
	    {
		LEFT(PARENT(r)) = r;
		return avl_rem_fix_up_right(PARENT(r));
	    }
	    else if (NodeIsRightChild(r))
	    {
		RIGHT(PARENT(r))= r;
		return avl_rem_fix_up_left (PARENT(r));
	    }

	    DBG(printf("new root: '%s'\n",avlNodeKeyAsString(r)));

	    return r;
	}
      }
    }

    /*** NEVER REACHED ***/
    fprintf(stderr,"%s,%d: It is impossible that we should be here!\n",
    	__FILE__,__LINE__);
    return AVLNULL;
}
static AvlNode*
avl_rem_fix_up_left(AvlNode * p)
{
    AvlNode * 	pp;
    int		l_child,r_child;

    if (!p)
	return p;

    pp 	    = PARENT(p);
    l_child = NodeIsLeftChild(p);
    r_child = NodeIsRightChild(p);

    DBG( printf("avl_rem_fix_up_left()\n") );

    switch (p->an_high)
    {
    case RightHigh:		/* Case 1 : shorter */

	DBG(printf("Case 1\n"));

	p->an_high = EqualHigh;
	
	if (l_child) 	  return avl_rem_fix_up_right(pp);
	else if (r_child) return avl_rem_fix_up_left (pp);
	
	DBG(printf("new root: '%s'\n",avlNodeKeyAsString(p)));
	
	return p;
	
    case EqualHigh:		/* Case 2 : not shorter */
	
	DBG(printf("Case 2\n"));
	p->an_high = LeftHigh;

	DBG(printf("new root: '%s'\n",avlNodeKeyAsString(p)));

	return p;

    case LeftHigh:		/* Case 3 */
      {
	AvlNode * q = LEFT(p);

	if (!q) { I_ERROR(); return AVLNULL; }

	if (q->an_high == EqualHigh) /* Case 3a : not shorter */
	{
	    DBG(printf("Case 3a\n"));

	    p->an_high =  LeftHigh;
	    q->an_high = RightHigh;

	    p = avl_right_rotate(p);

	    if (l_child)	LEFT(pp) = p;
	    else if (r_child)	RIGHT(pp)= p;

	    DBG(printf("new root: '%s'\n",avlNodeKeyAsString(p)));

	    return p;
	}
	else if (q->an_high == LeftHigh) /* Case 3b : shorter */
	{
	    AvlNode * parent = PARENT(p);

	    DBG(printf("Case 3b\n"));

	    p->an_high = EqualHigh;
	    q->an_high = EqualHigh;

	    p = avl_right_rotate(p);

	    if (l_child)
	    {
		LEFT(parent) = p;

		return avl_rem_fix_up_right(parent);
	    }
	    else if (r_child)
	    {
		RIGHT(parent)= p;

		return avl_rem_fix_up_left (parent);
	    }

	    DBG(printf("new root: '%s'\n",avlNodeKeyAsString(p)));

	    return p;
	}
	else		/* Case 3c : shorter */
	{
	    AvlNode * r = RIGHT(q);

	    DBG(printf("Case 3c\n"));

	    if (!r) { I_ERROR(); return AVLNULL; }
	    
	    switch (r->an_high)
	    {
	    case LeftHigh:
		p->an_high = RightHigh;
		q->an_high = EqualHigh;
		break;

	    case EqualHigh:
		p->an_high = EqualHigh;
		q->an_high = EqualHigh;
		break;

	    case RightHigh:
		p->an_high = EqualHigh;
		q->an_high =  LeftHigh;
		break;
	    }
	    
	    r->an_high = EqualHigh;

	    LEFT(p) = avl_left_rotate( q);
	    r 	    = avl_right_rotate(p);

	    /* Update parent pointer */

	    if (l_child)	LEFT(pp) = r;
	    else if (r_child)	RIGHT(pp)= r;

	    if (NodeIsLeftChild(r))
	    {
		LEFT(PARENT(r)) = r;
		return avl_rem_fix_up_right(PARENT(r));
	    }
	    else if (NodeIsRightChild(r))
	    {
		RIGHT(PARENT(r))= r;
		return avl_rem_fix_up_left (PARENT(r));
	    }

	    /* r is now the new root of the tree */
	    DBG(printf("new root: '%s'\n",avlNodeKeyAsString(r)));

	    return r;
	}
      }
    }

    /*** never REACHED ***/
    fprintf(stderr,"%s,%d: It is impossible that we should be here!\n",
    	__FILE__,__LINE__);
    return AVLNULL;
}

/*----------------------------------------------------------------------*
 * Return the height of the tree, as a maximum of the highest subbranch	*
 *----------------------------------------------------------------------*/
static int
avlRealTreeHeight(AvlNode* node)
{
    int n = 0, l = 0, r = 0;

    if (!node)
	return 0;

    if (node)		n++;
    if (LEFT(node))	l = avlRealTreeHeight( LEFT(node));
    if (RIGHT(node))	r = avlRealTreeHeight(RIGHT(node));

    n += l > r ? l : r;

    return n;
}

/*----------------------------------------------------------------------*
 *                                                                      *
 * Exported to the outside world					*
 *                                                                      *
 *----------------------------------------------------------------------*/

/*
 * Get a node's properties 
 */
void*
avlNodeKey(AVLNODE node)
{ 
    return node ? NODE(node)->an_key : NULL;
}
char*
avlNodeKeyAsString(void* node)
{
    return node ? (char*) NODE(node)->an_key : NULL;
}
/*----------------------------------------------------------------------*
 * 			    Make a new tree.				*
 *									*
 * Per default, if cmpfun is NULL, then strcmp() is used, since we	*
 * assume null terminated strings as keys. In this case it is not	*
 * necessary to set the key_size to anything. If copy_keys is not 0	*
 * then the tree will copy each key into an own buffer. This can be 	*
 * usefull if you have keys that are updated from the same dynamic 	*
 * memory buffers. If anything else besides strings is used as key	*
 * then the key size must be given (if copy_keys is on).		*
 *----------------------------------------------------------------------*/
AVLTREE
avlNewTree(int(*cmpfun)(DSKEY,DSKEY),int copy_keys,int key_size)
{
    AvlTree *tr;

    tr = (AvlTree*) malloc(sizeof(AvlTree));

    if (!tr)
	return NULL;
    tr->at_copy_keys = copy_keys;
    tr->at_key_size  = key_size;
    LLOG(tr->at_copy_keys);
    LLOG(tr->at_key_size);

    if (!cmpfun)
	tr->at_key_comp = (AVLKEYCOMPFUN) strcmp;
    else 
	tr->at_key_comp = cmpfun;

    tr->at_nodes	= 0;
    tr->root		= AVLNULL;
    tr->current 	= AVLNULL;

    return (AVLTREE)tr;
}

/*----------------------------------------------------------------------*
 * Closes the tree, effectively removing it from memory. Called		*
 * recursively for each node of the tree. If func is not NULL, it is	*
 * applied on the node before freeing it.				*
 *----------------------------------------------------------------------*/
static void
avl_close_node(AVLNODE n,void (*func)(void*),int free_keys)
{
    AvlNode * node = (AvlNode*) n;

    if (!n)
    	return;

    if (LEFT(node))			/* Go into right subtree	*/
	avl_close_node( LEFT(node),func,free_keys);
    if (RIGHT(node))
	avl_close_node(RIGHT(node),func,free_keys);

    /* We closed both left and right subnodes, close this node too and
       return */

    if (func)
	func(node->an_data);

    /* If free_keys was on, we must free the keys as well */
    if (free_keys)
	free( (void*) node->an_key);

    free( (void*) node);

    return;
}
void
avlClose(AVLTREE tr)
{
    avlCloseWithFunction(tr,NULL);
}
void
avlCloseWithFunction(AVLTREE tr,void (*func)(void*))
{
    if (!tr)
	return;

    /* Close the whole tree */
    avl_close_node(TREEROOT(tr),func,TREE(tr)->at_copy_keys);

    /* And free the tree itself too */

    free( tr );
}
static void
avl_visit_node(AVLNODE n,void (*func)(void*),int type)
{
    AvlNode * node = (AvlNode*) n;

    if (type == AVLPreWalk)
	if (func)
	    func(node->an_data);

    if (LEFT(node))
	avl_visit_node( LEFT(node),func,type);

    if (type == AVLInWalk)
	if (func)
	    func(node->an_data);

    if (RIGHT(node))
	avl_visit_node(RIGHT(node),func,type);

    if (type == AVLPostWalk)
	if (func)
	    func(node->an_data);
}
void
avlWalk(AVLTREE tr,void (*func)(void*),int type)
{
    if (!tr)
	return;
    (void)avl_visit_node(TREEROOT(tr),func,type);
}
void
avlWalkAscending(AVLTREE tr,void (*func)(void*))
{
    AvlNode * node = avlMinimumNode(tr);

    while (node)
    {
	if (func)
	    func(node->an_data);

	node = avlNextNode(tr,node);
    }
}
void
avlWalkDescending(AVLTREE tr,void (*func)(void*))
{
    AvlNode * node = avlMaximumNode(tr);

    while (node)
    {
	if (func)
	    func(node->an_data);

	node = avlPrevNode(tr,node);
    }
}
/*
 *
 */
int
avlHeight(AVLTREE tr)
{
    if (!tr || !TREEROOT(tr))
	return 0;

    return avlRealTreeHeight(TREEROOT(tr));
}
/*
 * Insert a node in tr
 */
int
avlInsert(AVLTREE tr,const DSKEY key,void * data)
{
    AvlTree * 	tree = TREE(tr);
    AvlNode *	new_root;
    int 	grew = 0;

    if (!tr)
	{ XLOG(tr); return -1; }

    new_root = (AvlNode*) avl_insert(tr,tree->root,key,data,&grew);

    if (!new_root)
	/* Duplicate entry found */
	return -1;

    tree->root = new_root;
    tree->at_nodes++;

    return 0;
}
/*
 * Find a node in the tree tr. 
 */
AVLNODE
avlFindNode(AVLTREE tr,DSKEY key)
{
    return CURR(tr) = avl_find_node(TREE(tr),TREEROOT(tr),key);
}
void*
avlFind(AVLTREE tr,DSKEY key)
{
    AvlNode * node;

    node = avlFindNode(tr,key);
    if (!node)
    	{ XLOG(node); return NULL; }

    return node->an_data;
}
/*
 * Find the minimum node in a tree tr. Set the currency to it as well.
 */
AVLNODE
avlMinimumNode(AVLTREE tr)
{
    if (!tr)
	{ XLOG(tr); return NULL; }

    return ((AvlTree*)tr)->current = avl_minimum_node(TREEROOT(tr));
}
/*
 * Find the data that is stored with the minimum node
 */
void*
avlMinimum(AVLTREE tr)
{
    AvlNode *node;

    node = avlMinimumNode(tr);

    if (!node)
    	{ XLOG(node); return NULL; }

    return node->an_data;
}
/*
 * Find the maximum node in a tree tr
 */
AVLNODE
avlMaximumNode(AVLTREE tr)
{
    if (!tr)
	{ XLOG(tr); return NULL; }

    return TREE(tr)->current = avl_maximum_node(TREEROOT(tr));
}
void*
avlMaximum(AVLTREE tr)
{
    AvlNode *node;
    
    if (!tr)
	{ XLOG(tr); return NULL; }

    node = avlMaximumNode(tr);

    if (!node)
    	{ XLOG(node); return NULL; }

    return node->an_data;
}
/* 
 * Given a node n in the tree rooted at tr, find its immediate successor.
 * The tree's currency is set to the new node.
 */
AVLNODE
avlNextNode(AVLTREE tr,AVLNODE n)
{
    AvlNode* node = NODE(n);

    if (!tr)
	{ XLOG(tr); return NULL; }

    if (RIGHT(node))
	return TREE(tr)->current = avl_minimum_node(RIGHT(node));

    while (PARENT(node) && node == RIGHT(PARENT(node)))
	node = PARENT(node);

    return TREE(tr)->current = PARENT(node);
}
/*
 * Find the successor of the node with the given key in the subtree
 * rooted at tr.  */
AVLNODE
avlNextNodeByKey(AVLTREE tr,DSKEY key)
{
    AvlNode* node;

    node = avlFindNode(tr,key);

    if (!node)
	{ XLOG(node); return NULL; }
    return avlNextNode(tr,(AVLNODE)node);
}
/*
 * Given a node in the tree, find its immediate predecessor.
 */
AVLNODE
avlPrevNode(AVLTREE tr,AVLNODE n)
{
    AvlNode* node = NODE(n);

    if (!tr)
	{ XLOG(tr); return NULL; }

    if (LEFT(node))
	return TREE(tr)->current = avl_maximum_node(LEFT(node));

    while (PARENT(node) && node == LEFT(PARENT(node)))
	node = PARENT(node);

    return TREE(tr)->current = PARENT(node);
}
/*
 * Find the predecessor of the node with the given key in the subtree
 * rooted at tr.  */
AVLNODE
avlPrevNodeByKey(AVLTREE tr,DSKEY key)
{
    AvlNode* node;

    node = avlFindNode(tr,key);

    if (!node)
	{ XLOG(node); return NULL; }

    return avlPrevNode(tr,node);
}

/*
 * Get a node's data
 */
int
avlGetData(AVLNODE node,void **data)
{
    if (!node)
	{ XLOG(node); return -1; }

    if (data)
	*data = NODE(node)->an_data;

    return 0;
}
void*
avlNodeData(AVLNODE node)
{
    if (!node)
    	{ XLOG(node); return NULL; }

    return NODE(node)->an_data;
}
void*
avlNodeUpdateData(AVLNODE node,void* new_data)
{
    void *old_data;

    if (!node)
	{ XLOG(node); return NULL; }

    old_data = NODE(node)->an_data;

    NODE(node)->an_data = new_data;

    return old_data;
}
void*
avlUpdateData(AVLTREE tr,DSKEY key,void* new_data)
{
    return avlNodeUpdateData(avlFindNode(tr,key),new_data);
}
/*
 * Remove a node matching the key provided from the tree. The node is
 * freed, and we return its data on success. Otherwise, NULL is returned.
 */
void*
avlRemoveByKey(AVLTREE tr,DSKEY key)
{
    AvlNode * node = avlFindNode(tr,key);
    void *    data;

    if (!node)
	{ XLOG(node); return NULL; }

    data = node->an_data;

    avlRemoveNode(tr,node);

    return data;
}
/*----------------------------------------------------------------------*
 * Remove a node from the tree tr. If the function returns 0, then it	*
 * was successfull. After the call, node is still intact, it can be used*
 * further, or it can be freed with a call to avlFree().		*
 *----------------------------------------------------------------------*/
int
avlRemoveNode(AVLTREE tr,AVLNODE node)
{
    AvlTree * tree = TREE(tr);

    if (!tr)
	{ XLOG(tr); return -1; }

    if (!node)
	{ XLOG(node); return -1; }	    /* No such node */

    if (tree->current == node)
	tree->current = NULL;

    if (avl_remove_node(TREE(tr),NODE(node)))
    {
	fprintf(stderr,"avlRemoveNode(): UNFORSEEN ERROR OCCURRED\n");
	return -1;			    /* A nasty error ?? */
    }

    tree->at_nodes--;

    return 0;
}
/*----------------------------------------------------------------------*
 * Support for currencies						*
 *----------------------------------------------------------------------*/
int
avlSetCurrent(AVLTREE tr,DSKEY key)
{
    AvlNode *	node;
    if (!(node = avlFindNode(tr,key)))
	{ XLOG(node); return -1; }

    TREE(tr)->current = node;
    return 0;
}
int
avlClearCurrent(AVLTREE tr)
{
    if (!tr)
	{ XLOG(tr); return -1; }
    TREE(tr)->current = NULL;
    return 0;
}
/* Get the data stored with the current node. If current has not been 
 * set, then NULL is returned */
void*
avlCurrent(AVLTREE tr)
{
    AvlTree * tree = (AvlTree *)tr;

    if (!tree)
	{ XLOG(tree); return NULL; }
    if (tree->current)
	return tree->current->an_data;
    return NULL;
}
/*
 * Get the previous data. If the currency is not set, then avlPrev()
 * returns the last node in the tree
 */
void*
avlPrev(AVLTREE tr)
{
    AvlTree * tree = (AvlTree *)tr;
    AvlNode * node;

    if (!tree)
	{ XLOG(tr); return NULL; }

    if (!tree->current)
	return avlMaximum(tr);

    node = avlPrevNode(tr,tree->current);

    return node ? node->an_data : NULL;
}
/*
 * Get the next node's data. If no currency is set, then avlNext() gets
 * the first node in the tree
 */
void*
avlNext(AVLTREE tr)
{
    AvlTree * tree = (AvlTree *)tr;
    AvlNode * node;

    if (!tr)
	{ XLOG(tr); return NULL; }

    if (!tree->current)
	return avlMinimum(tr);

    node = avlNextNode(tr,tree->current);

    return node ? node->an_data : NULL;
}
/*
 * Remove the current node from the tree. Node's data is returned to 
 * the caller. Currency is lost after this operation.
 */
void*
avlCut(AVLTREE tr)
{
    AvlNode *	curr = tr->current;
    
    if (avlRemoveNode(tr,tr->current))
	{ XLOG(tr); XLOG(curr); return NULL; }
	
    return curr ? curr->an_data : NULL;
}
/*----------------------------------------------------------------------*
 * Free a node								*
 * The data stored with the node remains untouched.			*
 *----------------------------------------------------------------------*/
void
avlFreeNode(void * node)
{
    if (!node)
	return;

    CUTOFF(NODE(node));
    
    if (NODE(node)->an_key)
	free( (void*) NODE(node)->an_key);
    
    free(node);
}
/*----------------------------------------------------------------------*
 * Some more specific routines.						*
 *----------------------------------------------------------------------*/
int     avlTotalNodes(AVLTREE tr) { return TREE(tr)->at_nodes; 	}
AVLNODE avlRootNode(  AVLTREE tr) { return TREEROOT(tr);	}
AVLNODE avlLeftNode(  AVLNODE n)  { return LEFT(NODE(n));	}
AVLNODE avlRightNode( AVLNODE n)  { return RIGHT(NODE(n));	}
int     avlNodeHeight(AVLNODE n)  { return NODE(n)->an_high;	}

/*----------------------------------------------------------------------*
 * Check the tree for consistency.					*
 *----------------------------------------------------------------------*/

/* Thu Nov 29 23:45:33 CET 2001 */

/* An AVL tree has the following property: the heights of its left and
 * right children should not differ by more than 1, and, both its 
 * children are themselves AVL trees.
 * If the tree implementation is error free (in an ideal world), then 
 * running this is pointless. If the implementation should change, then
 * it is advisable to run this as the tree is being built in order to
 * detect the problems (a.k.a. ``bugs'').
 */
static int
avl_check_node(AvlNode *node,int depth)
{
    int left_h = -1,right_h = -1;
    int has_left = 0;
    int has_right= 0;

    if (LEAF(node))
	return 0;

    if (LEFT(node))
    {
	has_left = 1;
	left_h = avl_check_node( LEFT(node),depth + 1);
    }
    if (RIGHT(node))
    {
	has_right = 1;
    	right_h= avl_check_node(RIGHT(node),depth + 1);
    }

    if (!has_left || !has_right)
    {
    	if ( (has_left  && left_h  > 0) || 
	     (has_right && right_h > 0) )
	{
	    const char * which = has_left ? " left " : " right ";

	    printf("Error in tree, %s child only has height"
	     	   " of more than 1!\n",which);
	    exit(0);
	}
    }
    if ( (left_h < right_h && right_h - left_h > 1)	||
    	 (left_h > right_h && left_h - right_h > 1) )
    {
	printf("ERROR in the tree, depth = %d\n",depth);
	printf("left height = %d, right height = %d\n",
		left_h,right_h);
	exit(-1);
    }
    return 1 + (left_h > right_h ? left_h : right_h);
}
int
avlCheck(AVLTREE tr)
{
    AvlTree * tree = TREE(tr);

    if (!tr)
	{ XLOG(tr); return -1; }

    return avl_check_node(tree->root,0);
}
