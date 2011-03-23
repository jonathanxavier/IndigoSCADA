/* eic.h
 *
 *	(C) Copyright May  7 1995, Edmond J. Breen.
 *		   ALL RIGHTS RESERVED.
 * This code may be copied for personal, non-profit use only.
 *
 */
#ifndef EICH_
#define EICH_

#define _need_eic_ptr

#include "sys/types.h"

#undef _need_eic_ptr


#include "eicval.h"


void *add_builtinfunc(char *name, val_t(*vfunc) ());
void startEiC(int argc, char ** argv);
void init_EiC(void);

extern AR_t * AR[3];
extern size_t ARGC;

#define arg_list                  AR_t *
#define getargc()                 ARGC
#define getargs()                 AR[2]
#define nextarg(x,type)           (*((type*)&(--x)->v))
#define arg(i,x,type)             (*((type*)&x[-(i+1)].v))

#define malloc(x)	xmalloc(x)
#define calloc(x,y)	xcalloc(x,y)
#define free(x)		xfree(x)
#define realloc(x,y)    xrealloc(x,y)


#endif /* EICH_ */














