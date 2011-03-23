
/* datastruct.h
 *
 *	(C) Copyright May  7 1995, Edmond J. Breen.
 *		   ALL RIGHTS RESERVED.
 * This code may be copied for personal, non-profit use only.
 *
 */

#define _need_eic_ptr

#include "sys/types.h"

#undef _need_eic_ptr

#include "stab.h"

#include "eicval.h"


typedef struct {
    int n;
    val_t * val;
}eicstack_t;

typedef struct {
    unsigned opcode;
    val_t  val;
    int ext;
    unsigned short line;
}inst_t;


typedef struct {
    unsigned int nextinst;  /* next instruction */
    unsigned int binst;     /* physical size */
    inst_t * inst;          /* instructions */
    char * Filename;        /* file with source code */
}code_t;

/* methods for code */
#define opcode(C,i)   ((C)->inst[i].opcode)
#define setopcode(C,i,y) (opcode(C,i) = y)
#define ivalcode(C,i) ((C)->inst[i].val.ival)
#define pvalcode(C,i) ((C)->inst[i].val.p.p)
#define nextinst(C)   ((C)->nextinst)
#define instline(C,i) ((C)->inst[i].line)
#define codeName(C)   (C)->Filename

typedef struct extern_t {
    char * name;
    type_expr * type;
    int  n;
    unsigned *loc;
    struct extern_t *nxt;
} extern_t;

/* methods for extern_t */
#define crt_extern()    xcalloc(sizeof(extern_t),1)
#define getExtName(x)   ((x)->name)
#define setExtName(x,y) ((x)->name = y)
#define getExtType(x)   ((x)->type)
#define setExtType(x,y) ((x)->type = y) 
#define getExtNext(x)   ((x)->nxt)
#define setExtNext(x,y) ((x)->nxt = y)


typedef struct {
    char cl;           /* closed flag */
    int n;             /* number of members */
    int tsize;         /* total size in bytes of struct */
    int align;         /* alignment of structure */
    type_expr **type;  /* member types */
    char **id;         /* member names */
    int *offset;       /* offsets to members data position*/
    int ntags;         /* number of tags */
    type_expr **tag;   /* tag types */
}struct_t;


typedef struct {
    unsigned short tok;
    int tab;		     /* name space table */
    char pflag;              /* processed flag */
    char sclass;             /* storage class */
    char typequal;           /* type qualifier */
    struct symentry_t * sym;
    code_t code;
    type_expr * type;
    val_t val;
}token_t;


/*CUT symEntry*/
typedef struct symentry_t  {
    unsigned int entry;        /* entry number */
    struct symentry_t *next;   /* link to next symentry */
    char *id;                  /* pointer to identifier string */
    char sclass;               /* storage class code */
    char typequal;             /* type qualifier */
    char level;                /* scope level */
    char nspace;               /* name space identifier */
    char ass;                  /* used to flag assignment */
    type_expr * type;          /* object type */
    val_t  val;                /* symbol value information */
    char *pname;               /* previous file name */
    char *fname;               /* file name pointer */
}symentry_t;
/*END CUT*/

typedef struct {
    stab_t stab;
    
    int level;
    code_t CODE;
    eicstack_t ARgar;        /* for garbage collection of AR units*/
    unsigned int ARsize,sp;
    AR_t *AR;   		 /* static activation record */
    unsigned int LARsize,lsp;
    AR_t *LAR;
    extern_t *link;
    
}environ_t;

#define getenvcode(env)  ((env)->CODE)

typedef struct {
    int n;                  /* number of enumerators */
    int *eval;              /* array of enumerator values */
    symentry_t **syms;      /* list of symbol tabel entries */
}enum_t;

typedef struct {
    int hsize;
    symentry_t **htab;
}hashtab_t;










