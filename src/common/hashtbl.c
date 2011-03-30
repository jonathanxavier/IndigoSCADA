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
 * Last modified: Mon Dec 24 10:37:00 EST 2001
 * $Id: hashtbl.c,v 1.4 2003/02/08 14:19:56 kingofgib Exp $
 */

# include "local.h"

typedef struct _bucket
{
    struct _bucket *	next;
    struct _bucket **	prev;
    int			idx;

    DSKEY		key;
    void *		data;
} HashBucket;

struct hashtable
{
    unsigned long	ht_size;
    int 		ht_items;
    int			ht_conflicts;

    HASHFUNC		ht_hashfun;
    HASHCMPFUNC		ht_cmpfun;
    HashBucket**	ht_table;
};

/*----------------------------- (  10 lines) --------------------------*/
static HashBucket * new_bucket(DSKEY,void *);
static void free_bucket(HashBucket *);
static HashBucket * find_bucket(HASHTABLE,DSKEY,int *);
static void remove_bucket(HASHTABLE,HashBucket *);
static unsigned hash1(unsigned char *);
/* static unsigned hash2(unsigned char *); */
static unsigned hash3(unsigned char *);
/* static unsigned hash4(unsigned char *); */
static int hashcmp1(unsigned char *,unsigned char *);

HASHTABLE
htMake(int size)
{
    return htMakeHashTable(size,(HASHFUNC)hash1,(HASHCMPFUNC)hashcmp1);
}
HASHTABLE
htMakeHashTable(int size,HASHFUNC hashfun,HASHCMPFUNC cmpfun)
{
    HashTable * 	p;

    DBG(debug("htMakeHashTable(size=%d,hashfun=%p,cmpfun=%p)\n",
	size,hashfun,cmpfun));

    if (size <= 0)
	size = 509;

    STDMALLOC(p,sizeof(HashTable),NULL);
    STDMALLOC(p->ht_table,size*sizeof(HashBucket*),NULL);
    memset(p->ht_table,0,size*sizeof(HashBucket*));

    DBG(debug("Created table of size %i\n",size));

    p->ht_size		= size;
    p->ht_items		= 0;
    p->ht_conflicts	= 0;
    p->ht_hashfun 	= hashfun;
    p->ht_cmpfun 	= cmpfun;
    
    return p;
}
void
htClose( HASHTABLE ht )
{
    htCloseWithFunction( ht, NULL );
}
void
htCloseWithFunction( HASHTABLE ht, void (*func)(void*) )
{
    HashBucket * bkt;
    unsigned 	 i;

    for ( i = 0; i < ht->ht_size; i++ )
    {
	/*
	 * While there's buckets in this slot, free 'em.
	 */
	while ( (bkt = ht->ht_table[i]) )
	{
	    /*
	     * Call the func() given on this bucket's data
	     * We assume func() will clean up the data.
	     */

	    func( bkt->data );
	    /*
	     * Get rid of this bucket
	     */
	    remove_bucket( ht, bkt );
	}
	/* 
	 * Move on to next slot.
	 */
    }

    /*
     * Get rid of the table, and control data
     */
    STDFREE( ht->ht_table );
    STDFREE( ht );
}

static HashBucket*
new_bucket(DSKEY key,void *data)
{
    HashBucket * bucket;

    STDMALLOC(bucket,sizeof(HashBucket),NULL);

    bucket->key  = key;
    bucket->data = data;
    bucket->idx	 = -1;

    return bucket;
}
static void 
free_bucket(HashBucket * bucket)
{
    STDFREE((void*)bucket);
}
int
htAdd(HASHTABLE table,DSKEY key,void *data)
{
    HashBucket	*bucket;
    HashBucket **p,*tmp;
    int 	idx = -1;

    if (find_bucket(table,key,&idx))
	return -1;

    bucket = new_bucket(key,data);
    if (!bucket)
	{ XLOG(bucket); return -1; }
    bucket->idx = idx;

    p = &(table->ht_table)[idx];

    if (*p)
    {
	table->ht_conflicts++;
	DBG(printf("key: %s -> %ul\n",(char*)key,table->ht_hashfun(key)));
    }
    tmp = *p;
    bucket->prev = p;
    *p  = bucket;
    bucket->next = tmp;

    if (tmp)
	tmp->prev = &bucket->next;

    table->ht_items++;

    return 0;
}
static HashBucket*
find_bucket(HASHTABLE table,DSKEY key,int *idx)
{
    HashBucket *p;
    int	i;

    i = table->ht_hashfun(key) % table->ht_size;

    p = table->ht_table[i];

    while (p && table->ht_cmpfun(key,p->key))
	p = p->next;

    if (idx)
	*idx = i;

    return p;
}
static void
remove_bucket(HASHTABLE table,HashBucket *bucket)
{
    int is_conflict = 0;

    if (!bucket)
	{ XLOG(bucket); return; }

    table->ht_items--;

    is_conflict = *bucket->prev != table->ht_table[bucket->idx] || 
    		   bucket->next;

    if ( (*bucket->prev = bucket->next) )
	bucket->next->prev = bucket->prev;

    table->ht_conflicts -= is_conflict;
 
    free_bucket(bucket);
    return;
}

