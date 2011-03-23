#ifndef PREPROC_H_
#define PREPROC_H_

/* cmode states */
#define CMcmt 1			/* in comment */
#define CMstr 2			/* in string */
#define CMchr 3			/* in character constant */
#define CMang 4			/* in < > */

#define INC   80
#define skipall(a)      while(*(a)) ++a;
#define skipfws(a)	while(*(a) && isspace(*(a))) ++a;
#define skipbws(a)	while(*(a) && isspace(*(a))) --a;
#define skip2char(a,b)  while(*(a) && *(a) != b) ++a;
#define skipnws(a)      while(*(a) && !isspace(*(a))) ++a;



extern char cmode;
char * process2(char * line,int bot,int top);
int    cpp_parse(char *s);
void pre_error(char *msg, ...);
int ismacroid(char *id);
int showMacro(char *id);

typedef struct fitem_t{
    int fd;                  /* file descriptor */
    char * fname;            /* file name */
    unsigned int lineno;     /* file lineno */
    unsigned char * buf;              /* file buffer */
    unsigned char * bufp;             /* file buffer pointer  */
    int n;                   /* number of chars left in buffer */
    struct fitem_t * next;
} fitem_t;

extern fitem_t *Infile;

#define CurrentFileName()   Infile->fname
#define CurrentLineNo()     Infile->lineno


/* preproc.c
-------------*/
int initpp(void);
char * nextproline(void);



#endif




