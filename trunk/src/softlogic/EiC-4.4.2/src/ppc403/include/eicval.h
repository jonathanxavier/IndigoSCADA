#ifndef EICVALH_
#define EICVALH_

#if !defined(_eic_ptr)
#define _eic_ptr
typedef struct {void *p, *sp, *ep;} ptr_t;
#endif

union VaL {
    char cval;   	    /* char value */
    unsigned char ucval;
    short sval;             /* short integer val */
    unsigned short usval;
    int ival;    	    /* integer value */
    unsigned  uival;
    long lval;              /* long integer */
    unsigned long ulval;
    float fval;  	    /* float value */
    double dval;            /* double float value */

    ptr_t p;

    div_t divval;
    ldiv_t ldivval;
        
    int (*func)(); 	     /* function pointer */
    union VaL (*vfunc)();

    struct symentry_t * sym;/* pointer into symbol table */

    pid_t pid;              /* process ID value */
    size_t szval;           /* generic size value */
    ssize_t sszval;         /* POSIX.1 byte count value */
    mode_t mval;            /* mode_t value */
    off_t offval;           /* file offset position */
};
typedef union VaL val_t;


#ifndef EICH_
typedef struct AR_t {
    val_t v;
    type_expr * type;
}AR_t;

#else
typedef struct AR_t {
    val_t v;
    void * type;
}AR_t;
#endif

#endif
