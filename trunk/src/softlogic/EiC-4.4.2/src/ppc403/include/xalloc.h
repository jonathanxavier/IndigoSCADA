/* xalloc.h
 *
 *	(C) Copyright Dec 12 1995, Edmond J. Breen.
 *		   ALL RIGHTS RESERVED.
 * This code may be copied for personal, non-profit use only.
 *
 */
#ifndef XALLOCH
#define XALLOCH



#define ymem
#ifdef ymem


#define xmalloc(N)          ymalloc(__FILE__,__LINE__,N)
#define xcalloc(N,M)	    ycalloc(__FILE__,__LINE__,N,M)
#define xrealloc(N,M)	    yrealloc(__FILE__,__LINE__,N,M)
#define xfree(N)	    yfree(__FILE__,__LINE__,N)
#define xmark(N,M)          ymark(__FILE__,__LINE__,N,M)
#define xmemcheck()         ymemcheck(__FILE__,__LINE__)
#define xdumpnonmark(N,M)   ydumpnonmark(N,M)

void  xfreemark(char mark);
int   xexchmark(char oldmark, char newmark);
int   xlookup(void *p);

int getMemMark(unsigned long item);
void freeMemItem(unsigned long item);

void   xalloc_CleanUp(size_t bot, size_t top);
size_t xalloc_NextEntryNum(void);

void *ymalloc(char *file,int lineno, size_t nbytes);
void *ycalloc(char *file,int lineno,size_t nelem,size_t elemsize);
void *yrealloc(char *file,int lineno,void *block, size_t nbytes);
void yfree(char *file,int lineno,void *block);
int  ymark(char *file,int lineno,void *block,char mark);
void ydumpnonmark(char *outfile,char mark);


extern unsigned long tot_memory;   /* total amount of memory allocated */
extern size_t tot_alloc;  	   /* total number of blocks allocated */
extern size_t tot_seen; 	   /* total number of blocks seen */
extern char XGMARK;		   /* memory mark, default = 0 */

#define MEM_LEAK   66
#define NON_LEAK   50

#define catchItem(X)  {size_t x = X; if(tot_seen == x)\
			   printf("\n\tCAUGHT ITEM %lu at %d in %s\n\n",\
				  (unsigned long)x, __LINE__,__FILE__);}

#else
#define xmalloc(N)              malloc(N)
#define xcalloc(N,M)            calloc(N,M)
#define xrealloc(N,M)           realloc(N,M)
#define xfree(N)                free(N)

#endif
#endif








