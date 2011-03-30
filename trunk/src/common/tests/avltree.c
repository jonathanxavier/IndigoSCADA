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

/* prototypes for supporting functions in nullstr.c */
extern const char * nullstr(const char *);
extern const char * okfail(int);

static char *	progname;

static int 
print_usage(void)
{
    printf("usage: %s <arg_list>\n",progname);
    printf(
"   The <arg_list> can be a completely random collection of strings.\n"
"   This program builds an AVLTree with the given strigs as data AND\n"
"   as key. Then it proceeds to test all the functions of the AVL Tree\n"
"   implementation.\n");
    return 0;
}
static void
printfunc(void *data)
{
    printf("user-data: %s\n",nullstr((const char*)data));
}
int
main(int argc,char **argv)
{
    AVLTREE 	tree;
    char *	s;

    progname = argv[0];

    if (argc == 1)
    {
	print_usage();
	return 0;
    }

    tree = avlMake();

    while (argc--)
	avlAdd(tree,argv[argc],(void*)argv[argc]);

    printf(" Number of nodes: %d\n",avlTotalNodes(tree));
    printf(" Tree height: %d\n",avlHeight(tree));

    for (s = avlFirst(tree); s; s = avlNext(tree))
    {
	printf("s = %s (%s)\n",s,okfail(s==avlCurrent(tree)));
    }
    printf("Current should now be NULL: %s\n",okfail(avlCurrent(tree)==NULL));

    printf("avlLast() returns %s\n",nullstr((char*)avlLast(tree)));
    printf("===============\n");
    printf("setting current to %s\n",argv[0]);
    avlSetCurrent(tree,argv[0]);
    printf("avlNext() returns %s\n",nullstr((char*)avlNext(tree)));
    printf("Current is now %s\n",nullstr((char*)avlCurrent(tree)));

    printf("Clearing current.\n");
    avlClearCurrent(tree);
    printf("avlCurrent returns %s\n",nullstr((char*)avlCurrent(tree)));
    printf("avlPrev() returns %s\n",nullstr((char*)avlPrev(tree)));
    printf("avlNext() returns %s\n",nullstr((char*)avlNext(tree)));
    printf("avlLast() returns %s\n",nullstr((char*)avlLast(tree)));
    printf("avlPrev() returns %s\n",nullstr((char*)avlPrev(tree)));
    printf("avlFirst() returns %s\n",nullstr((char*)avlFirst(tree)));
    printf("avlCurrent() returns %s\n",nullstr((char*)avlCurrent(tree)));
    printf("avlPrev() returns %s\n",nullstr((char*)avlPrev(tree)));
    printf("avlCurrent() returns %s\n",nullstr((char*)avlCurrent(tree)));
    printf("avlNext() returns %s\n",nullstr((char*)avlNext(tree)));

    printf("+---- avlWalkAscending() ----+\n");
    avlWalkAscending(tree,printfunc);
    printf("+---- avlWalkDescending() ----+\n");
    avlWalkDescending(tree,printfunc);
    printf("+---- avlWalk() ----+\n");
    printf("pre-order:\n");
    avlWalk(tree,printfunc,AVLPreWalk);
    printf("in-order:\n");
    avlWalk(tree,printfunc,AVLInWalk);
    printf("post-order:\n");
    avlWalk(tree,printfunc,AVLPostWalk);

    printf("+---- avlCloseWithFunction() ----+\n");
    avlCloseWithFunction(tree,printfunc);
    return 0;
}
