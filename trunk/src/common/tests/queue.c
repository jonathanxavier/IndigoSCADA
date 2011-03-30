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
# include "../ds.h"
extern const char* nullstr(const char *);
extern const char* okfail(int);

int
main(int argc,char **argv)
{
    QUEUE 	q;
    char *	s;
    int		i;

    if (argc == 1)
    {
	printf("Use: %s arguments\n",argv[0]);
	printf("    The arguments are placed in the queue and various test\n"
	       "    routines are performed on the queue.\n");
	return -1;
    }
    q = qMake();

    for (i = 0; i < argc; i++)
    {
	qAdd(q,(void*)argv[i]);
	printf("current should be last: %s\n",
	       okfail(qLast(q) == qCurrent(q)));
    }

    for (s = (char*)qFirst(q);s; s = (char*)qNext(q))
	printf("s = %s\n",s);
    printf("===============\n");

    printf("qLast() returns %s\n",nullstr((char*)qLast(q)));
    printf("qNext() returns %s (should be (null))\n",nullstr((char*)qNext(q)));
    printf("Current is now %s\n",nullstr((char*)qCurrent(q)));
    printf("qPrev() returns %s (should be same as last)\n",
	   nullstr((char*)qPrev(q)));
    printf("Current is now %s\n",nullstr((char*)qCurrent(q)));
    printf("qNext() returns %s\n",nullstr((char*)qNext(q)));
    printf("qFirst() returns %s\n",nullstr((char*)qFirst(q)));
    printf("qPrev() returns %s\n",nullstr((char*)qPrev(q)));
    printf("Current is now %s\n",nullstr((char*)qCurrent(q)));
    printf("qNext() returns %s\n",nullstr((char*)qFirst(q)));

    printf("\n\nUSE AS LINKED LIST:\n");
    printf("Current is now %s\n",nullstr((char*)qCurrent(q)));
    printf("qPrev() returns %s\n",nullstr((char*)qPrev(q)));
    printf("qPrev() returns %s\n",nullstr((char*)qPrev(q)));
    printf("qCurrent() returns %s\n",nullstr((char*)qCurrent(q)));
    printf("qPrev() returns %s\n",nullstr((char*)qPrev(q)));
    printf("etc....\n");
    return 0;
}