void *
htFind(HASHTABLE table,DSKEY key)
{
    HashBucket * p = find_bucket(table,key,NULL);

    return p ? p->data : NULL;
}

void*
htRemove(HASHTABLE table,DSKEY key)
{
    HashBucket * p = find_bucket(table,key,NULL);
    void * data	= NULL;

    if (!p)
	return NULL;

    data = p->data;
    remove_bucket(table,p);
    return data;
}

static unsigned
hash1(unsigned char *n)
{
    register unsigned h  = 0;
    register unsigned char * p    = n;

    while (*p)
        h = (h << 5) - h + *p++;

    return h;
}

/*
 * A default hash function.
 *
 * This one comes from 'Compiler Design in C', by Allen I. Holub,
 * PRENTICE HALL, ISBN 0-13-155045-4
 */
# define BITS_IN_UNSIGNED	32
# define SEVENTY_FIVE_PERCENT	((int)(BITS_IN_UNSIGNED * .75 ))
# define TWELVE_PERCENT		((int)(BITS_IN_UNSIGNED * .125))
# define HIGH_BITS		(~((unsigned)(~0) >> TWELVE_PERCENT))

static unsigned
hash3(unsigned char * n)
{
    unsigned 	key = 0;
    unsigned	g;

    for (; *n; ++n)
    {
	key = (key << TWELVE_PERCENT) + *n;
	if ((g = key & HIGH_BITS))
	    key = (key ^ (g >> SEVENTY_FIVE_PERCENT)) & ~HIGH_BITS;
    }

    return key;
}
/** { **
static unsigned
hash2(unsigned char *n)
{
    unsigned h  = 0;
    unsigned char * p    = n;

    while (*p)
	h = h*613 + *p++;
    return h;
}
unsigned int
hash4(unsigned char * s)
{
    int                 l = strlen(s);
    int                 x = l;
    int                 i = 0;
    char *              p = s;
    char *              b = s;
    unsigned int        r = 1;

    i = l * (l++);
 
    while (*b)
    {
        r += (r << 5 ) + (*b++)*x + x % l;
        x += (l++) % (i);
    }

    l = strlen(s);
    r += *p * l;
    r -= *(p + l -1 ) * l;

    l %= 31;
    r += *p++ * (l*l--);
    
    while (*p)
    {
        r += *p++ * (l*l--);
    }

    return r;
}
** } **/
static int
hashcmp1(unsigned char *a,unsigned char *b)
{
    for ( ;*a && *b && *a == *b; a++,b++)
    	;
    return *a-*b;
}
int
htSize(HASHTABLE table)
{
    return table->ht_size;
}
int
htItems(HASHTABLE table)
{
    return table->ht_items;
}
int
htEmpty(HASHTABLE table)
{
    return table->ht_items == 0;
}
int
htConflicts(HASHTABLE table)
{
    return table->ht_conflicts;
}
void
htWalk(HASHTABLE table,int all,void (*fun)(DSKEY,void*,int))
{
    unsigned i;

    if (!fun)
	return;

    for (i = 0; i < table->ht_size; i++)
    {
	HashBucket * bucket;
	int is_conflict = 0;

	bucket = table->ht_table[i];

	if (!bucket)
	    continue;
	if (!bucket->next && !all)
	    continue;

	is_conflict = *bucket->prev != table->ht_table[bucket->idx];

	fun(bucket->key,bucket->data,is_conflict);

	while ((bucket = bucket->next))
	    fun(bucket->key,bucket->data,1);
    }
}
