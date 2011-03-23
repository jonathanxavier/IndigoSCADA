#ifndef FUNCH_
#define FUNCH_

#include "typemod.h"

typedef struct {
    type_expr *type;   /* parameter type */
    char *name;        /* parameter name */
    char *pcomms;      /* parameter comment */
    void *def;         /* parameter default value */
} parm_t;



typedef struct {
    int Num;             /* # of parameters */
    parm_t * parm;


    type_expr **typ;  /* parameter types */
    char **nmes;      /* parameter names */
    char **pcomms;     /* parameter comments */

    
    int sn;            /* # of strings */
    char **strs;       /* string pointers */
    int stn;           /* # of static arrays */
    int *st;           /* pointer to static arrays */
    extern_t *link;    /* pointer to links */
    int ni;            /* # of initialisers */
    void **inzs;       /* initialisers */
    char *comm;        /* function comment */
} func_t;

/** func.c **/
/* methods */
#define getFNp(f)         ((f)->Num)     /* get number of parameters */
#define setFNp(f,i)       ((f)->Num = i)
#define getFPname(f,i)    ((f)->nmes[i])
#define getFPnames(f)     ((f)->nmes)
#define setFPnames(f,s)    ((f)->nmes = s)
#define setFPname(f,i,s)  ((f)->nmes[i] = s)
#define getFPty(f,i)      ((f)->typ[i])
#define getFPtyList(f)    ((f)->typ)
#define setFPtyList(f,s)  ((f)->typ = s)
#define setFPty(f,i,s)    ((f)->typ[i] = s) 
#define getFPcomm(f,i)    ((f)->pcomms[i])
#define getFPcomms(f)     ((f)->pcomms)
#define setFPcomms(f,s)   ((f)->pcomms = s)
#define setFPcomm(f,i,s) ((f)->pcomms[i] = s)


#define getFComm(f)      ((f)->comm)
#define setFComm(f,s)    ((f)->comm=s)

void make_func(token_t * e1);
void add_func_parm(func_t * f, type_expr ** type, char * name);
void add_func_str(func_t * f, char *s);
void add_func_static(func_t * f, int n);
void add_func_initialiser(func_t * f, void *s);
void free_func_inf(func_t * f);
void markFunc(type_expr * type, int mark);
void freeFuncComments(func_t *f);
int sameFuncs(type_expr *t1, type_expr *t2);

int IsVariadic(func_t *f);
int hasPrototype(func_t *f);
void swapFPLists(func_t *f1, func_t *f2);
int gotMissingNames(func_t *f);
 
int HasHiddenParm(type_expr *ty);
#endif









