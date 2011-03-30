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
extern const char* nullstr(const char *);
extern const char* okfail(int);

static char*
get_string(void)
{
    int c;
    static char buf[128];

    while ( (c = fgetc(stdin)) == '\n');

    ungetc(c,stdin);
    fgets(buf,sizeof buf,stdin);

    buf[strlen(buf) - 1] = '\0';

    return buf;
}
static void
pfunc(DSKEY key,void *data,int conflict)
{
    if (conflict)
	printf(", ");
    else
    	printf("\n");
    printf("%s",(char*)data);
}
static void
close_func( void * data )
{
    printf("Closing: %s\n",(char*)data);
}

int
main(int argc,char **argv)
{
    HASHTABLE 	ht;
    char *	s;
    int		i;

    ht = htMake(-1);

    for (i = 0; i < argc; i++)
	htAdd(ht,argv[i],(void*)argv[i]);

    printf("\nContents of hash table: %d items, %d slots\n",
	    htItems( ht ), htSize( ht ) );
    htWalk(ht,1,pfunc);
    printf("\n===============\n");

    while (1)
    {
	printf("Conflicts in table: %d\n",htConflicts(ht));
	htWalk(ht,0,pfunc);
	printf("\n");

	printf("Enter key to remove: (\"quit\" to stop): ");
	s = get_string();
	if (!strcmp(s,"quit"))
	    break;
	printf("Removing element with key \"%s\"... = %s\n",s,
		nullstr((char*)htRemove(ht,s)));
    }

    printf("Closing hash table\n" );
    htCloseWithFunction( ht, close_func );
    
    return 0;
}
