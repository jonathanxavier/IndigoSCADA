#define INCLUDE_INT 0 /*(sizeof(long) != sizeof(int))*/

#define CASE_INT	case t_char: case t_short:\
				case t_int: case t_enum
#define CASE_UINT	case t_uchar: case t_ushort: case t_uint
#define CASE_LONG   case t_long
#define CASE_ULONG  case t_ulong
#define CASE_FLOAT  case t_float: case t_double
#define CASE_NUM    CASE_INT:\
				CASE_UINT:\
				CASE_LONG:\
				CASE_ULONG:\
				CASE_FLOAT

/* typesets.c
--------------*/
void do_stooutput(token_t * e1);
void do_inc_dec(token_t *e1, int op);
void output(token_t *e1);
int do_binaryop(token_t *e1, token_t *e2,int op);
int unaryop(token_t *e1,int op);
void cast2comm(token_t *e1, token_t *e2);
int bin_validate(unsigned oper, token_t *e1, token_t *e2);
void castconst(token_t *e1, token_t *e2,int explicit);
void castvar(token_t *e1, token_t *e2, int explicit);

