#ifndef PARSERH_
#define PARSERH_

/* parser.c
------------*/
void initparser(void);
void parse(environ_t * env);
void stmt(token_t * e1);
void assign_expr(token_t * e1);
void cond_expr(token_t * e1);

#endif
