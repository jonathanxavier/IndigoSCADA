#ifndef LEXERH_
#define LEXERH_

/* lexer.h
 *
 *	(C) Copyright May  7 1995, Edmond J. Breen.
 *		   ALL RIGHTS RESERVED.
 * This code may be copied for personal, non-profit use only.
 *
 */
enum{
  NUM = 350, HEX, OCTAL, STR,
  CHAR,UCHAR,
  SHORT, USHORT,
  INT,UINT,
  LONG, ULONG,
  FLOAT,DOUBLE,

  ID,TYPENAME, FUNCTION,
  INC,		/* ++ */
  DEC,		/* -- */
  RARROW,	/* -> */
  LSHT,		/* << */
  RSHT,		/* >> */

  MISC, /* dummy for lexan */

  RELOP,  /* relational operators */
  LT,	/* < */
  LE,	/* <= */
  EQ,	/* == */
  NE,	/* != */
  GT,	/* >  */
  GE,	/* >= */

  LOR,	/* || */
  BOR, 	/* | */
  XOR,	/* ^ */

  LAND,	/* && */
  AND,	/* & */

  LOGOP,	/* logical operators */
  NOT,		/* ! */

  ASSOP,	/* assignment operators */
  ASS,		/* =  */
  ADDEQ,	/* += */
  SUBEQ,	/* -= */
  MULEQ,	/* *= */
  DIVEQ,	/* /= */
  MODEQ,	/* %= */
  RSHTEQ,	/* >>= */
  LSHTEQ,	/* <<= */
  ANDEQ,	/* &= */
  BOREQ,	/* |= */
  XOREQ	/* ^= */

};

void initlex(char *str);
int lexan(void);

#if 1

extern unsigned short STOKEN;
#define retractlexan()  STOKEN=token.tok

#else
#define ILOOKAHEAD

extern token_t TokenArray[];
extern int TokenI;

#define retractlexan()   do\
{\
     TokenArray[TokenI++] = token;\
     if(TokenI > 1) {\
       fprintf(stderr,"TokenI = %d line %d in %s\n",\
               TokenI, __LINE__,__FILE__);\
       exit(0);\
     }\
}while(0)

#endif

#endif  /* LEXERH_ */
