/* typemod.h
 *
 *	(C) Copyright Dec 14 1995, Edmond J. Breen.
 *		   ALL RIGHTS RESERVED.
 * This code may be copied for personal, non-profit use only.
 *
 */


#ifndef TYPEMODH_
#define TYPEMODH_


            /*CUT typeExprStruct*/
typedef struct type_expr {
    unsigned short obj;
    unsigned char alias;
    unsigned char base;
    unsigned char qual;
    union {
	void * inf;
	size_t sz;
    }u;
    struct type_expr * nxt;
} type_expr;
            /*END CUT*/

/* macro methods */
#define setBase(X)  do {type_expr * x = X; if(x) x->base=1;} while 0
#define nextType(x) (x)->nxt
#define setInf(x,y) (x)->u.inf = (y)
#define getNumElems(x)  (x)->u.sz
#define setNumElems(x,y)  (x)->u.sz = (y)

/* handle qualifers */
#define isconst(x)          ((x)->qual  & q_const)
#define isconstp(x)         ((x)->qual  & q_constp)
#define setConst(x)         ((x)->qual  |= q_const)
#define setConstp(x)     ((x)->qual  |= q_constp)
#define unsetConst(x)       ((x)->qual &= ~(q_const | q_constp))
#define IsTemp(x)      ((x)->qual & q_temp)
#define SetTemp(x)     ((x)->qual |= q_temp)

/* test 4 lose of const qualifier */ 
#define ConstIntegrity(x,y) \
(!((x)->qual & (q_const | q_constp)) && ((y)->qual & (q_const | q_constp)))

void * getInf(type_expr *t);

void setAsBaseType(type_expr * t);
type_expr *copyBaseType(type_expr * t);


type_expr * getcontype(type_expr * t1, type_expr * t2);
type_expr * catTypes(type_expr *t1, type_expr *t2);
int get_sizeof(type_expr * t);
int get_align(type_expr * t);
void exchtype(int obj, type_expr * t);
type_expr * succType(type_expr *t);
type_expr * revtype(type_expr *t);
void * freetype(type_expr *t);
void setaliases(type_expr *t, char a);
void set_bastype(unsigned t, type_expr * typ);
char basaliase(type_expr *t);
void * basinf(type_expr *t);
int bastype(type_expr *t);

int sametypes(type_expr *t1, type_expr *t2);
type_expr * addtype(unsigned obj,
			type_expr *t);
type_expr * transtype(type_expr *t);
type_expr * copytype(type_expr *t);
int gettype(type_expr *t);
int compareConst(type_expr *t1, type_expr *t2);
int compatibletypes(type_expr *t1, type_expr * t2);


#endif










