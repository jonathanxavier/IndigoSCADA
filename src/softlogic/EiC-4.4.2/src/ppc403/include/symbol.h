/* symbol.h
 *
 *	(C) Copyright May  7 1995, Edmond J. Breen.
 *		   ALL RIGHTS RESERVED.
 * This code may be copied for personal, non-profit use only.
 *
 */

#ifndef HSIZE
#define HSIZE 101
#endif
extern symentry_t *HTAB[];

int iskeyword(keyword_t *keywords,char*id,int n);
void eicpush(eicstack_t *s, val_t v);
int eicpop(eicstack_t *s, val_t *pop);
int init_hashtab(int size);
int hashsmc(char * s,int mod);
symentry_t * lookup(char nspace,char *id);
void free_sym(symentry_t *sym);
void remlevel(int level);
void lut_CleanUp(size_t bot);
void addoffsettolevel(char nspace,int level,int off);
void marktype(type_expr *type, char mark);
void markcode(symentry_t *sym,char mark);
void marksyms(char mark);
void remsym(symentry_t *sym);
symentry_t * insertLUT(char nspace,char *id,int type);
char * strsave(char *s);
void newsymtype(symentry_t *sym, type_expr *t);
void newsymARval(token_t *e1, val_t *v);
int nextstackitem(int level);
int stackit(symentry_t * sym,int level);
void inittoken(token_t * e1);
void freetoken(token_t * e1);

void initcode(code_t * code);
void killcode(symentry_t *sym);
void freecode(code_t * code);
void generate(code_t * code, int opcode,val_t *val,int ext);
void copycode(code_t * c1, code_t * c2);
void concode(code_t * c1, code_t * c2);
void contoken(token_t * e1, token_t * e2);
void swaptokens(token_t *e1, token_t * e2);

#define setCodeLineNo(C,idx,No); ((C)->inst[idx].line = No)

size_t lut_NextEntryNum(void);






