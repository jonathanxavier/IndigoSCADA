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
    PARRAY 	pa;
    char *	s;
    int		i;

    pa = paMake(3,1);

    for (i = 0; i < argc; i++)
	paAdd(pa,(void*)argv[i]);

    for (s = (char*)paFirst(pa);s; s = (char*)paNext(pa))
	printf("s = %s\n",s);
    printf("===============\n");

    printf("paLast() returns %s\n",nullstr((char*)paLast(pa)));
    printf("paNext() returns %s\n",nullstr((char*)paNext(pa)));
    printf("Current is now %s\n",nullstr((char*)paCurrent(pa)));

    printf("Clearing current.\n");
    paClearCurrent(pa);
    printf("Current is now %s\n",nullstr((char*)paCurrent(pa)));
    printf("paPrev() returns %s\n",nullstr((char*)paPrev(pa)));
    printf("paNext() returns %s\n",nullstr((char*)paNext(pa)));
    printf("paLast() returns %s\n",nullstr((char*)paLast(pa)));
    printf("paPrev() returns %s\n",nullstr((char*)paPrev(pa)));
    printf("Current is now %s\n",nullstr((char*)paCurrent(pa)));
    printf("paFirst() returns %s\n",nullstr((char*)paFirst(pa)));
    printf("Current is now %s\n",nullstr((char*)paCurrent(pa)));
    printf("Setting current to %d\n",3);
    paSetCurrent(pa,3);
    printf("Current is now %s\n",nullstr((char*)paCurrent(pa)));

    printf("paPrev() returns %s\n",nullstr((char*)paPrev(pa)));
    printf("paPrev() returns %s\n",nullstr((char*)paPrev(pa)));
    printf("paCurrent() returns %s\n",nullstr((char*)paCurrent(pa)));
    printf("paNext() returns %s\n",nullstr((char*)paNext(pa)));
    printf("Removing 3rd element\n");
    paRemove(pa,3);
    for (s = (char*)paFirst(pa);s; s = (char*)paNext(pa))
	printf("s = %s\n",s);
    printf("===============\n");

    printf("Removing 1st element\n");
    paRemove(pa,1);
    printf("Removing -3 element\n");
    paRemove(pa,-3);

    for (s = (char*)paFirst(pa);s; s = (char*)paNext(pa))
	printf("s = %s\n",s);
    printf("===============\n");
    
    return 0;
}
