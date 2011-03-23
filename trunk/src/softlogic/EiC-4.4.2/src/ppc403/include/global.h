/* global.h
 *
 *	(C) Copyright May  7 1995, Edmond J. Breen.
 *		   ALL RIGHTS RESERVED.
 * This code may be copied for personal, non-profit use only.
 *
 */
#ifndef GLOBALH
#define GLOBALH

#define  DONE   0
#define BSIZE 128
#define NONE -1
#define EOS  '\0'
#define TRUE    1
#define FALSE   0

typedef enum {
    t_error,   t_bool,    t_char, t_uchar,   t_short, t_ushort,  /*5*/
    t_int,     t_enum,    t_uint, t_long,    t_ulong, t_float,   /*11*/
    t_double,  t_pointer, t_void, t_struct,  t_union,            /*16*/
    t_lval,    t_array,   t_func, t_funcdec, t_elem,  t_eic,     /*22*/
    t_builtin, t_var,     t_hidden
} obj_t;

/* unsafe macros */
#define isArithmetic(t) (t >= t_char && t <= t_double)
#define isIntegral(t)   (t >= t_char && t <= t_ulong)

typedef struct {
    char *id;
    int  token;
} keyword_t;

/* qualifiers */
enum {q_notset = 0,
	  q_const=2,
	  q_constp = 4,
	  q_volatile = 8,
	  q_temp = 16};

/* storage class */
enum {c_auto=1,c_static=2,c_register=4,c_typedef=8,
	  c_extern=16, c_base=32, c_private=64};

typedef enum { /* keyword symbols */
    autosym = 300,
    breaksym, casesym, charsym, constsym, continuesym,
    defaultsym, dosym, doublesym, elsesym, enumsym, externsym,
    floatsym, forsym, idsym, ifsym, intsym, longsym,
    registersym, returnsym, shortsym,  signedsym, sizeofsym,
    staticsym, structsym, switchsym, typedefsym, unionsym,
    unsignedsym, voidsym, volatilesym, whilesym
} keysyms_t;


#include "typemod.h"
#include "datastruct.h"


extern environ_t * ENV;

                   /*CUT nameSpaceCodes*/
extern int work_tab;
enum{ /* name space codes */
	eic_tab,   /* name space for EiC commands */
	stand_tab, /* name space for basic variables */
	tag_tab   /* name space for struct/union and enumeration tags */
    };
                   /*END CUT*/ 

enum {eickmark, eicgstring, eicstay};
int GI(token_t * e1);

/* error handling  globals
---------------------------*/
extern int ParseError;          /* if 1 an error has occured */
extern int ErrorRecover;	/* if 1 attempting to recover from error*/
extern int EiC_errs;		/* Number of errors and warnings */
#endif
