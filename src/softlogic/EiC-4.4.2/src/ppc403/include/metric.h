/* metric.h
 *
 *	(C) Copyright Jun  3 1995, Edmond J. Breen.
 *		   ALL RIGHTS RESERVED.
 * This code may be copied for personal, non-profit use only.
 *
 */

enum {m_char, m_short, m_int, m_long, m_float, m_double, m_pointer, m_struct};
/*  intel 386,486,586 */
#if defined(i386) || defined(__i386) || defined(__i386__)
#define EIC_LITTLE_ENDIAN 1
short metric[8][2] = { /* size and alignment size */
       { 1, 1,},  /* char */
       { 2, 2,},  /* short */
       { 4, 4,},  /* int */
       { 4, 4,},  /* long */
       { 4, 4,},  /* float */
       { 8, 4,},  /* double */
       { 4, 4,},  /* pointer */
       { 0, 4,},   /* struct*/
};
#endif

/* sparc */
#if defined(sparc) || defined(__sparc) || defined(__sparc__)
#define EIC_LITTLE_ENDIAN 0
short metric[8][2] = {
       { 1, 1,},  /* char */
       { 2, 2,},  /* short */
       { 4, 4,},  /* int */
       { 4, 4,},  /* long */
       { 4, 4,},  /* float */
       { 8, 8,},  /* double */
       { 4, 4,},  /* pointer */
       { 0, 1,},  /* struct */
};
#endif

/* powerpc */
#ifdef POWERPC
#define EIC_LITTLE_ENDIAN 0
short metric[8][2] = {
       { 1, 1,},  /* char */
       { 2, 2,},  /* short */
       { 4, 4,},  /* int */
       { 4, 4,},  /* long */
       { 4, 4,},  /* float */
       { 8, 8,},  /* double */
       { 4, 4,},  /* pointer */
       { 0, 1,},  /* struct */
};
#endif


/* Dec Alpha */
#ifdef _OSF1
#define EIC_LITTLE_ENDIAN  1
short metric[8][2] = { /* size, alignment pairs */
	{1, 1}, /* char */
	{2, 2}, /* short */
	{4, 4}, /* int */
	{8, 8}, /* long */
	{4, 4}, /* float */
	{8, 8}, /* double */
	{8, 8}, /* pointer */
	{0, 1}, /* struct*/
};
#endif


/* Silicon Graphics */
#ifdef _IRIX
#define EIC_LITTLE_ENDIAN 0
short metric[8][2] = { /* size, alignment pairs */
       {1, 1}, /* char */
       {2, 2}, /* short */
       {4, 4}, /* int */
       {4, 4}, /* long */
       {4, 4}, /* float */
       {8, 8}, /* double */
       {4, 4}, /* pointer*/  
       {0, 1}, /* struct*/
};
#endif
