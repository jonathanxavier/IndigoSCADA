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
# include <stdio.h>
# include <string.h>
# include "../ds.h"
extern const char * nullstr(const char *);

static int setcmp(void*,void*);		/* Forward declaration */

/* Use this to compare set elements, in this case strings */
int
setcmp(void *s1,void *s2)
{
    return ! strcmp((const char*)s1,(const char*)s2);
}

# define PR_SET(s,set)	\
    for (s = (char*)setFirst(set); s; s = setNext(set))	\
	printf("s = %s\n",nullstr(s));

int
main(int argc,char **argv)
{
    SET set;
    SET set2;
    SET tmp;

    int args = argc / 2;
    char *s;

    /* Create a set and a bag (the latter happens by passing 0 as the 
     * second argument). The last parameter is 1, which means that the elements
     * in the set can be ordered (which is the case with strings).
     */
    set  = setNew((SETCMPFUN)strcmp,1,1);
    set2 = setNew((SETCMPFUN)strcmp,0,1);

    while (args--)
    {
	setAdd(set2,argv[args]);
	setAdd(set,strdup(argv[args + argc / 2]));
    }

    printf("Added %d elements to first set\n",setSize(set));
    PR_SET(s,set);
    printf("Added %d elements to second set\n",setSize(set2));
    PR_SET(s,set2);

    tmp = setUnion1(set,set2);
    printf("Union of set and set2: %d elements\n",setSize(tmp));
    PR_SET(s,tmp);
    setClose(tmp);

    tmp = setUnion1(set2,set);
    printf("Union of set2 and set: %d elements\n",setSize(tmp));
    PR_SET(s,tmp);
    setClose(tmp);

    if ( ( tmp = setIntersect(set2,set)) )
    {
	printf("Intersection of set2 and set: %d elements\n",setSize(tmp));
	PR_SET(s,tmp);
	setClose(tmp);
    }

    if ( (tmp = setXIntersect(set2,set)) )
    {
	printf("XIntersection of set2 and set: %d elements\n",setSize(tmp));
	PR_SET(s,tmp);
	setClose(tmp);
    }
    if ( (tmp = setDifference(set2,set)) )
    {
	printf("Difference of set2 and set: %d elements\n",setSize(tmp));
	PR_SET(s,tmp);
	setClose(tmp);
    }
    if ( (tmp = setDifference(set,set2)) )
    {
	printf("Difference of set and set2: %d elements\n",setSize(tmp));
	PR_SET(s,tmp);
	setClose(tmp);
    }

    return 0;
}
