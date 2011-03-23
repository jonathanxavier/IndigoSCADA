#include "stdlib.h"
#include "ctype.h"
#include "limits.h"

#include "string.h"
#include "math.h"
#include "float.h"
#include "unistd.h"
#include "fcntl.h"


#include "eic.h"


#include "xalloc.h"
#include "assert.h"
#include "signal.h"

#include "sys/types.h"
#include "unistd.h"

#define getptrarg(x,y)  y = arg(x,getargs(),ptr_t)

#define _READ    0x1    /* file open for reading */
#define _WRITE   0x2    /* file open for writing */
#define _UNBUF   0x4    /* file is unbuffered */
#define _LNBUF   0x8    /* file is line buffered  */
#define _FLBUF   0x10   /* file is fully buffered */
#define _UPDATE  0x20   /* file opened for updating */
#define _EOF     0x40   /* end of file has occured */
#define _ERR     0x80   /* error has occured */

#define _RDP    0x100  /* read has occured since last file-positioning */
#define _WRP    0x200  /* write has occured since last file-positioning */
#define _EXTBUF 0x400  /* external buffer assigned to file */

#define _UNFIL  0x800  /* file buffer not freed on close */
#define EOF     (-1)    /* end of file indicator */


char *ppcgets(char *str);
void puts(char *str);
void outbyte(int ch);
int inbyte(void);
int getchq(void);
void byteout(unsigned char val, unsigned long addr);
int bytein(unsigned long addr);


extern char *startstr;
extern int _stsptr,persist;

val_t eic_gets(void)
{
    val_t v;
    getptrarg(0,v.p);
    v.p.p = ppcgets(v.p.p);
    return v;
}     

val_t eic_puts(void)
{
    val_t v;
    arg_list ap = getargs();
    puts(arg(0,ap,ptr_t).p);
    return(v); 
}

val_t eic_putch(void)
{
    val_t v;
    arg_list ap = getargs();
    outbyte(arg(0,ap,int));
    v.ival=0;
    return v;
}
val_t eic_getch(void)
{
    val_t v;
    v.ival = inbyte();
    return v;
}

val_t eic_getchq(void)
{
    val_t v;
    v.ival = getchq();
    return v;
}

val_t eic_out(void)
{
    val_t v;
    arg_list ap = getargs();
    byteout(arg(0,ap,int),arg(1,ap,int));
    return v;
}

val_t eic_in(void)
{
    val_t v;
    v.ival = bytein(nextarg(getargs(),int));
    return v;
}

val_t eic_load(void)
{
    val_t v;
    getptrarg(0,v.p);
    _stsptr=0;
    persist=0;
    startstr=v.p.p;
    return v;
}


int charin_(char **buf)
{
    int ch;
    ch = **buf;
    *buf += 1;
    if(!ch)
        ch = EOF;
    return ch;
}

int charback_(int ch, char **buf)
{
    *buf -= 1;
    return ch;
}

int charout_(int c, char **buf)
{
    char *s;
    s = *buf;
    *s++ = c;
    *s = 0;
    *buf = s;
    return 1;
}

