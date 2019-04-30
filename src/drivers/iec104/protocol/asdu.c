/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
//#include <assert.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef WIN32
#include <winsock2.h>
#endif
#ifndef WIN32
#include <sys/time.h>
#include <sys/param.h>
#include <arpa/inet.h>
#include <asm/byteorder.h>
#include <glib-2.0/glib.h>
#endif
#include "event.h"
#include "compat/sys/queue.h"
#include "fifoc.h"
#include "clear_crc_eight.h"
#include "iec104.h"
#include "iec_library.h"
#include "iec104_types.h"
#include "iec_item.h"
#include "itrace.h"
#ifdef WIN32
#include <io.h>
#endif
#ifdef __unix__
#include <sys/io.h>
#include <dirent.h>
#include <portable.h>
#endif

//Control direction: SCADA-------------> RTU
//Monitor direction: SCADA<------------- RTU


/////globals//////////////////////////////////////////////////////////////////////////////////
u_short common_address_of_asdu = 0;
u_short select_before_operate = 0;
char iec104ServerAddress[80];
char iec104ServerPort[80];
char err_msg[100];
FILE *iec_protocol_log_stream = NULL;
char iec_optional_log_file_name[80] = {0};
int log_all_slave_messages = 1; //<-- metterla nel file .ini per disabilitarla se le fprintf creano troppo carico alla CPU

//Interface to field//////////////////////////////////////////////////////////////////////////
//Multithread shared fifo
static fifo_h fifo_control_direction = NULL; //fifo in control direction: SCADA-------------> RTU DA server if s->type == IEC_SLAVE
//static fifo_h fifo_control_direction_ae; //fifo in control direction: SCADA-------------> RTU AE server if s->type == IEC_SLAVE
static fifo_h fifo_monitor_direction = NULL; //fifo in monitor direction: SCADA<------------- RTU

//////////////////State machines///////////////////////////////////////////
static int state_database = DB_STARTUP; //if we NEED to load the database in the iec 104 slave
static int state_file_transfer = FT_IDLE;
static int state_gateway = GW_STARTUP;
static int state_general_interrogation = GI_IDLE;
static int state_monitoring_direction = MSG_IDLE;
static int state_control_direction = CMD_IDLE;
static int state_clock_synchronisation = CLK_IDLE;
static int state_tcp_link = LNK_IDLE;
int state_of_slave = SLAVE_NOT_INITIALIZED;
static int state_global_logs = LOGS_STARTUP;
///////////////////////////////////////////////////////////////////////////

////////////////////////general interrogation//////////////////////////////
static int point_to_load = 0;
static int points_to_load_in_a_scan = 100; //Numero di punti caricati durante un clock o passata nella macchina a stati
////////////GI timeout/////////////////////
static time_t start_epoch = 0;
static time_t finish_epoch = 0;
static cp56time2a finish;
static cp56time2a start;
static time_t delta;
#define MAX_GI_TIME_IN_SECONDS 60
///////////////////////////////////////////////
//Select Before Operate: we execute the command if it arrives within 60 seconds after Select Before Operate
#define MAX_TIME_AFTER_SBO_IN_SECONDS 60
//#define USE_SELECT_BEFORE_OPERATE //comment this line if you do not want Select Before Operate

////////////////Database//////////////////////////////////////////////////
static time_t db_start_epoch = 0;
static time_t db_finish_epoch = 0;
#define DB_COUNT_DOWN_TIME_IN_SECONDS 20

static struct iec_item* iec_items_table = NULL;
static int num_iec_items = 0;
static char file_configurazione[_MAX_PATH + _MAX_FNAME];
static char punti_opc_pg[_MAX_PATH + _MAX_FNAME];
///////////////////////CRC database//////////////////////////////////////////////////////////////////
//#include "crc_32.h"
static unsigned long CRC_file_configurazione_master = 0;
static unsigned long CRC_file_configurazione_slave  = 0;

///////////////File transfer/////////////////////////////////////////////////////////////////////////
static u_int file_checksum;
static u_int section_checksum;

///////////////////////////////////File transfer variables///////////////////////////////////////////
//Per evitare la frammentazione usare la #define SOCK_NO_DELAY 
static int MAX_SEGMENT_LEN = 224; //bytes or octets
static unsigned int length_of_file = 0;
static struct cp56time2a ft_creat_time;
static int length_of_segment = 0;
static int number_of_segments = 0;
static int resto_dei_byte_da_leggere = 0;
static char file_trasferito[_MAX_PATH + _MAX_FNAME];
static char file_transf_working_dir[_MAX_PATH];

#define CSV_FILE_EXT		"csv" //estensione del file da trasferire
static int index_of_file_to_transfer = 0;
static int num_of_file_loaded = 0;
#define MAX_FILES_TO_LOAD_IN_FILE_TRANSFER_QUEUE 20
static char file_da_trasferire[MAX_FILES_TO_LOAD_IN_FILE_TRANSFER_QUEUE][_MAX_PATH + _MAX_FNAME];
/////////////////////////////////////////////////////////////////////////////////////////////////////
////////////Logs check timeout/////////////////////
static time_t logs_start_epoch = 0;
static time_t logs_finish_epoch = 0;
static cp56time2a logs_finish;
static cp56time2a logs_start;
static time_t logs_delta;
#define TIME_IN_SECONDS_BETWEEN_CHECKS 3600
///////////////////////////////////////////////


#define IECASDU_PARSE_FUNC(name)					\
int name(struct iec_object *obj, u_short *com_addr, int *n,		\
	u_char *cause, u_char *test, u_char *pn, size_t ioa_len,	\
	size_t ca_len, unsigned char *buf, size_t buflen)

#define iecasdu_parse_type(objp, com_addrp, np, causep, testp, pnp, 	\
		ioa_len, ca_len, bufp, buflen, type_name, struct_name) 	\
	int i;						\
	u_int asdu_len;		\
	u_int addr_cur;		\
	u_char *add_ioa_p;	\
	struct iec_unit_id *unitp;					\
	struct type_name *typep;					\
	IT_IT("iecasdu_parse_type");\
	if(ioa_len == 0 || ioa_len > 3 || ca_len == 0 || ca_len > 2) \
	{	\
		sprintf(err_msg, "IOA or CASDU length are wrong");	\
		iec_call_exit_handler(__LINE__,__FILE__, err_msg); \
	}	\
	unitp = (struct iec_unit_id *) bufp;		\
	if(unitp->sq)	\
	{	\
		asdu_len = ca_len + IEC_TYPEID_LEN + (unitp->num * sizeof(struct type_name)) + ioa_len;		\
	}	\
	else	\
	{	\
		asdu_len = ca_len + IEC_TYPEID_LEN + unitp->num * ((sizeof(struct type_name)) + ioa_len);	\
	}	\
	if(unitp->type != F_SG_NA_1)	\
	{	\
		if (asdu_len != buflen)	\
			return (1);	\
	}	\
	*com_addrp = (*unitp).ca;	\
	add_ioa_p = (u_char *) ((u_char *) unitp + ca_len + IEC_TYPEID_LEN);	\
	typep = (struct type_name *) ((u_char *) unitp + ca_len + IEC_TYPEID_LEN + ioa_len);	\
	*np = unitp->num;						\
	*causep = unitp->cause;						\
	*testp = unitp->t;						\
	*pnp = unitp->pn;						\
	if (unitp->sq) {						\
		addr_cur = MASK_IOA_OCTETS&(*((unsigned int*)add_ioa_p));	\
		for (i = 0; i < unitp->num; i++, objp++, typep++, addr_cur++) { \
			objp->ioa = addr_cur;				\
			objp->o.struct_name = *typep;			\
		}							\
	} else {							\
		for (i = 0; i < unitp->num; i++, objp++) {		\
			obj->ioa = MASK_IOA_OCTETS&(*((unsigned int*)add_ioa_p));	\
			objp->o.struct_name = *typep;			\
			add_ioa_p = (u_char *) ((u_char *) typep + sizeof(struct type_name));\
			typep = (struct type_name *) ((u_char *) typep + sizeof(struct type_name) + ioa_len);\
		}							\
	}								


IECASDU_PARSE_FUNC(iecasdu_parse_type1)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type1, type1);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type3)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type3, type3);

	IT_EXIT;
	return 0;
}


IECASDU_PARSE_FUNC(iecasdu_parse_type7)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type7, type7);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type9)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type9, type9);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type11)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type11, type11);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type13)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type13, type13);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type15)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type15, type15);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type37)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type37, type37);

	IT_EXIT;
	return 0;
}

int iecasdu_parse_type30(struct iec_object *obj, u_short *com_addr, int *n,
	u_char *cause, u_char *test, u_char *pn, size_t ioa_len,
	size_t ca_len, unsigned char *buf, size_t buflen)
{

	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type30, type30);

//#define iecasdu_parse_type(objp, com_addrp, np, causep, testp, pnp, 	
//		ioa_len, ca_len, bufp, buflen, type_name, struct_name) 	
/*
	int i;						
	u_int asdu_len;						
	u_int addr_cur;
	u_char *add_ioa_p;					
	struct iec_unit_id *unitp;					
	struct iec_type30 *typep;					
	
	assert((ioa_len && ioa_len <= 3) && (ca_len && ca_len <= 2));	
	unitp = (struct iec_unit_id *) buf;		

	if(unitp->sq)	
	{	
		asdu_len = ca_len + IEC_TYPEID_LEN + (unitp->num * sizeof(struct iec_type30)) + ioa_len;		
	}	
	else	
	{	
		asdu_len = ca_len + IEC_TYPEID_LEN + unitp->num * ((sizeof(struct iec_type30)) + ioa_len);	
	}	

	if(unitp->type != F_SG_NA_1)	
	{	
		if (asdu_len != buflen)	
			return (1);	
	}

	*com_addr = (*unitp).ca;	
	add_ioa_p = (u_char *) ((u_char *) unitp + ca_len + IEC_TYPEID_LEN);
	typep = (struct iec_type30 *) ((u_char *) unitp + ca_len + IEC_TYPEID_LEN + ioa_len);	
	*n = unitp->num;						
	*cause = unitp->cause;						
	*test = unitp->t;						
	*pn = unitp->pn;						

	if (unitp->sq)
	{						
		addr_cur = MASK_IOA_OCTETS&(*((unsigned int*)add_ioa_p));

		for (i = 0; i < unitp->num; i++, obj++, typep++, addr_cur++)
		{ 
			obj->ioa = addr_cur;				
			obj->o.type30 = *typep;			
		}							
	} 
	else 
	{
		for (i = 0; i < unitp->num; i++, obj++)
		{		
			obj->ioa = MASK_IOA_OCTETS&(*((unsigned int*)add_ioa_p));

			obj->o.type30 = *typep;			
			add_ioa_p = (u_char *) ((u_char *) typep + sizeof(struct iec_type30));
			typep = (struct iec_type30 *) ((u_char *) typep + sizeof(struct iec_type30) + ioa_len);
		}							
	}								
*/
	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type31)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type31, type31);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type33)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type33, type33);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type34)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type34, type34);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type35)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type35, type35);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type36)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type36, type36);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type45)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type45, type45);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type46)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type46, type46);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type58)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type58, type58);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type59)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type59, type59);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type48)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type48, type48);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type49)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type49, type49);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type50)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type50, type50);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type61)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type61, type61);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type62)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type62, type62);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type63)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type63, type63);

	IT_EXIT;
	return 0;
}


IECASDU_PARSE_FUNC(iecasdu_parse_type51)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type51, type51);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type64)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type64, type64);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type70)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type70, type70);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type100)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type100, type100);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type103)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type103, type103);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type104)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type104, type104);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type107)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type107, type107);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type105)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type105, type105);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type106)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len,
		ca_len, buf, buflen, iec_type106, type106);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type120)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type120, type120);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type121)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type121, type121);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type122)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type122, type122);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type123)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type123, type123);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type124)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type124, type124);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type125)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type125, type125);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type126)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type126, type126);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type150)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type150, type150);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type151)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type151, type151);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type152)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type152, type152);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type153)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type153, type153);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type154)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type154, type154);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type155)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type155, type155);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type156)
{
	iecasdu_parse_type(obj, com_addr, n, cause, test, pn, ioa_len, 
		ca_len, buf, buflen, iec_type156, type156);

	IT_EXIT;
	return 0;
}

/**
 * iecasdu_parse - parse ASDU unit
 * @param obj : array of information objects, MUST be at least IEC_OBJECT_MAX
 * @param type : returned type identifier (1-127)
 * @param com_addr : returned common address of ASDU
 * @param cnt : returned number of information objects in obj array
 * @param cause : returned cause identifier (0-63)
 * @param test : returned test bit (1=test, 0=not test)
 * @param pn : returned P/N bit (0=positive confirm, 1=negative confirm)
 * @param ioa_len : information object address length (1-3)
 * @param ca_len : common address length (1-2)
 * @param buf : buffer which contains unparsed ASDU
 * @param buflen : ASDU length
 * @return : 0 - success, 1 - incorrect ASDU, 2 - unknown ASDU type
 */
int iecasdu_parse(struct iec_object *obj, u_char *type, u_short *com_addr, 
	int *cnt, u_char *cause, u_char *test, u_char *pn, size_t ioa_len, 
	size_t ca_len, u_char *buf, size_t buflen)
{
	int ret = 0;
	struct iec_unit_id *unitp;

	IT_IT("iecasdu_parse");
	
	unitp = (struct iec_unit_id *) buf;

	switch (unitp->type) {
		case M_SP_NA_1:
			*type = M_SP_NA_1;
			ret = iecasdu_parse_type1(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_DP_NA_1:
			*type = M_DP_NA_1;
			ret = iecasdu_parse_type3(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_BO_NA_1:
			*type = M_BO_NA_1;
			ret = iecasdu_parse_type7(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_ME_NA_1:
			*type = M_ME_NA_1;
			ret = iecasdu_parse_type9(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_ME_NB_1:
			*type = M_ME_NB_1;
			ret = iecasdu_parse_type11(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_ME_NC_1:
			*type = M_ME_NC_1;
			ret = iecasdu_parse_type13(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_IT_NA_1:
			*type = M_IT_NA_1;
			ret = iecasdu_parse_type15(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_SP_TB_1:
			*type = M_SP_TB_1;
			ret = iecasdu_parse_type30(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_DP_TB_1:
			*type = M_DP_TB_1;
			ret = iecasdu_parse_type31(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_BO_TB_1:
			*type = M_BO_TB_1;
			ret = iecasdu_parse_type33(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_ME_TD_1:
			*type = M_ME_TD_1;
			ret = iecasdu_parse_type34(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_ME_TE_1:
			*type = M_ME_TE_1;
			ret = iecasdu_parse_type35(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_ME_TF_1:
			*type = M_ME_TF_1;
			ret = iecasdu_parse_type36(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case M_IT_TB_1:
			*type = M_IT_TB_1;
			ret = iecasdu_parse_type37(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		break;
		case C_SC_NA_1://comando singolo
		{
			*type = C_SC_NA_1;
			ret = iecasdu_parse_type45(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_SC_TA_1://comando singolo con time stamp
		{
			*type = C_SC_TA_1;
			ret = iecasdu_parse_type58(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_DC_NA_1://comando doppio
		{
			*type = C_DC_NA_1;
			ret = iecasdu_parse_type46(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_DC_TA_1://comando doppio con time stamp
		{
			*type = C_DC_TA_1;
			ret = iecasdu_parse_type59(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_SE_NA_1://set point command, normalized value
		{
			*type = C_SE_NA_1;
			ret = iecasdu_parse_type48(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_SE_NB_1://set point command, scaled value
		{
			*type = C_SE_NB_1;
			ret = iecasdu_parse_type49(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_SE_NC_1://set point command, short floating point number
		{
			*type = C_SE_NC_1;
			ret = iecasdu_parse_type50(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_SE_TA_1://set point command, normalized value with time
		{
			*type = C_SE_TA_1;
			ret = iecasdu_parse_type61(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_SE_TB_1://set point command, scaled value with time
		{
			*type = C_SE_TB_1;
			ret = iecasdu_parse_type62(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_SE_TC_1://set point command, short floating point number with time
		{
			*type = C_SE_TC_1;
			ret = iecasdu_parse_type63(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_BO_NA_1:
		{
			*type = C_BO_NA_1;
			ret = iecasdu_parse_type51(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_BO_TA_1:
		{
			*type = C_BO_TA_1;
			ret = iecasdu_parse_type64(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case M_EI_NA_1://end of initialization
		{
			*type = M_EI_NA_1;
			ret = iecasdu_parse_type70(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case C_IC_NA_1://general interrogation
		{
			*type = C_IC_NA_1;
			ret = iecasdu_parse_type100(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_CS_NA_1://clock synchronisation
		{
			*type = C_CS_NA_1;
			ret = iecasdu_parse_type103(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}			
		break;
		case C_TS_NA_1: //test command
		{
			*type = C_TS_NA_1;
			ret = iecasdu_parse_type104(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case C_TS_TA_1:  //test command with time stamp
		{
			*type = C_TS_TA_1;
			ret = iecasdu_parse_type107(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case C_RP_NA_1:
		{
			*type = C_RP_NA_1;
			ret = iecasdu_parse_type105(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case C_CD_NA_1:
		{
			*type = C_CD_NA_1;
			ret = iecasdu_parse_type106(obj, com_addr, cnt, cause, test, pn,
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case F_FR_NA_1:
		{
			*type = F_FR_NA_1;
			ret = iecasdu_parse_type120(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case F_SR_NA_1:
		{
			*type = F_SR_NA_1;
			ret = iecasdu_parse_type121(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case F_SC_NA_1:
		{
			*type = F_SC_NA_1;
			ret = iecasdu_parse_type122(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case F_LS_NA_1:
		{
			*type = F_LS_NA_1;
			ret = iecasdu_parse_type123(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case F_AF_NA_1:
		{
			*type = F_AF_NA_1;
			ret = iecasdu_parse_type124(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case F_SG_NA_1:
		{
			*type = F_SG_NA_1;
			ret = iecasdu_parse_type125(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case F_DR_TA_1:
		{
			*type = F_DR_TA_1;
			ret = iecasdu_parse_type126(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case M_ME_TN_1:
		{
			*type = M_ME_TN_1;
			ret = iecasdu_parse_type150(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case M_ME_TO_1:
		{
			*type = M_ME_TO_1;
			ret = iecasdu_parse_type151(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case M_ME_TP_1:
		{
			*type = M_ME_TP_1;
			ret = iecasdu_parse_type152(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case M_ME_TQ_1:
		{
			*type = M_ME_TQ_1;
			ret = iecasdu_parse_type153(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case M_ME_TR_1:
		{
			*type = M_ME_TR_1;
			ret = iecasdu_parse_type154(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case M_ME_TS_1:
		{
			*type = M_ME_TS_1;
			ret = iecasdu_parse_type155(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		case M_ME_TT_1:
		{
			*type = M_ME_TT_1;
			ret = iecasdu_parse_type156(obj, com_addr, cnt, cause, test, pn, 
				ioa_len, ca_len, buf, buflen);
		}
		break;
		default:
			*type = unitp->type;
			ret = 2;
		break;
	}

	IT_EXIT;
	return(ret);
}

#if 0  
/*****************************************************************************
*unsigned _getsystime(timestruc, milliseconds) - Get current system time
*
*Purpose:
*
*Entry:
.       struct tm * ptm - time structure
*
*Exit:
*       milliseconds of current time
*
*Exceptions:
*
*******************************************************************************/

unsigned __cdecl _getsystime(struct tm * ptm)
{
    SYSTEMTIME  st;

    GetLocalTime(&st);

    ptm->tm_isdst       = -1;   /* mktime() computes whether this is */
                                /* during Standard or Daylight time. */
    ptm->tm_sec         = (int)st.wSecond;
    ptm->tm_min         = (int)st.wMinute;
    ptm->tm_hour        = (int)st.wHour;
    ptm->tm_mday        = (int)st.wDay;
    ptm->tm_mon         = (int)st.wMonth - 1;
    ptm->tm_year        = (int)st.wYear - 1900;
    ptm->tm_wday        = (int)st.wDayOfWeek;

    /* Normalize uninitialized fields */
    mktime(ptm);

    return (st.wMilliseconds);
}

/*********************************************************************
*unsigned _setsystime(timestruc, milliseconds) - Set new system time
*
*Purpose:
*
*Entry:
*       struct tm * ptm - time structure
*       unsigned milliseconds - milliseconds of current time
*
*Exit:
*       0 if succeeds
*       system error if fails
*
*Exceptions:
*
*******************************************************************************/

unsigned __cdecl _setsystime(struct tm * ptm, unsigned uMilliseconds)
{
    SYSTEMTIME  st;

    /* Normalize uninitialized fields */
    mktime(ptm);

    st.wYear            = (WORD)(ptm->tm_year + 1900);
    st.wMonth           = (WORD)(ptm->tm_mon + 1);
    st.wDay             = (WORD)ptm->tm_mday;
    st.wHour            = (WORD)(ptm->tm_hour);
    st.wMinute          = (WORD)ptm->tm_min;
    st.wSecond          = (WORD)ptm->tm_sec;
    st.wMilliseconds    = (WORD)uMilliseconds;

    if (!SetLocalTime(&st)) {
        return ((int)GetLastError());
    }

    return (0);
}

#endif

void get_local_iec_time(struct cp56time2a* time)
{
	struct timeb tb;
	struct tm	*ptm;
		
	IT_IT("get_local_iec_time");

	memset(time, 0x00,sizeof(cp56time2a));

    ftime(&tb);

	ptm = localtime(&tb.time);

    if(ptm)
	{																			//from	struct tm	to		struct cp56time2a
		time->hour = ptm->tm_hour;												//		[0.23]				<0.23>
		time->min = ptm->tm_min;												//		[0..59]				<0..59>
		time->msec = ptm->tm_sec*1000 + tb.millitm;								//							<0.. 59999>
		time->mday = ptm->tm_mday;												//		[1..31]				<1..31>
		time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday;		//							<1..7>
		time->month = ptm->tm_mon + 1;											//		[0..11]				<1..12>
		time->year = ptm->tm_year - 100;										//		year (after 1900)	<0.99>  
		time->iv = 0;															//							<0..1> Invalid: <0> is valid, <1> is invalid
		time->su = (u_char)tb.dstflag;											//							<0..1> SUmmer time: <0> is standard time, <1> is summer time
	}
	
	IT_EXIT;
    return;
}


int set_iec_time(struct cp56time2a* time)
{

#ifdef WIN32
	SYSTEMTIME  st;

	IT_IT("set_iec_time");

	st.wYear            = (WORD)((time->year) + 2000);
    st.wMonth           = (WORD)(time->month);
    st.wDay             = (WORD)(time->mday);
    st.wHour            = (WORD)(time->hour);
    st.wMinute          = (WORD)(time->min);
    st.wSecond          = (WORD)(time->msec/1000);
    st.wMilliseconds    = (WORD)(time->msec%1000);
#endif
#ifdef __unix__	

	struct tm tm;
	struct timeval tv;
	
	IT_IT("set_iec_time");

	tm.tm_year	= (int) ((time->year)+100);	//years since 1900
	tm.tm_mon	= (int) (time->month);
	tm.tm_hour	= (int) (time->hour);
	tm.tm_min	= (int) (time->min);
	tm.tm_sec	= (int) (time->msec/1000);

	tv.tv_sec 	= mktime(&tm);
	tv.tv_usec	= (long) ((time->msec%1000)*1000); //microseconds
#endif

	fprintf(stderr, "Try to set host time to: ");
	fflush(stderr);
	
	fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
	time->hour,
	time->min,
	time->msec/1000,
	time->msec%1000,
	time->mday,
	time->month,
	time->year,
	time->iv,
	time->su);
	fflush(stderr);

	IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
	time->hour,
	time->min,
	time->msec/1000,
	time->msec%1000,
	time->mday,
	time->month,
	time->year,
	time->iv,
	time->su);

#ifdef WIN32
    if(!SetLocalTime(&st)) 
	{
		fprintf(stderr, "Failed to set host time\n");
		fflush(stderr);

		IT_EXIT;
        return ((int)GetLastError());
    }
#endif //WIN32
#ifdef __unix__

	if (settimeofday(&tv, NULL) == -1)
	{
		fprintf(stderr, "Failed to set host time\n");
		fflush(stderr);

		IT_EXIT;
		return errno;
	}

#endif //__unix__

	fprintf(stderr, "Set of host time done\n");
	fflush(stderr);

	IT_EXIT;
    return 0;
	
}

void get_utc_iec_time(struct cp56time2a* time)
{
	struct timeb tb;
	struct tm	*ptm;
		
	IT_IT("get_utc_iec_time");

    ftime (&tb);
	ptm = gmtime(&tb.time); //UTC time

	if(ptm)
	{		
		time->hour = ptm->tm_hour;					//<0.23>
		time->min = ptm->tm_min;					//<0..59>
		time->msec = ptm->tm_sec*1000 + tb.millitm; //<0.. 59999>
		time->mday = ptm->tm_mday; //<1..31>
		time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
		time->month = ptm->tm_mon + 1; //<1..12>
		time->year = ptm->tm_year - 100; //<0.99>
		time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
		time->su = (u_char)tb.dstflag; //<0..1> SUmmer time: <0> is standard time, <1> is summer time
	}
	
	IT_EXIT;
    return;
}

time_t Epoch_from_cp56time2a(const struct cp56time2a* time)
{
	struct tm	t;
	time_t epoch = 0;
	int ms;
	
	memset(&t, 0x00, sizeof(struct tm));
	
	t.tm_hour = time->hour;
	t.tm_min = time->min;
	t.tm_sec = time->msec/1000;
	ms = time->msec%1000; //not used
	t.tm_mday = time->mday;
	t.tm_mon = time->month - 1;	  //from <1..12> to	<0..11>				
	t.tm_year = time->year + 100; //from <0..99> to <years from 1900>
	t.tm_isdst = time->su;
	
	epoch = mktime(&t);

	if((epoch == -1) || (time->iv == 1))
	{
		epoch = 0;
	}

	return epoch;
}

////////////////////iec buffers//////////////////////////////////////////////////////////////////////////
struct alloc_iec_buf v_iec_buf[IEC_104_MAX_EVENTS];
////////////////////////////////////////////////////////////////////////////////////////////////////////

void get_iec_buf(struct iec_buf **c)
{
	int q_i;
	
	for(q_i = 0; q_i < IEC_104_MAX_EVENTS; q_i++)
	{
		if(v_iec_buf[q_i].used == 0)
		{
			memset(v_iec_buf[q_i].c, 0x00, IEC104_BUF_LEN);
			*c = v_iec_buf[q_i].c;
			v_iec_buf[q_i].used = 1;
			return;
		}
	}

	sprintf(err_msg, "No more space in iec queue");

	//se ho esaurito gli eventi disponibili, cade il processo
	iec_call_exit_handler(__LINE__,__FILE__, err_msg);
	//*c = NULL;
}

////////////////////////////////read csv //////////////////////////////////////////
#define MAXFLDS 200     /* maximum possible number of fields */
#define MAXFLDSIZE 200   /* longest possible field */
 
void parse_cvs( char *record, char *delim, char arr[][MAXFLDSIZE],int *fldcnt)
{
	char*p = strtok(record,delim);

	int fld=0;
 
	while(p != NULL)
	{
	    strcpy(arr[fld], p);
		fld++;
		p=strtok('\0',delim);
	}

	*fldcnt=fld;
}

///////////////////////////////////directory support//////////////////////////////

typedef int (*PFN_dodir)(const char *name);

/* Called once for each directory */
int dodir(const char *filename)
{
    return 0;
}

/* Called once for each file */
int dofile(const char *filename)
{
	char *pdest;
	static char buffer[_MAX_PATH];
	char file_ext[50];
	
	strcpy(buffer, filename);

	pdest = strrchr(buffer, '.');

	if(pdest)
	{
		strncpy(file_ext, pdest + 1, 4);

		file_ext[4] = '\0';

		if(!_stricmp(file_ext, CSV_FILE_EXT))
		{
			fprintf(stderr,"Found file %s to transfer\n", buffer);
			fflush(stderr);

			strcpy(file_da_trasferire[num_of_file_loaded], buffer);

			num_of_file_loaded++;

			if(num_of_file_loaded >= MAX_FILES_TO_LOAD_IN_FILE_TRANSFER_QUEUE)
			{
				sprintf(err_msg,"num_of_file_loaded >= MAX_FILES_TO_LOAD_IN_FILE_TRANSFER_QUEUE");

				iec_call_exit_handler(__LINE__,__FILE__, err_msg);			
			}
		}
	}

    return 0;
}

/* Walk through directory 'path', calling dodir() for given directory
 * and dofile() for each file.
 * If recurse=1, recurse into subdirectories, calling dodir() for
 * each directory.
 */
int dirwalk(char *path, int recurse, PFN_dodir dodir, PFN_dodir dofile)
{    
#ifdef WIN32
	WIN32_FIND_DATA find_data;
	HANDLE find_handle;
	char pattern[_MAX_PATH];	/* orig pattern + modified pattern */
	char base[_MAX_PATH];
	char name[_MAX_PATH];
	BOOL bMore = TRUE;
	char *p;
		
	if (path) 
	{
		strcpy(pattern, path);

		if (strlen(pattern) != 0)  
		{
			p = pattern + strlen(pattern) -1;
			if (*p == '\\')
				*p = '\0';		// truncate trailing backslash
		}
		
		strcpy(base, pattern);

		if (strchr(base, '*') != NULL) 
		{
			// wildcard already included
			// truncate it from the base path
			if ( (p = strrchr(base, '\\')) != NULL )
				*(++p) = '\0';
		}
		else if (isalpha((unsigned char)pattern[0]) && (pattern[1]==':' && pattern[2]=='\0'))  
		{
			strcat(pattern, "\\*");		// search entire disk
			strcat(base, "\\");
		}
		else 
		{
			// wildcard NOT included
			// check to see if path is a directory
			find_handle = FindFirstFile(pattern, &find_data);
			if (find_handle != INVALID_HANDLE_VALUE) 
			{
				FindClose(find_handle);

				if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
				{
					strcat(pattern, "\\*");		// yes, search files 
					strcat(base, "\\");
				}
				else 
				{
					dofile(path);				// no, return just this file
					return 0;
				}
			}
			else
			{
				return 1;	// path invalid
			}
		}
	}
	else 
	{
		base[0] = '\0';
		strcpy(pattern, "*");
	}
	
	find_handle = FindFirstFile(pattern,  &find_data);

	if (find_handle == INVALID_HANDLE_VALUE)
	{
		return 1;
	}
	
	while(bMore) 
	{
		strcpy(name, base);
		strcat(name, find_data.cFileName);

		if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		{
			if(strcmp(find_data.cFileName, ".") && strcmp(find_data.cFileName, "..")) 
			{
				dodir(name);

				if(recurse)
				{
					dirwalk(name, recurse, dodir, dofile);
				}
			}
		}
		else 
		{
			dofile(name);
		}

		bMore = FindNextFile(find_handle, &find_data);
	}

	FindClose(find_handle);
#endif //WIN32
#ifdef __unix__
	struct stat find_data;
                  DIR *d = NULL;
	struct dirent *de = NULL;
	char pattern[_MAX_PATH];	/* orig pattern + modified pattern */
	char base[_MAX_PATH];
	char name[_MAX_PATH];
	BOOL bMore = TRUE;
	char *p;
		
	if (path) 
	{
		strcpy(pattern, path);

		if (strlen(pattern) != 0)  
		{
			p = pattern + strlen(pattern) -1;
			if (*p == '/')
				*p = '\0';		// truncate trailing backslash
		}
		
		strcpy(base, pattern);

		if (strchr(base, '*') != NULL) 
		{
			// wildcard already included
			// truncate it from the base path
			if ( (p = strrchr(base, '/')) != NULL )
				*(++p) = '\0';
		}
		else if (pattern[0]=='\0')  
		{
			strcat(pattern, "/*");		// search entire disk
			strcat(base, "/");
		}
		else 
		{
			// wildcard NOT included
			// check to see if path is a directory
			if (!stat(pattern, &find_data)) 
			{
				if ((find_data.st_mode & S_IFMT) == S_IFDIR ) 
				{
					strcat(pattern, "/*");		// yes, search files 
					strcat(base, "/");
				}
				else 
				{
					dofile(path);				// no, return just this file
					return 0;
				}
			}
			else
			{
				return 1;	// path invalid
			}
		}
	}
	else 
	{
		strcpy(base, "./");			//use current working directory
		strcpy(pattern, "./*");
	}
	
	d = opendir(base);
	if (d==NULL)
		return 1;
		
	while(de = readdir(d)) 
	{
		strcpy(name, base);
		strcat(name, de->d_name);

		if (!stat(name, &find_data)) {
			if ((find_data.st_mode & S_IFMT) == S_IFDIR ) 
			{
				if(strcmp(de->d_name, ".") && strcmp(de->d_name, "..")) 
				{
					dodir(name);

					if(recurse)
					{
						dirwalk(name, recurse, dodir, dofile);
					}
				}
			}
			else 
			{
				dofile(name);
			}
		}
	}

	closedir(d);
#endif //__unix__

	return 0;
}


void process_timer_send_frame(struct iecsock *s, void *arg)
{
	IT_IT("process_timer_send_frame");

	switch(state_global_logs)
	{
		case LOGS_STARTUP:
		{
			//Get utc time
			get_utc_iec_time(&logs_start);
			logs_start_epoch = Epoch_from_cp56time2a(&logs_start);
			state_global_logs = LOGS_WAIT_FOR_CHECK;
		}
		break;
		case LOGS_WAIT_FOR_CHECK:
		{
			get_utc_iec_time(&logs_finish);

			logs_finish_epoch = Epoch_from_cp56time2a(&logs_finish);

			//fprintf(stderr,"finish_epoch = %lu\n", finish_epoch);
			//fflush(stderr);
			
			logs_delta = logs_finish_epoch - logs_start_epoch;

			//fprintf(stderr,"delta = %lu\n", delta);
			//fflush(stderr);

			if(logs_delta > TIME_IN_SECONDS_BETWEEN_CHECKS)
			{
				state_global_logs = LOGS_CHECK_AGE;
			}
		}
		break;
		case LOGS_CHECK_AGE:
		{
			fprintf(stderr, "Checking length of log files\n");
			fflush(stderr);
			//log file management
			if(iec_protocol_log_stream != NULL)
			{
				struct _stat fbuf;

				int hnd = fileno(iec_protocol_log_stream);
				
				if(_fstat(hnd, &fbuf)) 
				{ 
					//error
				}
				else
				{ 
					unsigned int length_of_log_file = 0;
					length_of_log_file = fbuf.st_size;

					//if(length_of_log_file > 1000000UL)
					if(length_of_log_file > 268435456UL)
					{
						struct tm *ptm;
						time_t t;
						char log_time_stamp[100];
						char log_file_name[250];

						fclose(iec_protocol_log_stream);
						
						//_getsystime(&ptm);
						t = time(NULL);
						ptm = localtime(&t);
						
						sprintf(log_time_stamp, "_%d_%d_%d_%d_%d_%d",
						ptm->tm_year + 1900,
						ptm->tm_mon + 1,
						ptm->tm_mday,
						ptm->tm_hour,
						ptm->tm_min,
						ptm->tm_sec);

						strcpy(log_file_name, iec_optional_log_file_name);
						strcat(log_file_name, log_time_stamp);
						strcat(log_file_name, ".log");
						
						iec_protocol_log_stream = freopen(log_file_name, "w", stderr);

						//TODO on 22-10-2011
						//Remove file older than 1 month
					}
				} 
			}

			state_global_logs = LOGS_STARTUP;
		}
		break;
		default:
		break;
	}

	/////////Config database state machine
	switch(state_database)
	{
		case DB_STARTUP:
		{
			get_utc_iec_time(&start);
			db_start_epoch = Epoch_from_cp56time2a(&start);

			state_database = DB_START_COUNT_DOWN_TIMER;
		}
		break;
		case DB_START_COUNT_DOWN_TIMER:
		{
			//Wait eventually opc_client_da.exe to request to LOAD_DATABASE

			if(db_start_epoch)
			{
				get_utc_iec_time(&finish);

				db_finish_epoch = Epoch_from_cp56time2a(&finish);

				//fprintf(stderr,"db_finish_epoch = %lu\n", finish_epoch);
				//fflush(stderr);
				
				delta = db_finish_epoch - db_start_epoch;

				//fprintf(stderr,"delta = %lu\n", delta);
				//fflush(stderr);

				if(delta > DB_COUNT_DOWN_TIME_IN_SECONDS)
				{
                    //disabled for IndigoSCADA
					//state_database = LOAD_DATABASE;
					db_start_epoch = 0;
				}
			}
		}
		break;
		case LOAD_DATABASE:
		{
			int error = 0;

			/*

			if(iec_items_table != NULL) //if we are reloading the database, when due to a reset process command
			{
				free(iec_items_table);
				iec_items_table = NULL;
				num_iec_items = 0;
			}

			if(iec_items_table == NULL)
			{
				int j;
				FILE *db = NULL;
				
				char tmp[2048] = {0x0};
				int fldcnt = 0;
				char arr[MAXFLDS][MAXFLDSIZE] = {0x0};
				int recordcnt = 0, tmp_counter = 0, work_counter = 0, semicolon_position_1 = 0, k;
				char* working_line = NULL;
				//////////////////////CRC//////////////////
				unsigned long crc = 0;
				long charcnt = 0;
				int err = 0;
				///////////////////////////////////////////
				
				db = fopen(file_configurazione,"rb");

				if(db == NULL)
				{
					//Il database non è presente sul gateway
					fprintf(stderr,"Failed to open config file %s\n",file_configurazione);
					fflush(stderr);
					state_database = DB_STARTUP;
					error = 1;
					goto error_LOAD_DATABASE;
				}
								
				recordcnt = 0;
				while(fgets(tmp, sizeof(tmp), db) != 0){recordcnt++;}
				num_iec_items = recordcnt - 1; //Tolgo la riga dell'header

				rewind(db);

				//check for empty file apa+++ 17-06-2011
				if(recordcnt < 2)
				{
					//Il database non è stato generato correttamente
					fprintf(stderr,"Number of records is < 2\n");
					fflush(stderr);
					state_database = DB_STARTUP;
					error = 1;
					goto error_LOAD_DATABASE;
				}
				/////////////////////////////////////begin CRC calculation///////////////////////////////////////////
											
				err = crc32file(file_configurazione, &crc, &charcnt);

				if(err != 0)
				{
					fprintf(stderr,"Failed to calculate checksum");
					fflush(stderr);
					state_database = DB_STARTUP;
					error = 1;
					goto error_LOAD_DATABASE;
				}

				fprintf(stderr,"CRC = %08lX; char count = %7ld; Errors = %d; File = %s\n", crc, charcnt, err, file_configurazione);
				fflush(stderr);
								
				if(s->type == IEC_MASTER)
				{
					CRC_file_configurazione_master = crc;
				}
				else if(s->type == IEC_SLAVE)
				{
					CRC_file_configurazione_slave  = crc;				
				}

				/////////////////////////////////////end CRC calculation///////////////////////////////////////////
				rewind(db);

				iec_items_table = (struct iec_item *)malloc(num_iec_items * sizeof(struct iec_item));

				if(iec_items_table == NULL)
				{
					fprintf(stderr,"Error allocating gateway database\n");
					fflush(stderr);
					state_database = DB_STARTUP;
					error = 1;
					goto error_LOAD_DATABASE;
				}

				memset(iec_items_table, 0x00, num_iec_items * sizeof(struct iec_item));

				//////////////////read csv////////////////////////////////////////////////////////
				working_line = (char*)malloc(2000);
				
				if(working_line == NULL)
				{
					fprintf(stderr,"Error in malloc at line %d\n", __LINE__);
					fflush(stderr);
					error = 1;
					goto error_LOAD_DATABASE;
				}

				fprintf(stderr,"Loading %s database\n", file_configurazione);
				fflush(stderr);
			 				
				for(j = -1; fgets(tmp, sizeof(tmp), db) != 0; j++) //read each record
				{
					if(j == -1)
					{
						continue; //salto la riga dell'header
					}
					
					semicolon_position_1 = 0;

					for(tmp_counter = 0, work_counter = 0; *(tmp + tmp_counter) != '\n' ; tmp_counter++)
					{
						if(*(tmp + tmp_counter) == ';')
						{
							if(tmp_counter - semicolon_position_1 == 1)
							{
								*(working_line + work_counter) = ' ';
								work_counter++;
							}

							semicolon_position_1 = tmp_counter;
						}
						
						*(working_line + work_counter) = *(tmp + tmp_counter);
						work_counter++;
					}

					*(working_line + work_counter) = '\n';
					*(working_line + work_counter) = '\0';

					//parsing....

					recordcnt++;

					parse_cvs(working_line, ";", arr, &fldcnt);    //split record into fields

					//fprintf(stderr,"Record %d contiene %d colonne\n", j + 1, fldcnt);
					//fflush(stderr);

					for(k = 0; k < fldcnt; k++)
					{                              
						//fprintf(stderr,"\tField number: %3d==%s ", k + 1, arr[k]); // print each field
						//fflush(stderr);

						switch(k+1)
						{
							case 1:
							{
								//opc_server_item_id
								strcpy(iec_items_table[j].opc_server_item_id, arr[k]);

								fprintf(stderr,"%s", arr[k]); // print each field
								fflush(stderr);
								fprintf(stderr,";");
								fflush(stderr);
							}
							break;
							case 2:
							{
								//ioa_control_center
								iec_items_table[j].iec_obj.ioa = atoi(arr[k]);
								iec_items_table[j].ioa_control_center = atoi(arr[k]);

								fprintf(stderr,"%s", arr[k]); // print each field
								fflush(stderr);
								fprintf(stderr,";");
								fflush(stderr);
							}
							break;
							case 3:
							{
								//iec_type

								iec_items_table[j].iec_type = atoi(arr[k]);

								fprintf(stderr,"%s", arr[k]); // print each field
								fflush(stderr);
								fprintf(stderr,";");
								fflush(stderr);

								switch(iec_items_table[j].iec_type)
								{
									case M_SP_NA_1:
									{
										iec_items_table[j].iec_obj.o.type1.iv = 1;
									}
									break;
									case M_DP_NA_1:
									{
										iec_items_table[j].iec_obj.o.type3.iv = 1;
									}
									break;
									case M_BO_NA_1:
									{
										iec_items_table[j].iec_obj.o.type7.iv = 1;
									}
									break;
									case M_ME_NA_1:
									{
										iec_items_table[j].iec_obj.o.type9.iv = 1;
									}
									break;
									case M_ME_NB_1:
									{
										iec_items_table[j].iec_obj.o.type11.iv = 1;
									}
									break;
									case M_ME_NC_1:
									{
										iec_items_table[j].iec_obj.o.type13.iv = 1;
									}
									break;
									case M_IT_NA_1:
									{
										iec_items_table[j].iec_obj.o.type15.iv = 1;
									}
									break;
									case M_SP_TB_1:
									{
										iec_items_table[j].iec_obj.o.type30.iv = 1;
									}
									break;
									case M_DP_TB_1:
									{
										iec_items_table[j].iec_obj.o.type31.iv = 1;
									}
									break;
									case M_BO_TB_1:
									{
										iec_items_table[j].iec_obj.o.type33.iv = 1;
									}
									break;
									case M_ME_TD_1:
									{
										iec_items_table[j].iec_obj.o.type34.iv = 1;
									}
									break;
									case M_ME_TE_1:
									{
										iec_items_table[j].iec_obj.o.type35.iv = 1;
									}
									break;
									case M_ME_TF_1:
									{
										iec_items_table[j].iec_obj.o.type36.iv = 1;
									}
									break;
									case M_IT_TB_1:
									{
										iec_items_table[j].iec_obj.o.type37.iv = 1;
									}
									break;
									case 0:
									{
										//0 significa che l' OPC item ha un tipo non supportato nel gateway
										fprintf(stderr,"Not supported type\n");
										fflush(stderr);
									}
									break;
									default:
									{
										fprintf(stderr,"Not supported type\n");
										fflush(stderr);
									}
									break;
								}
							}
							break;
							case 4:
							{
								//hClient
								iec_items_table[j].hClient = atoi(arr[k]);
								
								fprintf(stderr,"%s", arr[k]); // print each field
								fprintf(stderr,";");
							}
							break;
							default:
							{
								fprintf(stderr,"Unknown field in record of config database %s\n",file_configurazione);
								fflush(stderr);
							}
							break;
						}
					}

					fprintf(stderr,"\n");
					fflush(stderr);
				}

				fprintf(stderr,"Allocated config database %s\n",file_configurazione);
				fflush(stderr);

				error_LOAD_DATABASE:

				if(db)
				{
					fclose(db);
					db = NULL;
				}

				if(working_line)
				{
					free(working_line);
					working_line = NULL;
				}
			}
			*/

			if(s->type == IEC_SLAVE)
			{
				//Siamo nello SLAVE, che, ad avvenuto caricamento del database, invia una M_EI_NA_1 al MASTER
				//Send M_EI_NA_1
				if(error == 0)
				{
					int len;				
					struct iec_buf *c;
					struct iec_unit_id answer_unit;				
					struct iec_type70 answer_type;				
					u_int ioa;	//importante: ioa da 3 ottetti deve essere u_int e non u_short					
					u_char *cp;	
					
					ioa = 0; //IOA deve essere nullo
					answer_type.coi_bs1 = 1;  //initialization after local change of parameters
					answer_type.coi_ui7 = 0; //local power switch on
					answer_unit.type = M_EI_NA_1;	//M_EI_NA_1			
					answer_unit.num = 1;				
					answer_unit.sq = 0;

					if(error)
					{
						answer_unit.pn = 1;//negative
					}
					else
					{
						answer_unit.pn = 0;//positive
					}

					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;				
					answer_unit.originator = 0;				
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type70) + IOA_ADDRLEN) * answer_unit.num);				
					if (len > IEC104_ASDU_MAX)
					{
						state_database = DB_STARTUP;
						error = 1;
						break;
					}

					get_iec_buf(&c);

					c->data_len = len;				
					cp = c->data;				
					answer_unit.cause = 0x04; 	//initialised
					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
					memcpy(cp, &ioa, IOA_ADDRLEN);				
					cp += IOA_ADDRLEN;				
					memcpy(cp, &answer_type, sizeof(struct iec_type70));				
					cp += sizeof(struct iec_type70);				
					iecsock_prepare_iframe(c);				
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

					fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
					fflush(stderr);

					IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);


					state_of_slave = SLAVE_INITIALIZED;
				}
				//////////////////////////////////////////////////////////////////////////////////////
			}

			if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, che, ad avvenuto caricamento del database, sends M_BO_NA_1 with CRC of database just loaded
				if(error == 0)
				{
					int len;				
					struct iec_buf *c;
					struct iec_unit_id answer_unit;				
					struct iec_type7 answer_type;
					u_int ioa;	//importante: ioa da 3 ottetti deve essere u_int e non u_short					
					u_char *cp;	
					
					ioa = 0; //IOA nullo ?

					answer_type.bl = 0;
					answer_type.iv = 0;
					answer_type.nt = 0;
					answer_type.ov = 0;
					answer_type.sb = 0;
					
					if(s->type == IEC_MASTER)
					{
						memcpy(&(answer_type.stcd), &CRC_file_configurazione_master, sizeof(CRC_file_configurazione_master));
					}
					//else if(s->type == IEC_SLAVE)
					//{
					//	memcpy(&(answer_type.stcd), &CRC_file_configurazione_slave, sizeof(CRC_file_configurazione_slave));
					//}
					
					answer_unit.type = M_BO_NA_1;
					answer_unit.num = 1;				
					answer_unit.sq = 0;
					answer_unit.pn = 0;				
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;				
					answer_unit.originator = 0;				
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type7) + IOA_ADDRLEN) * answer_unit.num);
					if (len > IEC104_ASDU_MAX)
					{
						error = 1;
						state_database = DB_STARTUP;
						break;				
					}

					get_iec_buf(&c);
					
					c->data_len = len;				
					cp = c->data;				
					answer_unit.cause = 0x03; 	//Spontaneous
					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
					memcpy(cp, &ioa, IOA_ADDRLEN);				
					cp += IOA_ADDRLEN;				
					memcpy(cp, &answer_type, sizeof(struct iec_type7));				
					cp += sizeof(struct iec_type7);				
					iecsock_prepare_iframe(c);				
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

					fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
					fflush(stderr);

					IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
					//////////////////////////////////////////////////////////////////////////////////////
				}
			}

			if(error == 0)
			{
				state_database = DATABASE_LOADED;
			}
		}
		break;
		default:
		break;
	}

	/////////File transfer state machine
	#ifdef SUPPORT_FILE_TRANSFER
	switch(state_file_transfer)
	{
		case FT_IDLE:
		{
		}
		break;
		case FT_ERROR:
		{
			fprintf(stderr, "An error occurred during file transfer\n");
			fflush(stderr);
			sprintf(err_msg, "An error occurred during file transfer");
			//state_file_transfer = FT_SCAN_WORKING_DIRECTORY;
			iec_call_exit_handler(__LINE__,__FILE__,err_msg);
		}
		break;
		case FT_SCAN_WORKING_DIRECTORY:
		{
			//Siamo nel file sender side

			index_of_file_to_transfer = 0;
			num_of_file_loaded = 0;

			if(dirwalk(file_transf_working_dir, 0, &dodir, &dofile))
			{
				fprintf(stderr, "Error in scanning the working file transfer directory\n");
				fflush(stderr);
				state_file_transfer = FT_ERROR;
			}
			else
			{
				if(num_of_file_loaded > 0)
				{
					state_file_transfer = FT_PREPARE_DIRECTORY;

					//apa+++ 11-12-2010
					//Stopping count down to load database, because we are trasfering db files
					state_database = DB_STARTUP;
				}
				else
				{
					//Aspetta nuovi file che il producer mettera' nella working directory
				}
			}
		}
		break;
		case FT_PREPARE_DIRECTORY:
		{
			//Siamo nel file sender side

			//Send F_DR_TA_1
			
			int len;
			struct iec_buf *c;
			struct iec_unit_id answer_unit;
			struct iec_type126 answer_type;				
			u_int ioa;	//importante: ioa da 3 ottetti deve essere u_int e non u_short					
			u_char *cp;
			FILE *fr  = NULL;
			struct _stat fbuf;
			int fh, result;
			struct tm	*ptm;
			
			length_of_file = 0;
			number_of_segments = 0;
			resto_dei_byte_da_leggere = 0;

			fprintf(stderr,"index_of_file_to_transfer = %d\n", index_of_file_to_transfer);
			fflush(stderr);

			if((fh = _open(file_da_trasferire[index_of_file_to_transfer], _O_RDONLY | _O_BINARY)) ==  -1)
			{
				fprintf(stderr,"Failed to open file %s at line %d %s\n", file_da_trasferire[index_of_file_to_transfer], __LINE__, __FILE__);
				fflush(stderr);
				state_file_transfer = FT_ERROR;
				break;
			}

			//Get data associated with "fh":
			result = _fstat(fh, &fbuf);

			//Check if statistics are valid:
			if(result != 0)
			{
			  fprintf(stderr,"Bad file handle\n");
			  fflush(stderr);
			  _close(fh);
			  state_file_transfer = FT_ERROR;
			  break;
			}
			else
			{
				length_of_file = fbuf.st_size;

				memset(&ft_creat_time, 0, sizeof(struct cp56time2a));

				ptm = gmtime(&fbuf.st_ctime); //file creation UTC time

				if(ptm)
				{
					ft_creat_time.hour = ptm->tm_hour;					//<0.23>
					ft_creat_time.min = ptm->tm_min;					//<0..59>
					ft_creat_time.msec = ptm->tm_sec*1000 + 0; //Force 0 milliseconds <0.. 59999>
					ft_creat_time.mday = ptm->tm_mday; //<1..31>
					ft_creat_time.wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
					ft_creat_time.month = ptm->tm_mon + 1; //<1..12>
					ft_creat_time.year = ptm->tm_year - 100; //<0.99>
					ft_creat_time.iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
					ft_creat_time.su = 0; //Force to 0 //<0..1> SUmmer time: <0> is standard time, <1> is summer time
				}
			}

			_close(fh);
			
			//Max length of file is 2^24 -1
			if(length_of_file > 16777215UL)
			{
				fprintf(stderr,"File to transfer %s is too long: %d bytes\n", file_da_trasferire[index_of_file_to_transfer], length_of_file);
				fflush(stderr);
				state_file_transfer = FT_ERROR;
				break;
			}

			length_of_segment = MAX_SEGMENT_LEN;
			
			number_of_segments = length_of_file/MAX_SEGMENT_LEN;

			resto_dei_byte_da_leggere = length_of_file%MAX_SEGMENT_LEN;
	
			ioa = 601;
			answer_type.lof = length_of_file; //Lunghezza file su 24 bit, max is 16777215 bytes (2^24 - 1)
			answer_type.nof = 3; //Nome del file
			answer_type.sof_fa = 0; //file waits for transfer
			answer_type.sof_for = 0; //name defines file
			answer_type.sof_lfd = 1; //last file of the directory
			answer_type.sof_status = 0; //default
			answer_type.time = ft_creat_time;  //creation time of the file
			answer_unit.type = 126;	//F_DR_TA_1		
			answer_unit.num = 1;				
			answer_unit.sq = 1;  //Questa è un'asdu paccata => si possono impaccare nel frame 126 solo ioa adiacenti!
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type126) + IOA_ADDRLEN) * answer_unit.num);				
			if (len > IEC104_ASDU_MAX)
			{
				state_file_transfer = FT_ERROR;
				break;
			}

			get_iec_buf(&c);

			c->data_len = len;				
			cp = c->data;				
			answer_unit.cause = 0x03; 	//spontaneous			
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
			memcpy(cp, &ioa, IOA_ADDRLEN);				
			cp += IOA_ADDRLEN;				
			memcpy(cp, &answer_type, sizeof(struct iec_type126));				
			cp += sizeof(struct iec_type126);				
			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			//////////////////////////////////////////////////////////////////////////////////////

			state_file_transfer = FT_DIRECTORY_SENT;
		}
		break;
		case FT_ENDED_ON_SENDER_SIDE:
		{
			//Siamo nel file sender side

			//Send reset command (C_RP_NA_1) to receiver side of file, so the new configuration file it will be loaded by the receiver side
			int len;
			struct iec_buf *c;
			struct iec_unit_id answer_unit;
			struct iec_type105 answer_type;
			u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
			u_char *cp;

			answer_type.qrp = 1;  //General reset process

			ioa = 0; //ioa risposta C_RP_NA_1 deve essere nullo 
			answer_unit.type = 105; //C_RP_NA_1
			answer_unit.num = 1;
			answer_unit.sq = 0;
			answer_unit.pn = 0; //positive
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;
			answer_unit.originator = 0;
						
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type105) + IOA_ADDRLEN) * answer_unit.num);

			if (len > IEC104_ASDU_MAX)
			{
				state_file_transfer = FT_ERROR;
				break;
			}

			get_iec_buf(&c);
			
			c->data_len = len;

			cp = c->data;

			//activation
			answer_unit.cause = 0x06; 

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;
			
			memcpy(cp, &ioa, IOA_ADDRLEN);
			cp += IOA_ADDRLEN;
			memcpy(cp, &answer_type, sizeof(struct iec_type105));
			cp += sizeof(struct iec_type105);
								
			iecsock_prepare_iframe(c);
			TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			//////////////////////////////////////////////////////////////////////////////////////////////////
			
			state_file_transfer = FT_SCAN_WORKING_DIRECTORY;
		}
		break;
		case FT_RENAME_FILE_TRANSFERED:
		{
			//Siamo nel file receiver side
			// Attempt to rename file:
			int result;

			if(s->type == IEC_SLAVE)
			{
				result = rename(file_trasferito, punti_opc_pg);

				if(result != 0)
				{
					fprintf(stderr,"Could not rename '%s'\n", file_trasferito);
					fflush(stderr);
				}
				else
				{
					fprintf(stderr,"File '%s' renamed to '%s'\n", file_trasferito, punti_opc_pg);
					fflush(stderr);
				}

				//apa+++ 11-12-2010
				//Stopping waiting punti_opc_pg timer
				state_database = DB_STARTUP;
			}
			else if(s->type == IEC_MASTER)
			{
				result = rename(file_trasferito, file_configurazione);

				if(result != 0)
				{
					fprintf(stderr,"Could not rename '%s'\n", file_trasferito);
					fflush(stderr);
				}
				else
				{
					fprintf(stderr,"File '%s' renamed to '%s'\n", file_trasferito, file_configurazione);
					fflush(stderr);
				}
			}

			state_file_transfer = FT_SCAN_WORKING_DIRECTORY;
		}
		break;
		case FT_REMOVE_CONFIGURATION_FILE:
		{
			//Siamo nel file receiver side

			if(s->type == IEC_SLAVE)
			{
				//_unlink();
				if(remove(punti_opc_pg) == -1)
				{
					fprintf(stderr,"Error removing file %s\n", punti_opc_pg);
					fflush(stderr);
				}
				else
				{
					fprintf(stderr,"Removing file %s\n", punti_opc_pg);
					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//_unlink();
				if(remove(file_configurazione) == -1)
				{
					fprintf(stderr,"Error removing file %s\n", file_configurazione);
					fflush(stderr);
				}
				else
				{
					fprintf(stderr,"Removing file %s\n", file_configurazione);
					fflush(stderr);
				}
			}

			state_file_transfer = FT_RENAME_FILE_TRANSFERED;
		}
		break;
		case FT_REMOVE_TRANSFERED_FILE:
		{
			//Siamo nel file sender side

			if(remove(file_da_trasferire[index_of_file_to_transfer]) == -1)
			{
				fprintf(stderr,"Error removing file %s\n", file_da_trasferire[index_of_file_to_transfer]);
				fflush(stderr);
				state_file_transfer = FT_ERROR;
			}
			else
			{
				fprintf(stderr,"Removing file %s\n", file_da_trasferire[index_of_file_to_transfer]);
				fflush(stderr);

				index_of_file_to_transfer++;

				if(index_of_file_to_transfer >= num_of_file_loaded)
				{
					state_file_transfer = FT_ENDED_ON_SENDER_SIDE;
				}
				else
				{
					state_file_transfer = FT_PREPARE_DIRECTORY;
				}
			}
		}
		break;
		default:
		break;
	}
    #endif //SUPPORT_FILE_TRANSFER

	/////////Gateway state machine
	switch(state_gateway)
	{
		case GW_STARTUP:
		{
			//if(s->type == IEC_MASTER)
			//if(s->type == IEC_SLAVE)
			{
				//state_gateway = GW_SEND_NEW_CONFIG_FILE_TO_SLAVE;
			}
		}
		break;
		case GW_SEND_NEW_CONFIG_FILE_TO_SLAVE:
		{
			state_file_transfer = FT_SCAN_WORKING_DIRECTORY;
			state_gateway = GW_SCAN_FOR_CONFIG_FILE;
		}
		break;
		default:
		break;
	}

	/////////General interrogation state machine
	switch(state_general_interrogation)
	{
		case GI_IDLE:
		{
			if(s->type == IEC_SLAVE)
			{
				point_to_load = 0;
			}
		}
		break;
		case GI_REQUEST_GI_TO_LOWER_LEVER_SCADA:
		{
			if(s->type == IEC_SLAVE)
			{
				//The SLAVE sends to opc_client.exe or iec101master.exe the message C_IC_NA_1 (general interrogation)
				struct iec_item item_to_send;
				memset(&item_to_send,0x00, sizeof(struct iec_item));
				item_to_send.iec_type = C_IC_NA_1; //general interrogation command
				item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
				fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));
				//////////////////////////////////////////////////////////////////////////////////////////////////////

				state_general_interrogation = GI_REQUEST_GI_TO_LOWER_LEVEL_SCADA_DONE;
			}
		}
		break;
		case GI_LOAD_POINTS:
		{
			if(s->type == IEC_SLAVE)
			{
				general_interrogation_load_points(s);
			}
		}
		break;
		case GI_ACTIVATION:
		{
			if(s->type == IEC_MASTER)
			{
				//Get utc time of GI activation
				get_utc_iec_time(&start);
				start_epoch = Epoch_from_cp56time2a(&start);

				state_general_interrogation = GI_WAIT_FOR_CONFIRMATION;

				//Important note:
				//GI activation confirmation should arrive within the
				//MAX_GI_TIME_IN_SECONDS time
			}
		}
		break;
		case GI_WAIT_FOR_CONFIRMATION:
		{
			if(s->type == IEC_MASTER)
			{
				if(start_epoch)
				{
					get_utc_iec_time(&finish);

					finish_epoch = Epoch_from_cp56time2a(&finish);

					//fprintf(stderr,"finish_epoch = %lu\n", finish_epoch);
					//fflush(stderr);
					
					delta = finish_epoch - start_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);

					if(delta > MAX_GI_TIME_IN_SECONDS)
					{
						//la GI non si e' conclusa nel tempo previsto
						fprintf(stderr, "No GI confirmation within in %d seconds\n", MAX_GI_TIME_IN_SECONDS);
						fflush(stderr);
						sprintf(err_msg, "No GI confirmation within in %d seconds\n", MAX_GI_TIME_IN_SECONDS);
						state_general_interrogation = GI_IDLE;
						start_epoch = 0;
					}
				}
			}
		}
		break;
		case GI_CONFIRMATION:
		{
			if(s->type == IEC_MASTER)
			{
				state_general_interrogation = GI_WAIT_FOR_TERMINATION;
				start_epoch = 0;

				//Important note:
				//	GI activation termination arrives after:
				//	1) all spontaneous messages in front of GI
				//	2) all GI messages themselfs

				//	So, if we have thousand of spontaneous messages in front of GI messages
				//	or we have thousand of GI messages, the GI can take
				//	a lot of time to terminate. 
				//	So we do not know how many seconds, minutes it takes to terminate the GI,
				//	so we cannot start here a timer to check the arriving of GI activation termination.
			}
		}
		break;
		case GI_NEG_CONFIRMATION:
		{
			if(s->type == IEC_MASTER)
			{
				sprintf(err_msg, "Negative GI confirmation");
				state_general_interrogation = GI_IDLE;
			}		
		}
		break;
		case GI_WAIT_FOR_TERMINATION:
		{
			if(s->type == IEC_MASTER)
			{
				;
			}
		}
		break;
		case GI_TERMINATION:
		{
			if(s->type == IEC_MASTER)
			{
				start_epoch = 0;
				state_general_interrogation = GI_IDLE;
			}
		}
		break;
		case GI_NEG_TERMINATION:
		{
			if(s->type == IEC_MASTER)
			{
				fprintf(stderr, "Negative termination of General Interrogation\n");
				fflush(stderr);

				start_epoch = 0;
				state_general_interrogation = GI_IDLE;
			}
		}
		break;
		case GI_ERROR:
		{
			if(s->type == IEC_MASTER)
			{
				//exit process
				iec_call_exit_handler(__LINE__,__FILE__, err_msg);
			}
		}
		break;
		default:
		break;
	}

	/////////State machine of spontaneous and ack from field
	switch(state_monitoring_direction)
	{
		case MSG_IDLE:
		{
			if(s->type == IEC_SLAVE)
			{
				state_monitoring_direction = MSG_RECEIVE_MESSAGES_FROM_FIELD;	
			}
		}
		break;
		case MSG_RECEIVE_MESSAGES_FROM_FIELD:
		{
			if(s->type == IEC_SLAVE)
			{
				get_items_from_producer(s);
			}
			
			//Lo slave rimane sempre in questo stato di caricamento dei punti spontanei
			//state_monitoring_direction = MSG_IDLE;
		}
		break;
		default:
		break;
	}

	/////////State machine of commands from superior SCADA
	switch(state_control_direction)
	{
		case CMD_IDLE:
		{
			if(s->type == IEC_MASTER)
			{
				state_control_direction = CMD_SEND_COMMANDS;
			}
		}
		break;
		case CMD_SEND_COMMANDS:
		{
			if(s->type == IEC_MASTER)
			{
				get_items_from_producer(s);
			}
			
			//Il master rimane sempre in questo stato di invio comandi
			//state_monitoring_direction = CMD_IDLE;
		}
		break;
		default:
		break;
	}
	
	IT_EXIT;
}

//Single point
#define plot_info_sp(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.sp, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.sp, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
	}


//Double point
#define plot_info_dp(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i dp:%i bl:%i sb:%i nt:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.dp, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i dp:%i bl:%i sb:%i nt:%i iv:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.dp, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
	}


#define plot_info_sp_wt(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.sp, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.sp, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
	}


#define plot_info_dp_wt(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i dp:%i bl:%i sb:%i nt:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.dp, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i dp:%i bl:%i sb:%i nt:%i iv:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.dp, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
	}

//Misure

#define plot_info_mis_float(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i mv:%f bl:%i sb:%i nt:%i iv:%i ov:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.mv, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv, obj[i].o.struct_name.ov);\
		fflush(stderr);\
		IT_COMMENT7("Value: IOA:%i mv:%f bl:%i sb:%i nt:%i iv:%i ov:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.mv, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv, obj[i].o.struct_name.ov);\
	}

#define plot_info_mis_short(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i mv:%d bl:%i sb:%i nt:%i iv:%i ov:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.mv, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv, obj[i].o.struct_name.ov);\
		fflush(stderr);\
		IT_COMMENT7("Value: IOA:%i mv:%d bl:%i sb:%i nt:%i iv:%i ov:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.mv, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv, obj[i].o.struct_name.ov);\
	}


#define plot_info_mis_wt_short(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i mv:%d bl:%i sb:%i nt:%i iv:%i ov:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.mv, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv, obj[i].o.struct_name.ov);\
		fflush(stderr);\
		IT_COMMENT7("Value: IOA:%i mv:%d bl:%i sb:%i nt:%i iv:%i ov:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.mv, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv, obj[i].o.struct_name.ov);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
	}

#define plot_info_mis_wt_float(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i mv:%f bl:%i sb:%i nt:%i iv:%i ov:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.mv, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv, obj[i].o.struct_name.ov);\
		fflush(stderr);\
		IT_COMMENT7("Value: IOA:%i mv:%f bl:%i sb:%i nt:%i iv:%i ov:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.mv, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv, obj[i].o.struct_name.ov);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
	}

#define plot_info_counter(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i counter:%u sq:%i cy:%i ca:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.counter, obj[i].o.struct_name.sq, \
		obj[i].o.struct_name.cy, obj[i].o.struct_name.ca, obj[i].o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i counter:%u sq:%i cy:%i ca:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.counter, obj[i].o.struct_name.sq, \
		obj[i].o.struct_name.cy, obj[i].o.struct_name.ca, obj[i].o.struct_name.iv);\
	}


#define plot_info_counter_wt(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i counter:%u sq:%i cy:%i ca:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.counter, obj[i].o.struct_name.sq, \
		obj[i].o.struct_name.cy, obj[i].o.struct_name.ca, obj[i].o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i counter:%u sq:%i cy:%i ca:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.counter, obj[i].o.struct_name.sq, \
		obj[i].o.struct_name.cy, obj[i].o.struct_name.ca, obj[i].o.struct_name.iv);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
	}

//Bitstring of 32 bit

#define plot_info_bits(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i stcd:%u bl:%i sb:%i nt:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.stcd, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i stcd:%u bl:%i sb:%i nt:%i iv:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.stcd, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
	}

#define plot_info_bits_wt(struct_name) \
	for (i = 0; i < n_ioa_in_asdu; i++){	\
		fprintf(stderr, "Value: IOA:%i stcd:%u bl:%i sb:%i nt:%i iv:%i\n",\
		obj[i].ioa,\
		obj[i].o.struct_name.stcd, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i stcd:%u bl:%i sb:%i nt:%i iv:%i",\
		obj[i].ioa,\
		obj[i].o.struct_name.stcd, obj[i].o.struct_name.bl, \
		obj[i].o.struct_name.sb, obj[i].o.struct_name.nt, obj[i].o.struct_name.iv);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		obj[i].o.struct_name.time.hour,\
		obj[i].o.struct_name.time.min,\
		obj[i].o.struct_name.time.msec/1000,\
		obj[i].o.struct_name.time.msec%1000,\
		obj[i].o.struct_name.time.mday,\
		obj[i].o.struct_name.time.month,\
		obj[i].o.struct_name.time.year,\
		obj[i].o.struct_name.time.iv,\
		obj[i].o.struct_name.time.su);\
	}


#define send_all_items(struct_name) \
		for (i = 0; i < n_ioa_in_asdu; i++) \
		{	\
			item_to_send.iec_type = type; \
			item_to_send.iec_obj.ioa = obj[i].ioa; \
			item_to_send.iec_obj.o.struct_name = obj[i].o.struct_name; \
			item_to_send.msg_id = msg_sent_to_superior_scada++; \
			item_to_send.ioa_control_center = obj[i].ioa; \
			item_to_send.casdu = common_address_of_asdu; \
			item_to_send.cause = cause; \
			item_to_send.checksum = 0; \
			send_item_to_superior_scada(&item_to_send); \
		}
/*
#define prepare_item_to_send(struct_name) \
		i = 0; \
		item_to_send.iec_type = type; \
		item_to_send.iec_obj.ioa = obj[i].ioa; \
		item_to_send.iec_obj.o.struct_name = obj[i].o.struct_name; \
		item_to_send.msg_id = msg_sent_to_superior_scada++; \
		if(iec_items_table != NULL)\
		{\
			strcpy(item_to_send.opc_server_item_id, iec_items_table[obj[i].ioa  - 1].opc_server_item_id); \
		}\
		item_to_send.ioa_control_center = obj[i].ioa;
*/

static u_int msg_sent_to_superior_scada = 0;
static u_int msg_sent_in_control_direction = 0;

static time_t sbo_epoch = 0;
static time_t command_arrive_epoch = 0;

void process_data_received_hook(struct iecsock *s, struct iec_buf *b)
{
	int ret, i;
	int n_ioa_in_asdu; 
	u_short caddr;
	u_char cause, test, pn, type;
	struct iec_object obj[IEC_OBJECT_MAX];
	struct iec_buf *c;
	///////fifo////////////////////////////
	struct iec_item item_to_send;
	
	IT_IT("process_data_received_hook");

	//fprintf(stderr, "%s: data_len=%d\n", "data_received_hook", b->data_len);
	//fflush(stderr);

	IT_COMMENT2("%s: ASDU length=%d ", "data_received_hook", b->data_len);
	
	memset(&obj, 0, IEC_OBJECT_MAX*sizeof(struct iec_object));
		
	ret = iecasdu_parse(obj, &type, &caddr, &n_ioa_in_asdu, &cause, &test, &pn, 
		IOA_ADDRLEN, COM_ADDRLEN, b->data, b->data_len);

	if(ret == 1 || ret == 2)
	{
		//fprintf(stderr,"ret %d for type %d\n", ret, type);
		//fflush(stderr);

        if(ret == 1)
        {
           fprintf(stderr,"Incorrect ASDU with type %d\n", type);
		   fflush(stderr);
        }
        else
        if(ret == 2)
        {
           fprintf(stderr,"Unsupported ASDU with type %d\n", type);
		   fflush(stderr);
        }

		//sprintf(err_msg, "ret %d for type %d\n", ret, type);
		//iec_call_exit_handler(__LINE__,__FILE__,err_msg);
		return;
	}

	fprintf(stderr, "Rec. Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", type, caddr, n_ioa_in_asdu, cause, test, pn);
	fflush(stderr);

	IT_COMMENT6("Rec. Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", type, caddr, n_ioa_in_asdu, cause, test, pn);

	//Azzero tutta la struttura e setto solo i campi significativi
	memset(&item_to_send,0x00, sizeof(struct iec_item));

	switch(type)
	{
		case M_SP_NA_1:
		{
			plot_info_sp(type1);

			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type1);
				
			}
		}
		break;
		case M_DP_NA_1:
		{
			plot_info_dp(type3);

			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type3);
			}
		}
		break;
		case M_BO_NA_1:
		{
			plot_info_bits(type7);
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type7);
			}
		}
		break;
		case M_ME_NA_1:
		{
			plot_info_mis_short(type9);
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type9);
				
			}
		}
		break;
		case M_ME_NB_1:
		{
			plot_info_mis_short(type11);
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type11);
				
			}
		}
		break;
		case M_ME_NC_1:
		{
			plot_info_mis_float(type13);
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type13);
				
			}
		}
		break;
		case M_IT_NA_1:
		{
			plot_info_counter(type15);

			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type15);
				
			}
		}
		break;
		case M_SP_TB_1:
		{
			plot_info_sp_wt(type30);

			/*
			for (i = 0; i < n_ioa_in_asdu; i++) 
			{
				fprintf(stderr, "Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i\n",
				obj[i].ioa,
				obj[i].o.type30.sp, obj[i].o.type30.bl, 
				obj[i].o.type30.sb, obj[i].o.type30.nt, obj[i].o.type30.iv);
				fflush(stderr);

				IT_COMMENT6("Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i",
				obj[i].ioa,
				obj[i].o.type30.sp, obj[i].o.type30.bl, 
				obj[i].o.type30.sb, obj[i].o.type30.nt, obj[i].o.type30.iv);

				fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
				obj[i].o.type30.time.hour,
				obj[i].o.type30.time.min,
				obj[i].o.type30.time.msec/1000,
				obj[i].o.type30.time.msec%1000,
				obj[i].o.type30.time.mday,
				obj[i].o.type30.time.month,
				obj[i].o.type30.time.year,
				obj[i].o.type30.time.iv,
				obj[i].o.type30.time.su);
				fflush(stderr);

				IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
				obj[i].o.type30.time.hour,
				obj[i].o.type30.time.min,
				obj[i].o.type30.time.msec/1000,
				obj[i].o.type30.time.msec%1000,
				obj[i].o.type30.time.mday,
				obj[i].o.type30.time.month,
				obj[i].o.type30.time.year,
				obj[i].o.type30.time.iv,
				obj[i].o.type30.time.su);
			}
			*/
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type30);
				
			}
		}
		break;
		case M_DP_TB_1:
		{
			plot_info_dp_wt(type31);
			
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type31);
				
			}
		}
		break;
		case M_BO_TB_1:
		{
			plot_info_bits_wt(type33);
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type33);
				
			}
		}
		break;
		case M_ME_TD_1:
		{
			plot_info_mis_wt_short(type34);
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type34);
				
			}
		}
		break;
		case M_ME_TE_1:
		{
			plot_info_mis_wt_short(type35);

			/*
			for (i = 0; i < n_ioa_in_asdu; i++){
				fprintf(stderr, "Value: IOA:%i mv:%d bl:%i sb:%i nt:%i iv:%i ov:%i\n",
				obj[i].ioa,
				obj[i].o.type35.mv, 
				obj[i].o.type35.bl,
				obj[i].o.type35.sb, 
				obj[i].o.type35.nt,
				obj[i].o.type35.iv, 
				obj[i].o.type35.ov);
				fflush(stderr);
				
				fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
				obj[i].o.type35.time.hour,
				obj[i].o.type35.time.min,
				obj[i].o.type35.time.msec/1000,
				obj[i].o.type35.time.msec%1000,
				obj[i].o.type35.time.mday,
				obj[i].o.type35.time.month,
				obj[i].o.type35.time.year,
				obj[i].o.type35.time.iv,
				obj[i].o.type35.time.su);
				fflush(stderr);
			}
			*/
			
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction

			/*	
				assert(i == 1);
				i = 0; 
				item_to_send.iec_type = type; 
				item_to_send.iec_obj.ioa = obj[i].ioa; 
				item_to_send.iec_obj.o.type35 = obj[i].o.type35; 
				item_to_send.msg_id = msg_sent_to_superior_scada++; 
				if(iec_items_table != NULL) 
				{ 
					strcpy(item_to_send.opc_server_item_id, iec_items_table[obj[i].ioa  - 1].opc_server_item_id); 
					item_to_send.ioa_control_center = iec_items_table[obj[i].ioa  - 1].ioa_control_center;
				}
			*/
				send_all_items(type35);
				
			}
		}
		break;
		case M_ME_TF_1:
		{
			plot_info_mis_wt_float(type36);
			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type36);
				
			}
		}
		break;
		case M_IT_TB_1:
		{
			plot_info_counter_wt(type37);

			if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				send_all_items(type37);
				
			}
		}
		break;
		case M_EI_NA_1:
		{
			if(s->type == IEC_MASTER)
			{
				if(pn == 0)//positive initialised
				{
					//////////////Invio clock sync to SLAVE
					{
						int len;				
						struct iec_buf *c;				
						struct iec_unit_id answer_unit;
						struct iec_type103 answer_type;
						u_int ioa;	//importante: ioa da 3 ottetti deve essere u_int e non u_short					
						u_char *cp;

						get_local_iec_time(&(answer_type.time));
						
						ioa = 0;  //Nullo cfr. par. 7.3.4.4 IEC 101 standard
										
						//prepara la activation

						answer_unit.type = C_CS_NA_1;
						answer_unit.num = 1;
						answer_unit.sq = 0;
						answer_unit.pn = 0; //positive
						answer_unit.ca = common_address_of_asdu;
						answer_unit.t = 0;
						answer_unit.originator = 0;
									
						len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type103) + IOA_ADDRLEN) * answer_unit.num);

						if (len > IEC104_ASDU_MAX)
						{
							fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
							fflush(stderr);
							fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type103) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type103) + IOA_ADDRLEN);
							fflush(stderr);

							#ifdef WIN32
							ExitProcess(0);
                            #else
							exit(EXIT_FAILURE);
                            #endif
						}

						get_iec_buf(&c);
					
						c->data_len = len;

						cp = c->data;
						
						answer_unit.cause = 0x06; //activation

						memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
						cp += IEC_TYPEID_LEN + COM_ADDRLEN;
												
						memcpy(cp, &ioa, IOA_ADDRLEN);
						cp += IOA_ADDRLEN;
						memcpy(cp, &answer_type, sizeof(struct iec_type103));
						cp += sizeof(struct iec_type103);
														
						iecsock_prepare_iframe(c);
						TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
					}

					state_of_slave = SLAVE_INITIALIZED;

					//////////////////////////Invio GI to SLAVE/////////////////////////////////////////////////////////
					{
						int len;				
						struct iec_buf *c;				
						struct iec_unit_id answer_unit;				
						struct iec_type100 answer_type;
						u_int ioa;	//importante: ioa da 3 ottetti deve essere u_int e non u_short					
						u_char *cp;	
						ioa = 0;
						answer_type.qoi = 0x14; //Station interrogation
						answer_unit.type = C_IC_NA_1;
						answer_unit.num = 1;				
						answer_unit.sq = 0;				
						answer_unit.pn = 0;	//positive
						answer_unit.ca = caddr;				
						answer_unit.t = 0;				
						answer_unit.originator = 0;				
						len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type100) + IOA_ADDRLEN) * answer_unit.num);				

						if (len > IEC104_ASDU_MAX)
						{
							fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
							fflush(stderr);
							fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type100) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type100) + IOA_ADDRLEN);
							fflush(stderr);
							#ifdef WIN32
							ExitProcess(0);
                            #else
							exit(EXIT_FAILURE);
                            #endif
						}
						
						get_iec_buf(&c);
						
						c->data_len = len;				
						cp = c->data;				
						answer_unit.cause = 0x06; 	//activation
						memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
						cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
						memcpy(cp, &ioa, IOA_ADDRLEN);				
						cp += IOA_ADDRLEN;				
						memcpy(cp, &answer_type, sizeof(struct iec_type100));				
						cp += sizeof(struct iec_type100);				
						iecsock_prepare_iframe(c);				
						
						TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

						fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
						fflush(stderr);

						IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

						state_general_interrogation = GI_ACTIVATION;
					}
				}
				else
				{
                    struct iec_item item_to_send;
					memset(&item_to_send,0x00, sizeof(struct iec_item));
					item_to_send.iec_type = M_EI_NA_1;
					item_to_send.is_neg = 1;
                    item_to_send.cause = 0x04; //Initialized

                    i = 0;

					item_to_send.iec_obj.ioa = obj[i].ioa; //ioa risposta
					item_to_send.iec_obj.o.type70 = obj[i].o.type70;
					item_to_send.msg_id = msg_sent_in_control_direction++;
					item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));				
					fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));

					state_of_slave = SLAVE_NOT_INITIALIZED;

					fprintf(stderr, "Master detected error of initilization of slave\n");
					fflush(stderr);
				}
			}
			
			/*
			if(s->type == IEC_SLAVE)
			{
				//Nello SLAVE qui ricevo la conferma di inizializzazione del master
				//Mando la M_EI_NA_1 al master

				//Send M_EI_NA_1
				{
					int len;				
					struct iec_buf *c;
					struct iec_unit_id answer_unit;				
					struct iec_type70 answer_type;				
					u_int ioa;	//importante: ioa da 3 ottetti deve essere u_int e non u_short					
					u_char *cp;	
					
					ioa = 0; //IOA deve essere nullo
					answer_type.coi_bs1 = 1;  //initialization after local change of parameters
					answer_type.coi_ui7 = 0; //local power switch on
					answer_unit.type = M_EI_NA_1;	//M_EI_NA_1			
					answer_unit.num = 1;				
					answer_unit.sq = 0;
					
					//answer_unit.pn = 1;//negative
										
					answer_unit.pn = 0;//positive

					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;				
					answer_unit.originator = 0;				
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type70) + IOA_ADDRLEN) * answer_unit.num);				
					if (len > IEC104_ASDU_MAX)
					{
						state_database = DB_STARTUP;
						break;
					}

					get_iec_buf(&c);

					c->data_len = len;				
					cp = c->data;				
					answer_unit.cause = 0x04; 	//initialised
					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
					memcpy(cp, &ioa, IOA_ADDRLEN);				
					cp += IOA_ADDRLEN;				
					memcpy(cp, &answer_type, sizeof(struct iec_type70));				
					cp += sizeof(struct iec_type70);				
					iecsock_prepare_iframe(c);				
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

					fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
					fflush(stderr);

					IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				}
				//////////////////////////////////////////////////////////////////////////////////////
			}
			*/
		}
		break;
		case C_TS_NA_1:
		{
			if(s->type == IEC_SLAVE)
			{
				int len;
				struct iec_unit_id answer_unit;
				struct iec_type104 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				u_char *cp;
								
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++)
				{
					fprintf(stderr, "Value: IOA:%i fbp:%x\n",
					obj[i].ioa,
					obj[i].o.type104.fbp);
					fflush(stderr);
					IT_COMMENT2("Value: IOA:%i fbp:%x",
					obj[i].ioa,
					obj[i].o.type104.fbp);
					
					ioa = obj[i].ioa; //ioa risposta

					answer_type.fbp =  obj[i].o.type104.fbp;
				}
				
				i = 0;

				//prepara la activation confirmation

				answer_unit.type = C_TS_NA_1;
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type104) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type104) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type104) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type104));
				cp += sizeof(struct iec_type104);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//qui ricevo activation confirmation from slave

				if(cause == 0x07) //activation confirmation
				{
					if(pn == 0) //positive
					{
						fprintf(stderr,"Positive activation confirmation of C_TS_NA_1\n");
						fflush(stderr);
					}
					else
					{
						fprintf(stderr,"Negative activation confirmation of C_TS_NA_1\n");
						fflush(stderr);
					}
				}
			}
		}
		break;
		case C_TS_TA_1:
		{
			if(s->type == IEC_SLAVE)
			{
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++)
				{
					fprintf(stderr, "Value: IOA:%i fbp:%x\n",
					obj[i].ioa,
					obj[i].o.type107.fbp);
					fflush(stderr);
					IT_COMMENT2("Value: IOA:%i fbp:%x",
					obj[i].ioa,
					obj[i].o.type107.fbp);

					fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					obj[i].o.type107.time.hour,
					obj[i].o.type107.time.min,
					obj[i].o.type107.time.msec/1000,
					obj[i].o.type107.time.msec%1000,
					obj[i].o.type107.time.mday,
					obj[i].o.type107.time.month,
					obj[i].o.type107.time.year,
					obj[i].o.type107.time.iv,
					obj[i].o.type107.time.su);
					fflush(stderr);

					IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
					obj[i].o.type107.time.hour,
					obj[i].o.type107.time.min,
					obj[i].o.type107.time.msec/1000,
					obj[i].o.type107.time.msec%1000,
					obj[i].o.type107.time.mday,
					obj[i].o.type107.time.month,
					obj[i].o.type107.time.year,
					obj[i].o.type107.time.iv,
					obj[i].o.type107.time.su);
				}
				
				i = 0; //DO NOT REMOVE!
								
				{
					//send test command to opc_client.exe
					struct iec_item item_to_send;
					memset(&item_to_send,0x00, sizeof(struct iec_item));
					item_to_send.iec_type = C_TS_TA_1;
					item_to_send.iec_obj.ioa = obj[i].ioa; //ioa risposta
					item_to_send.iec_obj.o.type107 = obj[i].o.type107;
					item_to_send.msg_id = msg_sent_in_control_direction++;
					item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));				
					fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));
					///////////////////////////////////////////////////////////////////////////////////////////
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//qui ricevo activation confirmation from slave

				if(cause == 0x07) //activation confirmation
				{
					if(pn == 0) //positive
					{
						fprintf(stderr,"Positive activation confirmation of C_TS_TA_1\n");
						fflush(stderr);

						if(n_ioa_in_asdu != 1)
						{
							fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
							fflush(stderr);
							#ifdef WIN32
							ExitProcess(0);
                            #else
							exit(EXIT_FAILURE);
                            #endif
						}

						for (i = 0; i < n_ioa_in_asdu; i++)
						{
							fprintf(stderr, "Value: IOA:%i fbp:%x\n",
							obj[i].ioa,
							obj[i].o.type107.fbp);
							fflush(stderr);
							IT_COMMENT2("Value: IOA:%i fbp:%x",
							obj[i].ioa,
							obj[i].o.type107.fbp);

							fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
							obj[i].o.type107.time.hour,
							obj[i].o.type107.time.min,
							obj[i].o.type107.time.msec/1000,
							obj[i].o.type107.time.msec%1000,
							obj[i].o.type107.time.mday,
							obj[i].o.type107.time.month,
							obj[i].o.type107.time.year,
							obj[i].o.type107.time.iv,
							obj[i].o.type107.time.su);
							fflush(stderr);

							IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
							obj[i].o.type107.time.hour,
							obj[i].o.type107.time.min,
							obj[i].o.type107.time.msec/1000,
							obj[i].o.type107.time.msec%1000,
							obj[i].o.type107.time.mday,
							obj[i].o.type107.time.month,
							obj[i].o.type107.time.year,
							obj[i].o.type107.time.iv,
							obj[i].o.type107.time.su);
						}
						
						i = 0; //DO NOT REMOVE!
										
						{
							struct iec_item item_to_send;
							memset(&item_to_send,0x00, sizeof(struct iec_item));
							item_to_send.iec_type = C_TS_TA_1;
							item_to_send.iec_obj.ioa = obj[i].ioa; //ioa risposta
							item_to_send.iec_obj.o.type107 = obj[i].o.type107;
							item_to_send.msg_id = msg_sent_to_superior_scada++;
							item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));				
							fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
						}
					}
					else
					{
						fprintf(stderr,"Negative activation confirmation of C_TS_TA_1\n");
						fflush(stderr);
					}
				}
			}
		}
		break;
		case C_SC_NA_1://single command
		{
			if(s->type == IEC_SLAVE)
			{
				/*
				unsigned char asdu_45_confirmation[] = {
					0x2D, 0x01, 0x07, 0x00, 0x4D, 0x01, 0x19, 0x00, 0x00, 0x01
				};
				unsigned char asdu_45_termination[] = {
					0x2D, 0x01, 0x0A, 0x00, 0x4D, 0x01, 0x19, 0x00, 0x00, 0x01
				};
				*/

				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////
                
				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type45 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
								
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
				
				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i qu:%i scs:%i se:%i\n",
					obj[i].ioa,
					obj[i].o.type45.qu, obj[i].o.type45.scs, 
					obj[i].o.type45.se);
					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i qu:%i scs:%i se:%i\n",
					obj[i].ioa,
					obj[i].o.type45.qu, obj[i].o.type45.scs, 
					obj[i].o.type45.se);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.qu =  obj[i].o.type45.qu;
					//answer_type.scs = obj[i].o.type45.scs;
					//answer_type.se = obj[i].o.type45.se;
					//answer_type.res = obj[i].o.type45.res;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_SC_NA_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type45) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type45) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type45) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type45));
				cp += sizeof(struct iec_type45);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type45.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif
					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field (OPC client or iec101master)
						cp56time2a command_time_arrival;
						struct iec_item item_to_send;
						memset(&item_to_send,0x00, sizeof(struct iec_item));
						item_to_send.iec_type = C_SC_NA_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;
						//Add time stamp to command to check its life time
						///create an UTC time stamp at command arrival////////
						get_utc_iec_time(&command_time_arrival);
						item_to_send.iec_obj.o.type58.time = command_time_arrival;
						item_to_send.iec_obj.o.type45.scs = obj[i].o.type45.scs;
						item_to_send.iec_obj.o.type45.qu = obj[i].o.type45.qu;
						item_to_send.iec_obj.o.type45.se = obj[i].o.type45.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));				
						fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);

						fprintf(stderr,"Execute the command with utc initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						item_to_send.iec_obj.o.type58.time.hour,
						item_to_send.iec_obj.o.type58.time.min,
						item_to_send.iec_obj.o.type58.time.msec/1000,
						item_to_send.iec_obj.o.type58.time.msec%1000,
						item_to_send.iec_obj.o.type58.time.mday,
						item_to_send.iec_obj.o.type58.time.month,
						item_to_send.iec_obj.o.type58.time.year,
						item_to_send.iec_obj.o.type58.time.iv,
						item_to_send.iec_obj.o.type58.time.su);

						fflush(stderr);
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type45.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of single command\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of single command\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior scada
					    
					    struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_SC_NA_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of single command\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of single command\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}

						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));				
					    fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of single command\n");
							fflush(stderr);
					break;
				}
			}
		}			
		break;
		case C_SC_TA_1://single command with time stamp
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type58 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
				
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
				
				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i qu:%i scs:%i se:%i\n",
					obj[i].ioa,
					obj[i].o.type58.qu, obj[i].o.type58.scs, 
					obj[i].o.type58.se);

					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i qu:%i scs:%i se:%i\n",
					obj[i].ioa,
					obj[i].o.type58.qu, obj[i].o.type58.scs, 
					obj[i].o.type58.se);

					fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					obj[i].o.type58.time.hour,
					obj[i].o.type58.time.min,
					obj[i].o.type58.time.msec/1000,
					obj[i].o.type58.time.msec%1000,
					obj[i].o.type58.time.mday,
					obj[i].o.type58.time.month,
					obj[i].o.type58.time.year,
					obj[i].o.type58.time.iv,
					obj[i].o.type58.time.su);

					fflush(stderr);

					IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
					obj[i].o.type58.time.hour,
					obj[i].o.type58.time.min,
					obj[i].o.type58.time.msec/1000,
					obj[i].o.type58.time.msec%1000,
					obj[i].o.type58.time.mday,
					obj[i].o.type58.time.month,
					obj[i].o.type58.time.year,
					obj[i].o.type58.time.iv,
					obj[i].o.type58.time.su);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.qu =  obj[i].o.type58.qu;
					//answer_type.scs = obj[i].o.type58.scs;
					//answer_type.se = obj[i].o.type58.se;
					//answer_type.res = obj[i].o.type58.res;
					//answer_type.time = obj[i].o.type58.time;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_SC_TA_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type58) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type58) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type58) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type58));
				cp += sizeof(struct iec_type58);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type58.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif

					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field
						struct iec_item item_to_send;
						memset(&item_to_send, 0x00, sizeof(struct iec_item));
						item_to_send.iec_type = C_SC_TA_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;
						item_to_send.iec_obj.o.type58.time = obj[i].o.type58.time;
						item_to_send.iec_obj.o.type58.scs = obj[i].o.type58.scs;
						item_to_send.iec_obj.o.type58.qu = obj[i].o.type58.qu;
						item_to_send.iec_obj.o.type58.se = obj[i].o.type58.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));				
						fifo_put(fifo_control_direction, (char*)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);
						
						fprintf(stderr,"Execute the command with initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						obj[i].o.type58.time.hour,
						obj[i].o.type58.time.min,
						obj[i].o.type58.time.msec/1000,
						obj[i].o.type58.time.msec%1000,
						obj[i].o.type58.time.mday,
						obj[i].o.type58.time.month,
						obj[i].o.type58.time.year,
						obj[i].o.type58.time.iv,
						obj[i].o.type58.time.su);
						fflush(stderr);
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type58.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of single command with time stamp\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of single command with time stamp\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior scada
					    
					    struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_SC_TA_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of single command with time stamp\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of single command with time stamp\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}

						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
					    fifo_put(fifo_monitor_direction, (char*)&item_to_send, sizeof(struct iec_item));
					    ////////////////////////////////////////////////////////////////
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of single command with time stamp\n");
							fflush(stderr);
					break;
				}
			}
		}			
		break;
		case C_DC_NA_1: //double command
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type46 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
								
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
				
				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i qu:%i dcs:%i se:%i\n",
					obj[i].ioa,
					obj[i].o.type46.qu, obj[i].o.type46.dcs, 
					obj[i].o.type46.se);

					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i qu:%i dcs:%i se:%i\n",
					obj[i].ioa,
					obj[i].o.type46.qu, obj[i].o.type46.dcs, 
					obj[i].o.type46.se);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.qu =  obj[i].o.type46.qu;
					//answer_type.dcs = obj[i].o.type46.dcs;
					//answer_type.se = obj[i].o.type46.se;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_DC_NA_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type46) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type46) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type46) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
							exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type46));
				cp += sizeof(struct iec_type46);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type46.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif
					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field (OPC client)
						cp56time2a command_time_arrival;
						struct iec_item item_to_send;
						memset(&item_to_send, 0x00, sizeof(struct iec_item));
						item_to_send.iec_type = C_DC_NA_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;
						//Add time stamp to command to check its life time
						///create an UTC time stamp at command arrival////////
						get_utc_iec_time(&command_time_arrival);
						item_to_send.iec_obj.o.type59.time = command_time_arrival;
						//////////////////////////////////////////////////////
						item_to_send.iec_obj.o.type46.dcs = obj[i].o.type46.dcs;
						item_to_send.iec_obj.o.type46.qu = obj[i].o.type46.qu;
						item_to_send.iec_obj.o.type46.se = obj[i].o.type46.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
						fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);

						fprintf(stderr,"Execute the command with utc initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						item_to_send.iec_obj.o.type59.time.hour,
						item_to_send.iec_obj.o.type59.time.min,
						item_to_send.iec_obj.o.type59.time.msec/1000,
						item_to_send.iec_obj.o.type59.time.msec%1000,
						item_to_send.iec_obj.o.type59.time.mday,
						item_to_send.iec_obj.o.type59.time.month,
						item_to_send.iec_obj.o.type59.time.year,
						item_to_send.iec_obj.o.type59.time.iv,
						item_to_send.iec_obj.o.type59.time.su);

						fflush(stderr);

						//apa 25-08-2011
						///////////Send command ALSO to OPC AE server////////////////////////////////////////////////
						//NOTA BENE: IOA of C_DC_NA_1 COMMANDS MUST BE UNIQUE, AMONG OPC DA AND OPC AE COMMANDS
						//Gli IOA dei comandi 46 deveno essere univoci su tutta la I/O list

						//The fifo_control_direction_ae FIFO send only the acknowledge commands to OPC AE server
						//item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
						//fifo_put(fifo_control_direction_ae, (char *)&item_to_send, sizeof(struct iec_item));

						////////////////////////////////////////////////////////////////////////////////////////
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type46.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of double command\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of double command\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior scada
					    struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_DC_NA_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of double command\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of double command\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}

					    item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
					    fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of double command\n");
							fflush(stderr);
					break;
				}
			}
		}			
		break;
		case C_DC_TA_1://double command with time stamp
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type59 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
				
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i qu:%i dcs:%i se:%i\n",
					obj[i].ioa,
					obj[i].o.type59.qu, obj[i].o.type59.dcs, 
					obj[i].o.type59.se);

					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i qu:%i dcs:%i se:%i\n",
					obj[i].ioa,
					obj[i].o.type59.qu, obj[i].o.type59.dcs, 
					obj[i].o.type59.se);

					fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					obj[i].o.type59.time.hour,
					obj[i].o.type59.time.min,
					obj[i].o.type59.time.msec/1000,
					obj[i].o.type59.time.msec%1000,
					obj[i].o.type59.time.mday,
					obj[i].o.type59.time.month,
					obj[i].o.type59.time.year,
					obj[i].o.type59.time.iv,
					obj[i].o.type59.time.su);

					fflush(stderr);

					IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
					obj[i].o.type59.time.hour,
					obj[i].o.type59.time.min,
					obj[i].o.type59.time.msec/1000,
					obj[i].o.type59.time.msec%1000,
					obj[i].o.type59.time.mday,
					obj[i].o.type59.time.month,
					obj[i].o.type59.time.year,
					obj[i].o.type59.time.iv,
					obj[i].o.type59.time.su);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.qu =  obj[i].o.type59.qu;
					//answer_type.dcs = obj[i].o.type59.dcs;
					//answer_type.se = obj[i].o.type59.se;
					//answer_type.time = obj[i].o.type59.time;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_DC_TA_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type59) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type59) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type59) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type59));
				cp += sizeof(struct iec_type59);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type59.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif

					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field (OPC client)

						struct iec_item item_to_send;
						memset(&item_to_send, 0x00, sizeof(struct iec_item));
						item_to_send.iec_type = C_DC_TA_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;
						item_to_send.iec_obj.o.type59.time = obj[i].o.type59.time;
						item_to_send.iec_obj.o.type59.dcs = obj[i].o.type59.dcs;
						item_to_send.iec_obj.o.type59.qu = obj[i].o.type59.qu;
						item_to_send.iec_obj.o.type59.se = obj[i].o.type59.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
						fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);
						
						fprintf(stderr,"Execute the command with initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						obj[i].o.type59.time.hour,
						obj[i].o.type59.time.min,
						obj[i].o.type59.time.msec/1000,
						obj[i].o.type59.time.msec%1000,
						obj[i].o.type59.time.mday,
						obj[i].o.type59.time.month,
						obj[i].o.type59.time.year,
						obj[i].o.type59.time.iv,
						obj[i].o.type59.time.su);
						fflush(stderr);
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type59.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of double command with time stamp\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of double command with time stamp\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior scada
					    struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_DC_TA_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of double command with time stamp\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of double command with time stamp\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}

						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
					    ////////////////////////////////////////////////////////////////
					    fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
					    /////////////////////////////////////////////////////////////////
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of double command with time stamp\n");
							fflush(stderr);
					break;
				}
			}
		}
		break;
		case C_SE_NA_1: //set point command, normalized value
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type48 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
								
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
				
				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type48.sv,
					obj[i].o.type48.ql,	obj[i].o.type48.se);

					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type48.sv,
					obj[i].o.type48.ql,	obj[i].o.type48.se);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.ql =  obj[i].o.type48.ql;
					//answer_type.se = obj[i].o.type48.se;
					//answer_type.sv = obj[i].o.type48.sv;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_SE_NA_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type48) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type48) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type48) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type48));
				cp += sizeof(struct iec_type48);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type48.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif
					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field (OPC client)

						cp56time2a command_time_arrival;
						struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));

						item_to_send.iec_type = C_SE_NA_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;

						//Add time stamp to command to check its life time
						///create an UTC time stamp at command arrival////////
						get_utc_iec_time(&command_time_arrival);
						item_to_send.iec_obj.o.type61.time = command_time_arrival;
						//////////////////////////////////////////////////////
						item_to_send.iec_obj.o.type48.sv = obj[i].o.type48.sv;
						item_to_send.iec_obj.o.type48.ql = obj[i].o.type48.ql;
						item_to_send.iec_obj.o.type48.se = obj[i].o.type48.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
						fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);

						fprintf(stderr,"Execute the command with utc initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						item_to_send.iec_obj.o.type61.time.hour,
						item_to_send.iec_obj.o.type61.time.min,
						item_to_send.iec_obj.o.type61.time.msec/1000,
						item_to_send.iec_obj.o.type61.time.msec%1000,
						item_to_send.iec_obj.o.type61.time.mday,
						item_to_send.iec_obj.o.type61.time.month,
						item_to_send.iec_obj.o.type61.time.year,
						item_to_send.iec_obj.o.type61.time.iv,
						item_to_send.iec_obj.o.type61.time.su);

						fflush(stderr);
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type48.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of set point command\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of set point command\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior scada
					    struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_SE_NA_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}

						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));					    
					    fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
					    /////////////////////////////////////////////////////////////////
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of set point command\n");
							fflush(stderr);
					break;
				}
			}
		}			
		break;
		case C_SE_NB_1: //set point command, scaled value
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type49 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
								
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
				
				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type49.sv,
					obj[i].o.type49.ql,	obj[i].o.type49.se);
					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type49.sv,
					obj[i].o.type49.ql,	obj[i].o.type49.se);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.ql =  obj[i].o.type49.ql;
					//answer_type.se = obj[i].o.type49.se;
					//answer_type.sv = obj[i].o.type49.sv;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_SE_NB_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type49) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type49) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type49) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type49));
				cp += sizeof(struct iec_type49);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type49.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif
					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field (OPC client)

						cp56time2a command_time_arrival;
						struct iec_item item_to_send;
						memset(&item_to_send,0x00, sizeof(struct iec_item));
						item_to_send.iec_type = C_SE_NB_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;

						//Add time stamp to command to check its life time
						///create an UTC time stamp at command arrival////////
						get_utc_iec_time(&command_time_arrival);
						item_to_send.iec_obj.o.type62.time = command_time_arrival;
						//////////////////////////////////////////////////////
						item_to_send.iec_obj.o.type49.sv = obj[i].o.type49.sv;
						item_to_send.iec_obj.o.type49.ql = obj[i].o.type49.ql;
						item_to_send.iec_obj.o.type49.se = obj[i].o.type49.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
						fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);

						fprintf(stderr,"Execute the command with utc initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						item_to_send.iec_obj.o.type62.time.hour,
						item_to_send.iec_obj.o.type62.time.min,
						item_to_send.iec_obj.o.type62.time.msec/1000,
						item_to_send.iec_obj.o.type62.time.msec%1000,
						item_to_send.iec_obj.o.type62.time.mday,
						item_to_send.iec_obj.o.type62.time.month,
						item_to_send.iec_obj.o.type62.time.year,
						item_to_send.iec_obj.o.type62.time.iv,
						item_to_send.iec_obj.o.type62.time.su);

						fflush(stderr);
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type49.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of set point command\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of set point command\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior SCADA
					    struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_SE_NB_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}
					    
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
					    fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
					    /////////////////////////////////////////////////////////////////
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of set point command\n");
							fflush(stderr);
					break;
				}
			}
		}			
		break;
		case C_SE_NC_1: //set point command, short floating point
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type50 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
								
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type50.sv,
					obj[i].o.type50.ql,	obj[i].o.type50.se);
					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type50.sv,
					obj[i].o.type50.ql,	obj[i].o.type50.se);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.ql =  obj[i].o.type50.ql;
					//answer_type.se = obj[i].o.type50.se;
					//answer_type.sv = obj[i].o.type50.sv;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_SE_NC_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type50) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type50));
				cp += sizeof(struct iec_type50);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type50.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif
					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field (OPC client)

						cp56time2a command_time_arrival;
						struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));

						item_to_send.iec_type = C_SE_NC_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;

						//Add time stamp to command to check its life time
						///create an UTC time stamp at command arrival////////
						get_utc_iec_time(&command_time_arrival);
						item_to_send.iec_obj.o.type63.time = command_time_arrival;
						//////////////////////////////////////////////////////
						item_to_send.iec_obj.o.type50.sv = obj[i].o.type50.sv;
						item_to_send.iec_obj.o.type50.ql = obj[i].o.type50.ql;
						item_to_send.iec_obj.o.type50.se = obj[i].o.type50.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));												
						fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);

						fprintf(stderr,"Execute the command with utc initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						item_to_send.iec_obj.o.type63.time.hour,
						item_to_send.iec_obj.o.type63.time.min,
						item_to_send.iec_obj.o.type63.time.msec/1000,
						item_to_send.iec_obj.o.type63.time.msec%1000,
						item_to_send.iec_obj.o.type63.time.mday,
						item_to_send.iec_obj.o.type63.time.month,
						item_to_send.iec_obj.o.type63.time.year,
						item_to_send.iec_obj.o.type63.time.iv,
						item_to_send.iec_obj.o.type63.time.su);

						fflush(stderr);
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type50.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of set point command\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of set point command\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior SCADA
					    struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_SE_NC_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}

						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));					    
					    fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
					    /////////////////////////////////////////////////////////////////
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of set point command\n");
							fflush(stderr);
					break;
				}
			}
		}			
		break;
		case C_SE_TA_1: //set point command, normalized value with time
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type61 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
								
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type61.sv,
					obj[i].o.type61.ql,	obj[i].o.type61.se);
					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type61.sv,
					obj[i].o.type61.ql,	obj[i].o.type61.se);

					fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					obj[i].o.type61.time.hour,
					obj[i].o.type61.time.min,
					obj[i].o.type61.time.msec/1000,
					obj[i].o.type61.time.msec%1000,
					obj[i].o.type61.time.mday,
					obj[i].o.type61.time.month,
					obj[i].o.type61.time.year,
					obj[i].o.type61.time.iv,
					obj[i].o.type61.time.su);
					fflush(stderr);

					IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
					obj[i].o.type61.time.hour,
					obj[i].o.type61.time.min,
					obj[i].o.type61.time.msec/1000,
					obj[i].o.type61.time.msec%1000,
					obj[i].o.type61.time.mday,
					obj[i].o.type61.time.month,
					obj[i].o.type61.time.year,
					obj[i].o.type61.time.iv,
					obj[i].o.type61.time.su);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.ql =  obj[i].o.type61.ql;
					//answer_type.se = obj[i].o.type61.se;
					//answer_type.sv = obj[i].o.type61.sv;
					//answer_type.time = obj[i].o.type61.time;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_SE_TA_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type61) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type61));
				cp += sizeof(struct iec_type61);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type61.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif
					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field
						struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
						item_to_send.iec_type = C_SE_TA_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;
						item_to_send.iec_obj.o.type61.time = obj[i].o.type61.time;
						item_to_send.iec_obj.o.type61.sv = obj[i].o.type61.sv;
						item_to_send.iec_obj.o.type61.ql = obj[i].o.type61.ql;
						item_to_send.iec_obj.o.type61.se = obj[i].o.type61.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));												
						fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);

						fprintf(stderr,"Execute the command with utc initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						item_to_send.iec_obj.o.type61.time.hour,
						item_to_send.iec_obj.o.type61.time.min,
						item_to_send.iec_obj.o.type61.time.msec/1000,
						item_to_send.iec_obj.o.type61.time.msec%1000,
						item_to_send.iec_obj.o.type61.time.mday,
						item_to_send.iec_obj.o.type61.time.month,
						item_to_send.iec_obj.o.type61.time.year,
						item_to_send.iec_obj.o.type61.time.iv,
						item_to_send.iec_obj.o.type61.time.su);

						fflush(stderr);
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type61.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of set point command\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of set point command\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior SCADA
					    struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_SE_TA_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}
					    
					    item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
					    fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
					    /////////////////////////////////////////////////////////////////
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of set point command\n");
							fflush(stderr);
					break;
				}
			}
		}			
		break;
		case C_SE_TB_1: //set point command, scaled value with time
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type62 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
								
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type62.sv,
					obj[i].o.type62.ql,	obj[i].o.type62.se);
					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type62.sv,
					obj[i].o.type62.ql,	obj[i].o.type62.se);

					fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					obj[i].o.type62.time.hour,
					obj[i].o.type62.time.min,
					obj[i].o.type62.time.msec/1000,
					obj[i].o.type62.time.msec%1000,
					obj[i].o.type62.time.mday,
					obj[i].o.type62.time.month,
					obj[i].o.type62.time.year,
					obj[i].o.type62.time.iv,
					obj[i].o.type62.time.su);
					fflush(stderr);

					IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
					obj[i].o.type62.time.hour,
					obj[i].o.type62.time.min,
					obj[i].o.type62.time.msec/1000,
					obj[i].o.type62.time.msec%1000,
					obj[i].o.type62.time.mday,
					obj[i].o.type62.time.month,
					obj[i].o.type62.time.year,
					obj[i].o.type62.time.iv,
					obj[i].o.type62.time.su);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.ql =  obj[i].o.type62.ql;
					//answer_type.se = obj[i].o.type62.se;
					//answer_type.sv = obj[i].o.type62.sv;
					//answer_type.time = obj[i].o.type62.time;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_SE_TB_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type62) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type62));
				cp += sizeof(struct iec_type62);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type62.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif
					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field
						struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));

						item_to_send.iec_type = C_SE_TB_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;
						item_to_send.iec_obj.o.type62.time = obj[i].o.type62.time;
						item_to_send.iec_obj.o.type62.sv = obj[i].o.type62.sv;
						item_to_send.iec_obj.o.type62.ql = obj[i].o.type62.ql;
						item_to_send.iec_obj.o.type62.se = obj[i].o.type62.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
						fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);

						fprintf(stderr,"Execute the command with utc initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						item_to_send.iec_obj.o.type62.time.hour,
						item_to_send.iec_obj.o.type62.time.min,
						item_to_send.iec_obj.o.type62.time.msec/1000,
						item_to_send.iec_obj.o.type62.time.msec%1000,
						item_to_send.iec_obj.o.type62.time.mday,
						item_to_send.iec_obj.o.type62.time.month,
						item_to_send.iec_obj.o.type62.time.year,
						item_to_send.iec_obj.o.type62.time.iv,
						item_to_send.iec_obj.o.type62.time.su);

						fflush(stderr);
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type62.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of set point command\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of set point command\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior SCADA
					    struct iec_item item_to_send;
					    memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_SE_TB_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}
				    
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
					    fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
					    /////////////////////////////////////////////////////////////////
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of set point command\n");
							fflush(stderr);
					break;
				}
			}
		}
		break;
		case C_SE_TC_1: //set point command, short floating point with time
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//int j;
				//struct iec_unit_id answer_unit;
				//struct iec_type63 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
								
				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++) 
				{
					fprintf(stderr, "Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type63.sv,
					obj[i].o.type63.ql,	obj[i].o.type63.se);
					fflush(stderr);

					IT_COMMENT4("Value: IOA:%i sv:%d ql:%i se:%i\n",
					obj[i].ioa, obj[i].o.type63.sv,
					obj[i].o.type63.ql,	obj[i].o.type63.se);

					fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					obj[i].o.type63.time.hour,
					obj[i].o.type63.time.min,
					obj[i].o.type63.time.msec/1000,
					obj[i].o.type63.time.msec%1000,
					obj[i].o.type63.time.mday,
					obj[i].o.type63.time.month,
					obj[i].o.type63.time.year,
					obj[i].o.type63.time.iv,
					obj[i].o.type63.time.su);
					fflush(stderr);

					IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
					obj[i].o.type63.time.hour,
					obj[i].o.type63.time.min,
					obj[i].o.type63.time.msec/1000,
					obj[i].o.type63.time.msec%1000,
					obj[i].o.type63.time.mday,
					obj[i].o.type63.time.month,
					obj[i].o.type63.time.year,
					obj[i].o.type63.time.iv,
					obj[i].o.type63.time.su);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.ql =  obj[i].o.type63.ql;
					//answer_type.se = obj[i].o.type63.se;
					//answer_type.sv = obj[i].o.type63.sv;
					//answer_type.time = obj[i].o.type63.time;
				}

				//set the answer

				i = 0;

                /*
				answer_unit.type = C_SE_TC_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type63) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type63));
				cp += sizeof(struct iec_type63);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				if(obj[i].o.type63.se == 0)
				{
					#ifndef USE_SELECT_BEFORE_OPERATE
					cp56time2a sbo_time_arrival; //select before operate time arrival
					#endif
					cp56time2a command_arrive;
					get_utc_iec_time(&command_arrive);
					command_arrive_epoch = Epoch_from_cp56time2a(&command_arrive);

					//fprintf(stderr,"command_arrive_epoch = %lu\n", command_arrive_epoch);
					//fflush(stderr);

					#ifndef USE_SELECT_BEFORE_OPERATE
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);
					#endif
					
					delta = command_arrive_epoch - sbo_epoch;

					//fprintf(stderr,"delta = %lu\n", delta);
					//fflush(stderr);
					
					if(delta < MAX_TIME_AFTER_SBO_IN_SECONDS)
					{
						//Send command to field

						struct iec_item item_to_send;
						memset(&item_to_send,0x00, sizeof(struct iec_item));
						item_to_send.iec_type = C_SE_TC_1;
						item_to_send.iec_obj.ioa = obj[i].ioa;
						item_to_send.iec_obj.o.type63.time = obj[i].o.type63.time;
						item_to_send.iec_obj.o.type63.sv = obj[i].o.type63.sv;
						item_to_send.iec_obj.o.type63.ql = obj[i].o.type63.ql;
						item_to_send.iec_obj.o.type63.se = obj[i].o.type63.se;
						item_to_send.msg_id = msg_sent_in_control_direction++;
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
						fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

						//fprintf(stderr,"Command for %s from ioa %d\n",iec_items_table[j].opc_server_item_id, obj[i].ioa);
						//fflush(stderr);

						fprintf(stderr,"Command from ioa %d\n", obj[i].ioa);
						fflush(stderr);

						fprintf(stderr,"Execute the command with utc initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
						item_to_send.iec_obj.o.type63.time.hour,
						item_to_send.iec_obj.o.type63.time.min,
						item_to_send.iec_obj.o.type63.time.msec/1000,
						item_to_send.iec_obj.o.type63.time.msec%1000,
						item_to_send.iec_obj.o.type63.time.mday,
						item_to_send.iec_obj.o.type63.time.month,
						item_to_send.iec_obj.o.type63.time.year,
						item_to_send.iec_obj.o.type63.time.iv,
						item_to_send.iec_obj.o.type63.time.su);

						fflush(stderr);
					}
					else
					{
						char msg[100];
						u_short timeout = MAX_TIME_AFTER_SBO_IN_SECONDS;
						fprintf(stderr,"Command rejected. Command time arrive > %d s after Select Before Operate\n", timeout);
						fflush(stderr);

						sprintf(msg, "Command from IOA:%d rejected. Command time arrive > %d s after Select Before Operate\n", obj[i].ioa, timeout);
						log_message(msg);
					}
				}
				else if(obj[i].o.type63.se == 1)
				{
					//Select Before Operate
					//avvia un timer, se il timer supera il 60 s, il comando viene scartato

					///create an UTC time stamp at select arrival
					cp56time2a sbo_time_arrival; //select before operate time arrival
					get_utc_iec_time(&sbo_time_arrival);
					sbo_epoch = Epoch_from_cp56time2a(&sbo_time_arrival);

					fprintf(stderr,"Select Before Operate from ioa %d\n", obj[i].ioa);
					fflush(stderr);

					fprintf(stderr,"Select arrived at utc time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					sbo_time_arrival.hour,
					sbo_time_arrival.min,
					sbo_time_arrival.msec/1000,
					sbo_time_arrival.msec%1000,
					sbo_time_arrival.mday,
					sbo_time_arrival.month,
					sbo_time_arrival.year,
					sbo_time_arrival.iv,
					sbo_time_arrival.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of set point command\n");
							fflush(stderr);
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of set point command\n");
							fflush(stderr);
						}
					}
					break;
					case 0x0A: //activation termination
					{
						//Send activation termination to superior SCADA
						struct iec_item item_to_send;
						memset(&item_to_send,0x00, sizeof(struct iec_item));
					    item_to_send.iec_type = C_SE_TC_1;
                        item_to_send.cause = 0x0A;

                        i = 0;

                        item_to_send.iec_obj.ioa = obj[i].ioa;

						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 0; // In our implementation, this reserved value means that command is execute with success
						}
						else
						{
							fprintf(stderr,"Negative activation termination of set point command\n");
							fflush(stderr);

                            item_to_send.is_neg = 1; // This means negative termination
						}

						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
					    fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
					    /////////////////////////////////////////////////////////////////
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of set point command\n");
							fflush(stderr);
					break;
				}
			}
		}
		break;
		case C_BO_NA_1:
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Works without configuration database //
				//iec_items_table is NULL              //
				/////////////////////////////////////////

				//int len;
				//struct iec_unit_id answer_unit;
				//struct iec_type51 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
				//int j;

				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++)
				{
					fprintf(stderr, "Value: IOA:%i stcd:%u\n",
					obj[i].ioa,
					obj[i].o.type51.stcd);
					fflush(stderr);
					IT_COMMENT2("Value: IOA:%i stcd:%u",
					obj[i].ioa,
					obj[i].o.type51.stcd);

					ioa = obj[i].ioa; //ioa risposta

					//answer_type.stcd =  obj[i].o.type51.stcd;
				}
				
				i = 0;

				//prepara la activation confirmation

                /*
				answer_unit.type = C_BO_NA_1;
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type51) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type51));
				cp += sizeof(struct iec_type51);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */

				{
					//Send command to field (OPC client or iec101master)

					//La bufferizzazione dei comandi e' pericolosa.
	
					cp56time2a command_time_arrival;
					struct iec_item item_to_send;
					memset(&item_to_send,0x00, sizeof(struct iec_item));
					//union {
					//	struct iec_stcd stcd;
					//	float  commandValue;
					//	char command_string[4];
					//}v_stcd;

					item_to_send.iec_type = C_BO_NA_1;
					item_to_send.iec_obj.ioa = obj[i].ioa;

					//Add time stamp to command to check its life time
					///create an UTC time stamp at command arrival////////
					get_utc_iec_time(&command_time_arrival);
					item_to_send.iec_obj.o.type64.time = command_time_arrival;
					//////////////////////////////////////////////////////

					//e il valore da scrivere nell'item OPC lo prendo dal pacchetto appena arrivato dal MASTER
					item_to_send.iec_obj.o.type51.stcd = obj[i].o.type51.stcd;
					item_to_send.msg_id = msg_sent_in_control_direction++;
					item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
					fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

					fprintf(stderr,"Command from ioa %d value to write %u\n", obj[i].ioa, obj[i].o.type51.stcd);
					fflush(stderr);

					fprintf(stderr,"Execute the command with utc initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					item_to_send.iec_obj.o.type64.time.hour,
					item_to_send.iec_obj.o.type64.time.min,
					item_to_send.iec_obj.o.type64.time.msec/1000,
					item_to_send.iec_obj.o.type64.time.msec%1000,
					item_to_send.iec_obj.o.type64.time.mday,
					item_to_send.iec_obj.o.type64.time.month,
					item_to_send.iec_obj.o.type64.time.year,
					item_to_send.iec_obj.o.type64.time.iv,
					item_to_send.iec_obj.o.type64.time.su);

					fflush(stderr);
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//qui ricevo activation confirmation from slave

				if(cause == 0x07) //activation confirmation
				{
					if(pn == 0) //positive
					{
						fprintf(stderr,"Positive activation confirmation of C_BO_NA_1\n");
						fflush(stderr);
					}
					else
					{
						fprintf(stderr,"Negative activation confirmation of C_BO_NA_1\n");
						fflush(stderr);
					}
				}
			}
		}			
		break;
		case C_BO_TA_1:
		{
			if(s->type == IEC_SLAVE)
			{
				/////////////////////////////////////////
				//Funziona solo con database caricato  //
				//iec_items_table != NULL              //
				/////////////////////////////////////////

				//int len;
				//struct iec_unit_id answer_unit;
				//struct iec_type64 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;
				int j;

				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++)
				{
					fprintf(stderr, "Value: IOA:%i stcd:%u\n",
					obj[i].ioa,
					obj[i].o.type64.stcd);
					fflush(stderr);

					IT_COMMENT2("Value: IOA:%i stcd:%u",
					obj[i].ioa,
					obj[i].o.type64.stcd);

					fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					obj[i].o.type64.time.hour,
					obj[i].o.type64.time.min,
					obj[i].o.type64.time.msec/1000,
					obj[i].o.type64.time.msec%1000,
					obj[i].o.type64.time.mday,
					obj[i].o.type64.time.month,
					obj[i].o.type64.time.year,
					obj[i].o.type64.time.iv,
					obj[i].o.type64.time.su);
					fflush(stderr);

					IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
					obj[i].o.type64.time.hour,
					obj[i].o.type64.time.min,
					obj[i].o.type64.time.msec/1000,
					obj[i].o.type64.time.msec%1000,
					obj[i].o.type64.time.mday,
					obj[i].o.type64.time.month,
					obj[i].o.type64.time.year,
					obj[i].o.type64.time.iv,
					obj[i].o.type64.time.su);

					ioa = obj[i].ioa; //ioa risposta

					//answer_type.stcd =  obj[i].o.type64.stcd;
					//answer_type.time = obj[i].o.type64.time;
				}
				
				i = 0;

				//prepara la activation confirmation

                /*
				answer_unit.type = C_BO_TA_1;
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type64) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type64));
				cp += sizeof(struct iec_type64);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */
				
				//Execute the command
				//Execute the query:
				//select on the iec_item struct database
				//Given the ioa of the command, find with a loop
				//in the database the opc Item name to write.

				for(j = 0; j < num_iec_items; j++)
				{
					if(iec_items_table != NULL)
					{
						if(iec_items_table[j].iec_obj.ioa == obj[i].ioa)
						{
							//We EXECUTE the command:
							//Send a message to opc client to 
							//write the following opc item id
							//iec_items_table[j].opc_server_item_id
							
							//Send command to field (OPC client or iec101master)

							//La bufferizzazione dei comandi e' pericolosa.
					
							struct iec_item item_to_send;
							memcpy(&item_to_send, &iec_items_table[j], sizeof(struct iec_item));

							//sovrascrivo il tipo
							item_to_send.iec_type = C_BO_TA_1;
							item_to_send.iec_obj.ioa = obj[i].ioa;

							//e il valore da scrivere nell'item OPC lo prendo dal pacchetto appena arrivato dal MASTER
							item_to_send.iec_obj.o.type64.stcd = obj[i].o.type64.stcd;
							item_to_send.iec_obj.o.type64.time = obj[i].o.type64.time;
							item_to_send.msg_id = msg_sent_in_control_direction++;
							item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
							fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));

//							fprintf(stderr,"Command for %s from ioa %d value to write %u\n",iec_items_table[j].opc_server_item_id, obj[i].ioa, obj[i].o.type64.stcd);
//							fflush(stderr);

							fprintf(stderr,"Execute the command with initial time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
							obj[i].o.type64.time.hour,
							obj[i].o.type64.time.min,
							obj[i].o.type64.time.msec/1000,
							obj[i].o.type64.time.msec%1000,
							obj[i].o.type64.time.mday,
							obj[i].o.type64.time.month,
							obj[i].o.type64.time.year,
							obj[i].o.type64.time.iv,
							obj[i].o.type64.time.su);
							fflush(stderr);

							break;
						}
					}
				}
			}
			else if(s->type == IEC_MASTER)
			{
				//qui ricevo activation confirmation or activation termination from SLAVE

				if(cause == 0x07) //activation confirmation
				{
					if(pn == 0) //positive
					{
						fprintf(stderr,"Positive activation confirmation of C_BO_TA_1\n");
						fflush(stderr);
					}
					else
					{
						fprintf(stderr,"Negative activation confirmation of C_BO_TA_1\n");
						fflush(stderr);
					}
				}

				if(cause == 0x0A) //activation termination
				{
					if(pn == 0) //positive
					{
						fprintf(stderr,"Positive activation termination of C_BO_TA_1\n");
						fflush(stderr);

						if(n_ioa_in_asdu != 1)
						{
							fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
							fflush(stderr);
							#ifdef WIN32
							ExitProcess(0);
                            #else
							exit(EXIT_FAILURE);
                            #endif
						}

						for (i = 0; i < n_ioa_in_asdu; i++)
						{
							fprintf(stderr, "Value: IOA:%i stcd:%u\n",
							obj[i].ioa,
							obj[i].o.type64.stcd);
							fflush(stderr);

							IT_COMMENT2("Value: IOA:%i stcd:%u",
							obj[i].ioa,
							obj[i].o.type64.stcd);

							fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
							obj[i].o.type64.time.hour,
							obj[i].o.type64.time.min,
							obj[i].o.type64.time.msec/1000,
							obj[i].o.type64.time.msec%1000,
							obj[i].o.type64.time.mday,
							obj[i].o.type64.time.month,
							obj[i].o.type64.time.year,
							obj[i].o.type64.time.iv,
							obj[i].o.type64.time.su);
							fflush(stderr);

							IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
							obj[i].o.type64.time.hour,
							obj[i].o.type64.time.min,
							obj[i].o.type64.time.msec/1000,
							obj[i].o.type64.time.msec%1000,
							obj[i].o.type64.time.mday,
							obj[i].o.type64.time.month,
							obj[i].o.type64.time.year,
							obj[i].o.type64.time.iv,
							obj[i].o.type64.time.su);
						}
						
						i = 0; //DO NOT REMOVE!
										
						{
							
							struct iec_item item_to_send;
							memset(&item_to_send,0x00, sizeof(struct iec_item));
							item_to_send.iec_type = C_BO_TA_1;
							item_to_send.iec_obj.ioa = obj[i].ioa; //ioa risposta
							item_to_send.iec_obj.o.type64 = obj[i].o.type64;
							item_to_send.msg_id = msg_sent_to_superior_scada++;
							item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));							
							fifo_put(fifo_monitor_direction, (char *)&item_to_send, sizeof(struct iec_item));
							//////////////////////////////////////////////////////////////////////////////
						}
					}
					else
					{
						fprintf(stderr,"Negative activation termination of C_BO_TA_1\n");
						fflush(stderr);
					}
				}
			}
		}			
		break;
		case C_IC_NA_1://general interrogation command
		{
			if(s->type == IEC_SLAVE)
			{
				/*
				//Send GI activation confirmation to MASTER

				int len;
				struct iec_unit_id answer_unit;
				struct iec_type100 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				u_char *cp;

				assert(n_ioa_in_asdu == 1);

				for (i = 0; i < n_ioa_in_asdu; i++)
				{
					fprintf(stderr, "Value: IOA:%i qoi:%i\n",
					obj[i].ioa,
					obj[i].o.type100.qoi);
					fflush(stderr);

					IT_COMMENT2("Value: IOA:%i qoi:0x%02x\n",
					obj[i].ioa,
					obj[i].o.type100.qoi);

					ioa = obj[i].ioa; //ioa risposta
					answer_type.qoi =  obj[i].o.type100.qoi;
				}

				//C_IC_NA_1 confirmation
				//set the answer

				answer_unit.type = C_IC_NA_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type100) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;
				
				answer_unit.cause = 0x07; //activation confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type100));
				cp += sizeof(struct iec_type100);
										
				iecsock_prepare_iframe(c);
				
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
				*/

				state_of_slave = SLAVE_SENDING_DATA; //Lets the SLAVE events in write queues to go to MASTER before GI data

				state_general_interrogation = GI_REQUEST_GI_TO_LOWER_LEVER_SCADA;
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel Master, qui riceviamo le conferme o terminazioni dallo slave
				switch(cause)
				{
					case 0x07://activation confirmation
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation confirmation of general interrogation\n");
							fflush(stderr);

							state_general_interrogation = GI_CONFIRMATION;
						}
						else
						{
							fprintf(stderr,"Negative activation confirmation of general interrogation\n");
							fflush(stderr);

							state_general_interrogation = GI_NEG_CONFIRMATION;
						}
					}
					break;
					case 0x0A: //activation termination
					{
						if(pn == 0) //positive
						{
							fprintf(stderr,"Positive activation termination of general interrogation\n");
							fflush(stderr);

							state_general_interrogation = GI_TERMINATION;
						}
						else
						{
							fprintf(stderr,"Negative activation termination of general interrogation\n");
							fflush(stderr);

							state_general_interrogation = GI_NEG_TERMINATION;
						}
					}
					break;
					default:
							fprintf(stderr,"Unknown cause of general interrogation\n");
							fflush(stderr);
					break;
				}
			}
		}			
		break;
		case C_CS_NA_1://clock synchronisation command
		{
			if(s->type == IEC_SLAVE)
			{
				//int len;
				//struct iec_unit_id answer_unit;
				//struct iec_type103 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;

				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
				    exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++)
				{
					fprintf(stderr, "Value: IOA:%i\n", obj[i].ioa);
					fflush(stderr);

					IT_COMMENT1("Value: IOA:%i", obj[i].ioa);

					fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					obj[i].o.type103.time.hour,
					obj[i].o.type103.time.min,
					obj[i].o.type103.time.msec/1000,
					obj[i].o.type103.time.msec%1000,
					obj[i].o.type103.time.mday,
					obj[i].o.type103.time.month,
					obj[i].o.type103.time.year,
					obj[i].o.type103.time.iv,
					obj[i].o.type103.time.su);
					fflush(stderr);

					IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
					obj[i].o.type103.time.hour,
					obj[i].o.type103.time.min,
					obj[i].o.type103.time.msec/1000,
					obj[i].o.type103.time.msec%1000,
					obj[i].o.type103.time.mday,
					obj[i].o.type103.time.month,
					obj[i].o.type103.time.year,
					obj[i].o.type103.time.iv,
					obj[i].o.type103.time.su);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.time =  obj[i].o.type103.time;
				}

				i = 0;

				//Imposta orario locale
				if(set_iec_time(&(obj[i].o.type103.time)))
				{
					//Errore
					//answer_unit.pn = 1; //negative

					state_clock_synchronisation = CLK_IDLE;
				}
				else
				{
					//answer_unit.pn = 0; //positive

					state_clock_synchronisation = CLK_STATIONS_SYNCRONIZED;
				}

				//prepara la activation confirmation

                /*
				answer_unit.type = C_CS_NA_1;
				answer_unit.num = 1;
				answer_unit.sq = 0;
				//answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type103) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type103));
				cp += sizeof(struct iec_type103);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                */
                
                if(s->type == IEC_SLAVE)
                {
                    //Send clock synchronization command to field (OPC client or iec101master)
					struct iec_item item_to_send;
					memset(&item_to_send,0x00, sizeof(struct iec_item));
				    item_to_send.iec_type = C_CS_NA_1;
				    item_to_send.iec_obj.ioa = obj[i].ioa;
				    item_to_send.iec_obj.o.type103.time = obj[i].o.type103.time;
				    item_to_send.msg_id = msg_sent_in_control_direction++;
				    item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
				    fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));
                }
			}
			else if(s->type == IEC_MASTER)
			{
				//qui ricevo activation confirmation from slave

				if(cause == 0x07) //activation confirmation
				{
					if(pn == 0) //positive
					{
						state_clock_synchronisation = CLK_STATIONS_SYNCRONIZED;

						fprintf(stderr,"Positive activation confirmation of clock synchronization\n");
						fflush(stderr);
					}
					else
					{
						state_clock_synchronisation = CLK_IDLE;

						fprintf(stderr,"Negative activation confirmation of clock synchronization\n");
						fflush(stderr);
					}
				}

				if(cause == 0x03) //Spontaneous
				{
					if(pn == 0) //positive
					{
						//state_clock_synchronisation = CLK_STATIONS_SYNCRONIZED;

						fprintf(stderr,"Positive spontaneous of clock synchronization\n");
						fflush(stderr);
					}
					else
					{
						//state_clock_synchronisation = CLK_IDLE;

						fprintf(stderr,"Negative spontaneous of clock synchronization\n");
						fflush(stderr);
					}
				}
			}
		}			
		break;
		case C_RP_NA_1: //Reset process command
		{
			if(cause == 0x06) //activation
			{
				//int len;
				//struct iec_unit_id answer_unit;
				//struct iec_type105 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;

				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++)
				{
					fprintf(stderr, "Value: IOA:%i qrp:%i\n",
					obj[i].ioa,
					obj[i].o.type105.qrp);
					fflush(stderr);

					IT_COMMENT2("Value: IOA:%i qrp:0x%02x\n",
					obj[i].ioa,
					obj[i].o.type105.qrp);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.qrp =  obj[i].o.type105.qrp;
				}

                /*
				answer_unit.type = 105; //C_RP_NA_1
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type105) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
		
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type105));
				cp += sizeof(struct iec_type105);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
				////////////////////////////////////////////////////////
                */
							
				if(s->type == IEC_MASTER)
				{
					//Free the queues di scrittura
				    iecsock_flush_queues(s);

					//Reload config database disabled for IndigoSCADA
					//state_database = LOAD_DATABASE;
				}
                else if(s->type == IEC_SLAVE)
                {
                    //Send Reset process command to field (OPC client or iec101master)
				    struct iec_item item_to_send;
					memset(&item_to_send,0x00, sizeof(struct iec_item));
				    item_to_send.iec_type = C_RP_NA_1;
				    item_to_send.iec_obj.ioa = obj[i].ioa;
				    item_to_send.iec_obj.o.type105.qrp = obj[i].o.type105.qrp;
				    item_to_send.msg_id = msg_sent_in_control_direction++;
				    item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
				    fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));
                }
			}
			else if(cause == 0x07) //activation confirmation
			{
				if(pn == 0) //positive
				{
					fprintf(stderr,"Positive activation confirmation of reset process command\n");
					fflush(stderr);
				}
				else
				{
					fprintf(stderr,"Negative activation confirmation of reset process command\n");
					fflush(stderr);
				}
			}
		}
		break;
		case C_CD_NA_1: 
		{
			if(cause == 0x06) //activation
			{
				//int len;
				//struct iec_unit_id answer_unit;
				//struct iec_type106 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				//u_char *cp;

				if(n_ioa_in_asdu != 1)
				{
					fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				for (i = 0; i < n_ioa_in_asdu; i++)
				{
					fprintf(stderr, "Value: IOA:%i delay:%u\n",
					obj[i].ioa,
					obj[i].o.type106.two_oct_time);
					fflush(stderr);

					IT_COMMENT2("Value: IOA:%i delay:%u\n",
					obj[i].ioa,
					obj[i].o.type106.two_oct_time);

					ioa = obj[i].ioa; //ioa risposta
					//answer_type.two_oct_time =  obj[i].o.type106.two_oct_time;
				}

                /*
				answer_unit.type = C_CD_NA_1;
				answer_unit.num = n_ioa_in_asdu;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type106) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
		
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
							
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type106));
				cp += sizeof(struct iec_type106);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
				//////////////////////////////////////////////////////////////////////////////////////////////////
                */

                if(s->type == IEC_SLAVE)
                {
                    //Send Delay acquisition command to field (OPC client or iec101master)
				    struct iec_item item_to_send;
					memset(&item_to_send,0x00, sizeof(struct iec_item));
				    item_to_send.iec_type = C_CD_NA_1;
				    item_to_send.iec_obj.ioa = obj[i].ioa;
				    item_to_send.iec_obj.o.type106.two_oct_time = obj[i].o.type106.two_oct_time;
				    item_to_send.msg_id = msg_sent_in_control_direction++;
					item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
				    fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));
                }
			}
			else if(cause == 0x07) //activation confirmation
			{
				if(pn == 0) //positive
				{
					fprintf(stderr,"Positive activation confirmation of C_CD_NA_1\n");
					fflush(stderr);
				}
				else
				{
					fprintf(stderr,"Negative activation confirmation of C_CD_NA_1\n");
					fflush(stderr);
				}
			}
		}
		break;
		case F_FR_NA_1: //file ready
		{
			int len;
			struct iec_unit_id answer_unit;
			struct iec_type122 answer_type;
			u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
			u_char *cp;

			for (i = 0; i < n_ioa_in_asdu; i++)
			{
				fprintf(stderr, "Value: IOA:%i lof:%i nof:%i frq_ui7:%i frq_bs1:%i\n",
				obj[i].ioa,
				obj[i].o.type120.lof,
				obj[i].o.type120.nof,
				obj[i].o.type120.frq_ui7,
				obj[i].o.type120.frq_bs1);
				fflush(stderr);

				IT_COMMENT5("Value: IOA:%i lof:%i nof:%i frq_ui7:%i frq_bs1:%i",
				obj[i].ioa,
				obj[i].o.type120.lof,
				obj[i].o.type120.nof,
				obj[i].o.type120.frq_ui7,
				obj[i].o.type120.frq_bs1);
			}

			//initialise file checksum
			file_checksum = 0;
			
			i = 0;
			ioa = obj[i].ioa; //ioa risposta
			answer_type.nof = obj[i].o.type120.nof;
			answer_type.nos = 1; //name of section
			answer_type.scq_ui4low = 2; //request file
			answer_type.scq_ui4high = 0;

			answer_unit.type = F_SC_NA_1; //F_SC_NA_1
			answer_unit.num = 1;
			answer_unit.sq = 0;
			answer_unit.pn = 0; //positive
			answer_unit.ca = caddr;
			answer_unit.t = 0;
			answer_unit.originator = 0;
						
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type122) + IOA_ADDRLEN) * answer_unit.num);

			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}

			get_iec_buf(&c);
			
			c->data_len = len;

			cp = c->data;
			
			answer_unit.cause = 0x0D; //file transfer

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;
						
			memcpy(cp, &ioa, IOA_ADDRLEN);
			cp += IOA_ADDRLEN;
			memcpy(cp, &answer_type, sizeof(struct iec_type122));
			cp += sizeof(struct iec_type122);
								
			iecsock_prepare_iframe(c);
			TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			//////////////////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case F_SR_NA_1: //Section ready
		{
			int len;
			struct iec_unit_id answer_unit;
			struct iec_type122 answer_type;
			u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
			u_char *cp;

			for (i = 0; i < n_ioa_in_asdu; i++)
			{
				fprintf(stderr, "Value: IOA:%i lof:%i nof:%i nos:%i srq_ui7:%i srq_bs1:%i\n",
				obj[i].ioa,
				obj[i].o.type121.lof,
				obj[i].o.type121.nof,
				obj[i].o.type121.nos,
				obj[i].o.type121.srq_ui7,
				obj[i].o.type121.srq_bs1);
				fflush(stderr);

				IT_COMMENT6("Value: IOA:%i lof:%i nof:%i nos:%i srq_ui7:%i srq_bs1:%i",
				obj[i].ioa,
				obj[i].o.type121.lof,
				obj[i].o.type121.nof,
				obj[i].o.type121.nos,
				obj[i].o.type121.srq_ui7,
				obj[i].o.type121.srq_bs1);
			}

			//initialise section checksum
			section_checksum = 0;
			
			i = 0;
			ioa = obj[i].ioa; //ioa risposta
			answer_type.nof = obj[i].o.type121.nof;
			answer_type.nos = obj[i].o.type121.nos; //name of section
			answer_type.scq_ui4low = 6; //request section
			answer_type.scq_ui4high = 0;

			answer_unit.type = 122; //F_SC_NA_1
			answer_unit.num = 1;
			answer_unit.sq = 0;
			answer_unit.pn = 0; //positive
			answer_unit.ca = caddr;
			answer_unit.t = 0;
			answer_unit.originator = 0;
						
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type122) + IOA_ADDRLEN) * answer_unit.num);

			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}

			get_iec_buf(&c);
			
			c->data_len = len;

			cp = c->data;
			
			answer_unit.cause = 0x0D; //file transfer

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;
						
			memcpy(cp, &ioa, IOA_ADDRLEN);
			cp += IOA_ADDRLEN;
			memcpy(cp, &answer_type, sizeof(struct iec_type122));
			cp += sizeof(struct iec_type122);
								
			iecsock_prepare_iframe(c);
			TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			//////////////////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case F_SC_NA_1: //Call directory, select file, call file, call section
		{
			//siamo nel file sender side
			int len;
			struct iec_unit_id answer_unit;
			u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
			u_char *cp;
						
			for (i = 0; i < n_ioa_in_asdu; i++)
			{
				fprintf(stderr, "Value: IOA:%i nof:%i nos:%i scq_ui4low:%i scq_ui4high:%i\n",
				obj[i].ioa,
				obj[i].o.type122.nof,
				obj[i].o.type122.nos,
				obj[i].o.type122.scq_ui4low,
				obj[i].o.type122.scq_ui4high);
				fflush(stderr);

				IT_COMMENT5("Value: IOA:%i nof:%i nos:%i scq_ui4low:%i scq_ui4high:%i",
				obj[i].ioa,
				obj[i].o.type122.nof,
				obj[i].o.type122.nos,
				obj[i].o.type122.scq_ui4low,
				obj[i].o.type122.scq_ui4high);
			}

			i = 0;

			if(pn == 0)//positive
			{
				if(obj[i].o.type122.scq_ui4low == 0x01) //select file
				{
					struct iec_type120 answer_type;
					ioa = obj[i].ioa; //ioa risposta
					answer_type.nof = obj[i].o.type122.nof;
					answer_type.lof = length_of_file;
					answer_type.frq_ui7 = 0; //default
					answer_type.frq_bs1 = 0; //positive confirm of select, request, deactivate or delete
					
					answer_unit.type = 120; //F_FR_NA_1
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = caddr;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type120) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
				
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x0D; //file transfer

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type120));
					cp += sizeof(struct iec_type120);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
					//////////////////////////////////////////////////////////////////////////////////////////////////
				}
				else if(obj[i].o.type122.scq_ui4low == 0x02) //request file
				{
					struct iec_type121 answer_type;
					ioa = obj[i].ioa; //ioa risposta
					answer_type.nof = obj[i].o.type122.nof;
					answer_type.lof = length_of_file;
					answer_type.nos = obj[i].o.type122.nos;
					answer_type.srq_ui7 = 0; //default
					answer_type.srq_bs1 = 0; //section ready for load
									
					answer_unit.type = F_SR_NA_1; //
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = caddr;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type121) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
				
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x0D; //file transfer

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type121));
					cp += sizeof(struct iec_type121);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
					//////////////////////////////////////////////////////////////////////////////////////////////////
				}
				else if(obj[i].o.type122.scq_ui4low == 0x06) //request section
				{
					//Prepare segment to send
					struct iec_type125 answer_type;
					int j,k;
					FILE *fr = NULL;
					double  count = 0.0, perc_file_read;

					fprintf(stderr,"Prepare segments to send\n");
					fflush(stderr);

					file_checksum = 0;
					section_checksum = 0;

					if(strlen(file_da_trasferire[index_of_file_to_transfer]) == 0)
					{
						fprintf(stderr,"Bad file name to transfer\n");
						fflush(stderr);
						break;
					}
					
					fr = fopen(file_da_trasferire[index_of_file_to_transfer],"rb");

					if(fr == NULL)
					{
						fprintf(stderr,"Failed to open file %s at line %d %s\n", file_da_trasferire[index_of_file_to_transfer], __LINE__, __FILE__);
						fflush(stderr);
						state_file_transfer = FT_ERROR;
						break;
					}

					fprintf(stderr,"Open file %s\n"
					"number_of_segments = %d\n"
					"length_of_segment = %d\n"
					"resto_dei_byte_da_leggere= %d\n", 
					file_da_trasferire[index_of_file_to_transfer], number_of_segments, length_of_segment, resto_dei_byte_da_leggere);
					fflush(stderr);
						
					for (j = 0; j < number_of_segments; j++)
					{
						ioa = obj[i].ioa; //ioa risposta
						answer_type.nof = obj[i].o.type122.nof;
						answer_type.nos = obj[i].o.type122.nos;
						answer_type.los = length_of_segment;
										
						answer_unit.type = 125; //
						answer_unit.num = 1;
						answer_unit.sq = 0;
						answer_unit.pn = 0; //positive
						answer_unit.ca = caddr;
						answer_unit.t = 0;
						answer_unit.originator = 0;
									
						len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type125) + IOA_ADDRLEN) * answer_unit.num) + answer_type.los;

						if (len > IEC104_ASDU_MAX)
						{
							fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
							fflush(stderr);
							#ifdef WIN32
							ExitProcess(0);
                            #else
							exit(EXIT_FAILURE);
                            #endif
						}

						get_iec_buf(&c);
				
						c->data_len = len;

						cp = c->data;
						
						answer_unit.cause = 0x0D; //file transfer

						memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
						cp += IEC_TYPEID_LEN + COM_ADDRLEN;
												
						memcpy(cp, &ioa, IOA_ADDRLEN);
						cp += IOA_ADDRLEN;
						memcpy(cp, &answer_type, sizeof(struct iec_type125));
						cp += sizeof(struct iec_type125);
						
						//Append segment
						//segment = c->data + COM_ADDRLEN + IEC_TYPEID_LEN + IOA_ADDRLEN + sizeof(struct iec_type125);
						for(k = 0;k < length_of_segment; k++)
						{
							//cp[k] = k%16;
							int readb = fread(cp + k, sizeof(unsigned char), 1,fr);

							count = count + readb;

							if(ferror(fr))      
							{
								fprintf(stderr,"Error in reading file %s\n", file_da_trasferire[index_of_file_to_transfer]);
								fflush(stderr);
								state_file_transfer = FT_ERROR;
								break;
							}

							file_checksum = file_checksum + cp[k];
							section_checksum = section_checksum + cp[k];
						}
								
						iecsock_prepare_iframe(c);
						TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
						//////////////////////////////////////////////////////////////////////////////////////////////////

						perc_file_read = 100.0*count/(double)length_of_file;
						fprintf(stderr,"Reading %.2lf %%\r", perc_file_read);
						fflush(stderr);
					}

					if(resto_dei_byte_da_leggere != 0)
					{
						//Invio i bytes rimanenti

						ioa = obj[i].ioa; //ioa risposta
						answer_type.nof = obj[i].o.type122.nof;
						answer_type.nos = obj[i].o.type122.nos;
						answer_type.los = resto_dei_byte_da_leggere;
										
						answer_unit.type = 125; //
						answer_unit.num = 1;
						answer_unit.sq = 0;
						answer_unit.pn = 0; //positive
						answer_unit.ca = caddr;
						answer_unit.t = 0;
						answer_unit.originator = 0;
									
						len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type125) + IOA_ADDRLEN) * answer_unit.num) + answer_type.los;

						if (len > IEC104_ASDU_MAX)
						{
							fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
							fflush(stderr);
							#ifdef WIN32
							ExitProcess(0);
                            #else
							exit(EXIT_FAILURE);
                            #endif
						}

						get_iec_buf(&c);
				
						c->data_len = len;

						cp = c->data;
						
						answer_unit.cause = 0x0D; //file transfer

						memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
						cp += IEC_TYPEID_LEN + COM_ADDRLEN;
												
						memcpy(cp, &ioa, IOA_ADDRLEN);
						cp += IOA_ADDRLEN;
						memcpy(cp, &answer_type, sizeof(struct iec_type125));
						cp += sizeof(struct iec_type125);
						
						//Append remaing bytes
						
						for(k = 0;k < resto_dei_byte_da_leggere; k++)
						{
							int readb = fread(cp + k, sizeof(unsigned char), 1,fr);

							count = count + readb;

							if(ferror(fr))      
							{
								fprintf(stderr,"Error in reading file %s\n", file_da_trasferire[index_of_file_to_transfer]);
								fflush(stderr);
								state_file_transfer = FT_ERROR;
								break;
							}

							file_checksum = file_checksum + cp[k];
							section_checksum = section_checksum + cp[k];
						}
								
						iecsock_prepare_iframe(c);
						TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
						//////////////////////////////////////////////////////////////////////////////////////////////////

						perc_file_read = 100.0*count/(double)length_of_file;
						fprintf(stderr,"Reading %.2lf %%\r", perc_file_read);
						fflush(stderr);
					}

					fclose(fr);

					fprintf(stderr,"\n");
					fflush(stderr);

					fprintf(stderr,"Segments loaded\n");
					fflush(stderr);

					{
						//Send last segment asdu
						struct iec_type123 answer_type;
						i = 0;
						ioa = obj[i].ioa; //ioa risposta
						answer_type.nof = obj[i].o.type122.nof;
						answer_type.nos = obj[i].o.type122.nos;
						answer_type.chs = section_checksum%256;
						answer_type.lsq = 0x03; //section transfer without deactivation
						
						answer_unit.type = 123; //F_FR_NA_1
						answer_unit.num = 1;
						answer_unit.sq = 0;
						answer_unit.pn = 0; //positive
						answer_unit.ca = caddr;
						answer_unit.t = 0;
						answer_unit.originator = 0;
									
						len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type123) + IOA_ADDRLEN) * answer_unit.num);

						if (len > IEC104_ASDU_MAX)
						{
							fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
							fflush(stderr);
							#ifdef WIN32
							ExitProcess(0);
                            #else
							exit(EXIT_FAILURE);
                            #endif
						}

						get_iec_buf(&c);
				
						c->data_len = len;

						cp = c->data;
						
						answer_unit.cause = 0x0D; //file transfer

						memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
						cp += IEC_TYPEID_LEN + COM_ADDRLEN;
												
						memcpy(cp, &ioa, IOA_ADDRLEN);
						cp += IOA_ADDRLEN;
						memcpy(cp, &answer_type, sizeof(struct iec_type123));
						cp += sizeof(struct iec_type123);
														
						iecsock_prepare_iframe(c);
						TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

						fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i chs=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn, answer_type.chs);
						fflush(stderr);

						IT_COMMENT7("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i chs=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn, answer_type.chs);
						//////////////////////////////////////////////////////////////////////////////////////////////////
					}
				}
			}
			else
			{
				fprintf(stderr,"Receiving negative F_SC_NA_1, we are the sending side and the other side (the receiving side) is busy in sending another file\n"); //30-06-2010
				fflush(stderr);
				//So our file transfer goes idle //apa+++ 11-12-2010
				state_file_transfer = FT_IDLE; //apa+++ 11-12-2010
			}
		}
		break;
		case F_LS_NA_1: //Last section (i.e. end of file), last segment (i.e end of section)
		{
			int len;
			struct iec_unit_id answer_unit;
			struct iec_type124 answer_type;
			u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
			u_char *cp;
			int checksum_is_ok = 1;
			u_int remote_checksum;

			for (i = 0; i < n_ioa_in_asdu; i++)
			{
				fprintf(stderr, "Value: IOA:%i nof:%i nos:%i chs:%i lsq:%i\n",
				obj[i].ioa,
				obj[i].o.type123.nof,
				obj[i].o.type123.nos,
				obj[i].o.type123.chs,
				obj[i].o.type123.lsq);
				fflush(stderr);

				IT_COMMENT5("Value: IOA:%i nof:%i nos:%i chs:%i lsq:%i",
				obj[i].ioa,
				obj[i].o.type123.nof,
				obj[i].o.type123.nos,
				obj[i].o.type123.chs,
				obj[i].o.type123.lsq);
			}

			i = 0;

			if(obj[i].o.type123.lsq == 0x03) //last segment or section transfer without deactivation
			{
				//Test checksum
				remote_checksum = obj[i].o.type123.chs; //Section checksum
				section_checksum = section_checksum%256;
				if(remote_checksum != section_checksum)
				{
					checksum_is_ok = 0;
					fprintf(stderr, "Section checksum error\n");
					fflush(stderr);
				}

				ioa = obj[i].ioa; //ioa risposta
				answer_type.nof = obj[i].o.type123.nof;
				answer_type.nos = obj[i].o.type123.nos; //name of section
				if(checksum_is_ok)
				{
					answer_type.afq_ui4low = 3; //positive acknowledge of section transfer
					answer_type.afq_ui4high = 0; //default
				}
				else
				{
					answer_type.afq_ui4low = 4; //negative acknowledge of section transfer
					answer_type.afq_ui4high = 2; //checksum failed
				}

				answer_unit.type = 124; //F_AF_NA_1
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type124) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;
				
				answer_unit.cause = 0x0D; //file transfer

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type124));
				cp += sizeof(struct iec_type124);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
				//////////////////////////////////////////////////////////////////////////////////////////////////
			}
			else if(obj[i].o.type123.lsq == 0x01) //last section or file transfer without deactivation
			{
				//Test checksum
				remote_checksum = obj[i].o.type123.chs; //File checksum
				file_checksum = file_checksum%256;

				if(remote_checksum != file_checksum)
				{
					checksum_is_ok = 0;
					fprintf(stderr, "File checksum error\n");
					fflush(stderr);
				}

				ioa = obj[i].ioa; //ioa risposta
				answer_type.nof = obj[i].o.type123.nof;
				answer_type.nos = obj[i].o.type123.nos; //name of section
				if(checksum_is_ok)
				{
					answer_type.afq_ui4low = 1; //positive acknowledge of file transfer
					answer_type.afq_ui4high = 0; //default
					
					fprintf(stderr, "File transfer ended on receiver side\n");
					fflush(stderr);

					state_file_transfer = FT_REMOVE_CONFIGURATION_FILE;
				}
				else
				{
					answer_type.afq_ui4low = 2; //negative acknowledge of file transfer
					answer_type.afq_ui4high = 2; //checksum failed
				}
				
				answer_unit.type = 124; //F_AF_NA_1
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type124) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;
				
				answer_unit.cause = 0x0D; //file transfer

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type124));
				cp += sizeof(struct iec_type124);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
				//////////////////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
		case F_AF_NA_1: //ACK file, ACK section
		{
			int len;
			struct iec_unit_id answer_unit;
			struct iec_type123 answer_type;
			u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
			u_char *cp;
			
			for (i = 0; i < n_ioa_in_asdu; i++)
			{
				fprintf(stderr, "Value: IOA:%i nof:%i nos:%i afq_ui4low:%i afq_ui4high:%i\n",
				obj[i].ioa,
				obj[i].o.type124.nof,
				obj[i].o.type124.nos,
				obj[i].o.type124.afq_ui4low,
				obj[i].o.type124.afq_ui4high);
				fflush(stderr);

				IT_COMMENT5("Value: IOA:%i nof:%i nos:%i afq_ui4low:%i afq_ui4high:%i\n",
				obj[i].ioa,
				obj[i].o.type124.nof,
				obj[i].o.type124.nos,
				obj[i].o.type124.afq_ui4low,
				obj[i].o.type124.afq_ui4high);
			}

			i = 0;

			if(obj[i].o.type124.afq_ui4low == 0x03)
			{
				//Last section or positive acknowledge of section transfer
				
				ioa = obj[i].ioa; //ioa risposta
				answer_type.nof = obj[i].o.type124.nof;
				answer_type.nos = obj[i].o.type124.nos;
				answer_type.chs = file_checksum%256;
				answer_type.lsq = 0x01; //file transfer without deactivation
				
				answer_unit.type = 123; //F_FR_NA_1
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = caddr;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type123) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;
				
				answer_unit.cause = 0x0D; //file transfer

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type123));
				cp += sizeof(struct iec_type123);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
				//////////////////////////////////////////////////////////////////////////////////////////////////
			}
			else if(obj[i].o.type124.afq_ui4low == 0x01)
			{
				//Positive acknowledge of file transfer
				//File transfer ended
				fprintf(stderr, "File transfer ended on sender side\n");
				fflush(stderr);

				fprintf(stderr,"index_of_file_to_transfer = %d\n", index_of_file_to_transfer);
				fflush(stderr);
				fprintf(stderr,"num_of_file_loaded = %d\n", num_of_file_loaded);
				fflush(stderr);

				state_file_transfer = FT_REMOVE_TRANSFERED_FILE;
			}
			else if(obj[i].o.type124.afq_ui4low == 0x02)
			{
				//negative acknowledge of file transfer
				if(obj[i].o.type124.afq_ui4high == 0x02)
				{
					fprintf(stderr, "File checksum error\n");
					fflush(stderr);

					//Torna ad inviare lo stesso file puntato da index_of_file_to_transfer
					state_file_transfer = FT_PREPARE_DIRECTORY;
				}
			}
			else if(obj[i].o.type124.afq_ui4low == 0x04)
			{
				//negative acknowledge of section transfer
				if(obj[i].o.type124.afq_ui4high == 0x02)
				{
					fprintf(stderr, "Section checksum error\n");
					fflush(stderr);

					//Torna ad inviare lo stesso file puntato da index_of_file_to_transfer
					state_file_transfer = FT_PREPARE_DIRECTORY;
				}
			}
		}
		break;
		case F_SG_NA_1: //Segment
		{
			int j;
			u_char *segment;
			FILE *ft = NULL;
			u_char ch;

			if(strlen(file_trasferito) == 0)
			{
			    fprintf(stderr,"File name of transfered file is null\n");
				fflush(stderr);
				break;
			}

			ft = fopen(file_trasferito,"ab");

		    if(ft == NULL)
			{
			    fprintf(stderr,"Failed to open file %s\n",file_trasferito);
				fflush(stderr);
				state_file_transfer = FT_ERROR;
				break;
			}

			if(n_ioa_in_asdu != 1)
			{
				fprintf(stderr,"n_ioa_in_asdu != 1 at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}

			for (i = 0; i < n_ioa_in_asdu; i++)
			{
				fprintf(stderr, "Value: IOA:%i los:%i nof:%i nos:%i\n",
				obj[i].ioa,
				obj[i].o.type125.los,
				obj[i].o.type125.nof,
				obj[i].o.type125.nos);
				fflush(stderr);

				IT_COMMENT4("Value: IOA:%i los:%i nof:%i nos:%i\n",
				obj[i].ioa,
				obj[i].o.type125.los,
				obj[i].o.type125.nof,
				obj[i].o.type125.nos);

				//offset_segment = COM_ADDRLEN + IEC_TYPEID_LEN + IOA_ADDRLEN + sizeof(struct iec_type125);
				segment = b->data + COM_ADDRLEN + IEC_TYPEID_LEN + IOA_ADDRLEN + sizeof(struct iec_type125);

				length_of_segment = obj[i].o.type125.los;

				//inizio prova
				//for(j = 0;j < b->data_len; j++)
				//{
				//	unsigned char c = *((unsigned char*)b->data + j);
				//	fprintf(stderr,"0x%02x-\n", c);
				//	fflush(stderr);
				//	IT_COMMENT1("0x%02x", c);
				//}
				//fine prova
				
				for(j = 0;j < length_of_segment; j++)
				{
					ch = segment[j];
					file_checksum = file_checksum + ch;
					section_checksum = section_checksum + ch;
					fwrite(&segment[j] ,sizeof(unsigned char),1,ft);

					if(ferror(ft))
					{
						fprintf(stderr,"Error in writing file %s\n", file_trasferito);
						fflush(stderr);
						state_file_transfer = FT_ERROR;
						break;
					}

					//fprintf(ft,"%02x", ch);
					fflush(ft);
				}
			}
		
			fclose(ft);
		}
		break;
		case F_DR_TA_1: //File transfer directory
		{
			//Siamo nel file receiver side
			int len;
			struct iec_unit_id answer_unit;
			struct iec_type122 answer_type;
			u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
			u_char *cp;

			for (i = 0; i < n_ioa_in_asdu; i++)
			{
				fprintf(stderr, "Value: IOA:%i lof:%i nof:%i sof_fa:%i sof_for:%i sof_lfd:%i sof_status:%i\n",
				obj[i].ioa,
				obj[i].o.type126.lof,
				obj[i].o.type126.nof,
				obj[i].o.type126.sof_fa,
				obj[i].o.type126.sof_for,
				obj[i].o.type126.sof_lfd,
				obj[i].o.type126.sof_status);
				fflush(stderr);

				IT_COMMENT7("Value: IOA:%i lof:%i nof:%i sof_fa:%i sof_for:%i sof_lfd:%i sof_status:%i",
				obj[i].ioa,
				obj[i].o.type126.lof,
				obj[i].o.type126.nof,
				obj[i].o.type126.sof_fa,
				obj[i].o.type126.sof_for,
				obj[i].o.type126.sof_lfd,
				obj[i].o.type126.sof_status);

				fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
				obj[i].o.type126.time.hour,
				obj[i].o.type126.time.min,
				obj[i].o.type126.time.msec/1000,
				obj[i].o.type126.time.msec%1000,
				obj[i].o.type126.time.mday,
				obj[i].o.type126.time.month,
				obj[i].o.type126.time.year,
				obj[i].o.type126.time.iv,
				obj[i].o.type126.time.su);
				fflush(stderr);

				IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",
				obj[i].o.type126.time.hour,
				obj[i].o.type126.time.min,
				obj[i].o.type126.time.msec/1000,
				obj[i].o.type126.time.msec%1000,
				obj[i].o.type126.time.mday,
				obj[i].o.type126.time.month,
				obj[i].o.type126.time.year,
				obj[i].o.type126.time.iv,
				obj[i].o.type126.time.su);
			}

			//30-06-2010 change file transfer state machine to FT_RECEIVING_FILE
			//
			if(state_file_transfer == FT_SCAN_WORKING_DIRECTORY)
			{
				//no file transfer undergoing, so we can go in file transfer mode
				state_file_transfer = FT_RECEIVING_FILE;
				answer_unit.pn = 0; //positive
			}
			else
			{
				//a file transfer is undergoing in opposite direction, so we CANNOT go in file transfer mode
				answer_unit.pn = 1; //negative F_SC_NA_1
			}

			//select del primo file
			i = 0;
			ioa = obj[i].ioa; //ioa risposta
			answer_type.nof = obj[i].o.type126.nof;
			answer_type.nos = 1; //name of section
			answer_type.scq_ui4low = 1; //select file
			answer_type.scq_ui4high = 0;

			answer_unit.type = 122; //F_SC_NA_1
			answer_unit.num = 1;
			answer_unit.sq = 0;
			//answer_unit.pn = 0; //positive
			answer_unit.ca = caddr;
			answer_unit.t = 0;
			answer_unit.originator = 0;
						
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type122) + IOA_ADDRLEN) * answer_unit.num);

			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}

			get_iec_buf(&c);
			
			c->data_len = len;

			cp = c->data;
			
			answer_unit.cause = 0x0D; //file transfer

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;
						
			memcpy(cp, &ioa, IOA_ADDRLEN);
			cp += IOA_ADDRLEN;
			memcpy(cp, &answer_type, sizeof(struct iec_type122));
			cp += sizeof(struct iec_type122);
								
			iecsock_prepare_iframe(c);
			TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			//////////////////////////////////////////////////////////////////////////////////////////////////

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);
			
			if(state_file_transfer == FT_RECEIVING_FILE)
			{
				//Inizializzo file_trasferito a zero byte////////////////////////////////////////////////////////////

				FILE* ft;

				ft = fopen(file_trasferito,"wb");

				if(ft == NULL)
				{
					fprintf(stderr,"Failed to initialize file %s\n", file_trasferito);
					fflush(stderr);
					state_file_transfer = FT_ERROR;
					break;
				}

				fclose(ft);

				/*
				//Qui creo un nome univoco del file trasferito////////////////////////////////////////////////////
				{
					unsigned char* str;
					UUID uuid;
					UuidCreate(&uuid);
					UuidToString(&uuid, &str);

					file_trasferito[0] = '\0';

					if(GetModuleFileName(NULL, file_trasferito, _MAX_PATH))
					{
						*(strrchr(file_trasferito, '\\')) = '\0';        // Strip \\filename.exe off path
						*(strrchr(file_trasferito, '\\')) = '\0';        // Strip \\bin off path
    
						strcat(file_trasferito,"\\logs\\");
						strcat(file_trasferito, (const char*)str);
						strcat(file_trasferito,".csv");
					}

					RpcStringFree(&str);
				}
				*/
				/////////////////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
		default:
		break;
	}

	IT_EXIT;
}


//Single point
#define slave_plot_info_sp(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.sp, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.sp, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
	}


//Double point
#define slave_plot_info_dp(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i dp:%i bl:%i sb:%i nt:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.dp, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i dp:%i bl:%i sb:%i nt:%i iv:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.dp, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
	}


#define slave_plot_info_sp_wt(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.sp, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i sp:%i bl:%i sb:%i nt:%i iv:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.sp, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
	}


#define slave_plot_info_dp_wt(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i dp:%i bl:%i sb:%i nt:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.dp, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i dp:%i bl:%i sb:%i nt:%i iv:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.dp, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
	}

//Misure

#define slave_plot_info_mis_float(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i mv:%f bl:%i sb:%i nt:%i iv:%i ov:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.mv, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv, (items + k)->iec_obj.o.struct_name.ov);\
		fflush(stderr);\
		IT_COMMENT7("Value: IOA:%i mv:%f bl:%i sb:%i nt:%i iv:%i ov:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.mv, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv, (items + k)->iec_obj.o.struct_name.ov);\
	}

#define slave_plot_info_mis_short(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i mv:%d bl:%i sb:%i nt:%i iv:%i ov:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.mv, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv, (items + k)->iec_obj.o.struct_name.ov);\
		fflush(stderr);\
		IT_COMMENT7("Value: IOA:%i mv:%d bl:%i sb:%i nt:%i iv:%i ov:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.mv, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv, (items + k)->iec_obj.o.struct_name.ov);\
	}


#define slave_plot_info_mis_wt_short(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i mv:%d bl:%i sb:%i nt:%i iv:%i ov:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.mv, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv, (items + k)->iec_obj.o.struct_name.ov);\
		fflush(stderr);\
		IT_COMMENT7("Value: IOA:%i mv:%d bl:%i sb:%i nt:%i iv:%i ov:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.mv, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv, (items + k)->iec_obj.o.struct_name.ov);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
	}

#define slave_plot_info_mis_wt_float(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i mv:%f bl:%i sb:%i nt:%i iv:%i ov:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.mv, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv, (items + k)->iec_obj.o.struct_name.ov);\
		fflush(stderr);\
		IT_COMMENT7("Value: IOA:%i mv:%f bl:%i sb:%i nt:%i iv:%i ov:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.mv, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv, (items + k)->iec_obj.o.struct_name.ov);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
	}

#define slave_plot_info_counter(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i counter:%u sq:%i cy:%i ca:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.counter, (items + k)->iec_obj.o.struct_name.sq, \
		(items + k)->iec_obj.o.struct_name.cy, (items + k)->iec_obj.o.struct_name.ca, (items + k)->iec_obj.o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i counter:%u sq:%i cy:%i ca:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.counter, (items + k)->iec_obj.o.struct_name.sq, \
		(items + k)->iec_obj.o.struct_name.cy, (items + k)->iec_obj.o.struct_name.ca, (items + k)->iec_obj.o.struct_name.iv);\
	}


#define slave_plot_info_counter_wt(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i counter:%u sq:%i cy:%i ca:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.counter, (items + k)->iec_obj.o.struct_name.sq, \
		(items + k)->iec_obj.o.struct_name.cy, (items + k)->iec_obj.o.struct_name.ca, (items + k)->iec_obj.o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i counter:%u sq:%i cy:%i ca:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.counter, (items + k)->iec_obj.o.struct_name.sq, \
		(items + k)->iec_obj.o.struct_name.cy, (items + k)->iec_obj.o.struct_name.ca, (items + k)->iec_obj.o.struct_name.iv);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
	}

//Bitstring of 32 bit

#define slave_plot_info_bits(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i stcd:%u bl:%i sb:%i nt:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.stcd, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i stcd:%u bl:%i sb:%i nt:%i iv:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.stcd, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
	}

#define slave_plot_info_bits_wt(struct_name) \
	for(k = 0; k < answer_unit.num; k++){	\
		fprintf(stderr, "Value: IOA:%i stcd:%u bl:%i sb:%i nt:%i iv:%i\n",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.stcd, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
		fflush(stderr);\
		IT_COMMENT6("Value: IOA:%i stcd:%u bl:%i sb:%i nt:%i iv:%i",\
		(items + k)->iec_obj.ioa,\
		(items + k)->iec_obj.o.struct_name.stcd, (items + k)->iec_obj.o.struct_name.bl, \
		(items + k)->iec_obj.o.struct_name.sb, (items + k)->iec_obj.o.struct_name.nt, (items + k)->iec_obj.o.struct_name.iv);\
		fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
		fflush(stderr);\
		IT_COMMENT9("Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i",\
		(items + k)->iec_obj.o.struct_name.time.hour,\
		(items + k)->iec_obj.o.struct_name.time.min,\
		(items + k)->iec_obj.o.struct_name.time.msec/1000,\
		(items + k)->iec_obj.o.struct_name.time.msec%1000,\
		(items + k)->iec_obj.o.struct_name.time.mday,\
		(items + k)->iec_obj.o.struct_name.time.month,\
		(items + k)->iec_obj.o.struct_name.time.year,\
		(items + k)->iec_obj.o.struct_name.time.iv,\
		(items + k)->iec_obj.o.struct_name.time.su);\
	}


void send_items(struct iecsock *s, struct iec_item* items, int n_items)
{
	int len, k;
	struct iec_unit_id answer_unit;
	u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
	u_char *cp;
	struct iec_buf *c;

	IT_IT("send_items");

	fprintf(stderr,"Asdu type = %d\n", items->iec_type);
	fflush(stderr);

	fprintf(stderr,"items->cause = %d\n", items->cause);
	fflush(stderr);

    //0x03 Spontaneous <3> decimal
    //0x14 Interrogated by general interrogation <20> decimal

    //I messaggi della GI si accodano nella STESSA coda dei messaggi spontanei
	//quindi rimangono cronologicamente ordianti dal caricamento stesso della coda spontaneous_q.
	//Ordine cronologico di caricamento della spontaneous_q
	//Vengono inviati prima le variazioni (o spontanee) piu' vecchie, poi i messaggi GI e poi variazioni (o spontanee) piu' recenti

	switch(items->iec_type)
	{
		case M_SP_NA_1:
		{
			//Send M_SP_NA_1
			struct iec_type1 answer_type;

			//Update database
			if(iec_items_table != NULL)
			{
//				for(k = 0; k < n_items; k++)
//					iec_items_table[(items+k)->hClient - 1].iec_obj.o.type1 = (items+k)->iec_obj.o.type1; // - 1 poiche l'indice di iec_items_table parte da 0
			}

			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_SP_NA_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type1) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type1) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type1) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
				
			c->data_len = len;				
			cp = c->data;				
			answer_unit.cause = items->cause;
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type1;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type1));				
				cp += sizeof(struct iec_type1);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_sp(type1);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_DP_NA_1:
		{
			//Send M_DP_NA_1
			struct iec_type3 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
				//	iec_items_table[(items+k)->hClient - 1].iec_obj.o.type3 = (items+k)->iec_obj.o.type3;
			}
		
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_DP_NA_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type3) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type3) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type3) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;				
			answer_unit.cause = items->cause;
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type3;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type3));				
				cp += sizeof(struct iec_type3);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_dp(type3);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_BO_NA_1:
		{
			//Send M_BO_NA_1
			struct iec_type7 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type7 =  (items+k)->iec_obj.o.type7;
			}
			
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_BO_NA_1;
			answer_unit.num = n_items;				
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type7) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type7) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type7) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}

			get_iec_buf(&c);
						
			c->data_len = len;				
			cp = c->data;				
			answer_unit.cause = items->cause;
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type7;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type7));
				cp += sizeof(struct iec_type7);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_bits(type7);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_ME_NA_1:
		{
			//Send M_ME_NA_1
			struct iec_type9 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type9 = (items+k)->iec_obj.o.type9;
			}
			
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_ME_NA_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type9) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type9) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type9) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;				
			answer_unit.cause = items->cause;
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type9;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type9));
				cp += sizeof(struct iec_type9);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_mis_short(type9);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_ME_NB_1:
		{
			//Send M_ME_NB_1
			struct iec_type11 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type11 = (items+k)->iec_obj.o.type11;
			}

				////////////////////////////////////////////////////////////////////
				//
				//Send spontaneous
				answer_unit.type = M_ME_NB_1;
				answer_unit.num = n_items;
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type11) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type11) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type11) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
				
				get_iec_buf(&c);
				
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				

				for(k = 0; k < answer_unit.num; k++)
				{
					answer_type = (items + k)->iec_obj.o.type11;
					ioa = (items + k)->iec_obj.ioa;

					memcpy(cp, &ioa, IOA_ADDRLEN);				
					cp += IOA_ADDRLEN;				
					memcpy(cp, &answer_type, sizeof(struct iec_type11));				
					cp += sizeof(struct iec_type11);				
				}

				iecsock_prepare_iframe(c);				
				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

				if(log_all_slave_messages)
				{
					slave_plot_info_mis_short(type11);
				}
				//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_ME_NC_1:
		{
			//Send M_ME_NC_1
			struct iec_type13 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type13 = (items+k)->iec_obj.o.type13;
			}
			
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_ME_NC_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type13) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type13) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type13) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;				
			answer_unit.cause = items->cause;
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type13;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type13));
				cp += sizeof(struct iec_type13);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_mis_float(type13);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_IT_NA_1:
		{
			//Send M_IT_NA_1
			struct iec_type15 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type15 = (items+k)->iec_obj.o.type15;
			}
			
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_IT_NA_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type15) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type15) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type15) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;				
			answer_unit.cause = items->cause;
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type15;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type15));
				cp += sizeof(struct iec_type15);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_counter(type15);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_SP_TB_1:
		{
			//Send M_SP_TB_1
			struct iec_type30 answer_type;		
			
			//Update database;
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type30 = (items+k)->iec_obj.o.type30;
			}
			
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_SP_TB_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type30) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type30) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type30) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;

            if(items->cause == 0x14)
            {
                answer_unit.cause = 0x05; 	//request <5> decimale
            }
            else
            {
				answer_unit.cause = items->cause;
            }

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type30;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type30));
				cp += sizeof(struct iec_type30);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_sp_wt(type30);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_DP_TB_1:
		{
			//Send M_DP_TB_1
			struct iec_type31 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type31 = (items+k)->iec_obj.o.type31;
			}
			
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_DP_TB_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type31) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type31) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type31) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;				

			if(items->cause == 0x14)
            {
                answer_unit.cause = 0x05; 	//request <5> decimale
            }
            else
            {
				answer_unit.cause = items->cause;
            }

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type31;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type31));
				cp += sizeof(struct iec_type31);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_dp_wt(type31);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_BO_TB_1:
		{
			//Send M_BO_TB_1
			struct iec_type33 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type33 = (items+k)->iec_obj.o.type33;
			}
			
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_BO_TB_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type33) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type33) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type33) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;				

			if(items->cause == 0x14)
            {
                answer_unit.cause = 0x05; 	//request <5> decimale
            }
            else
            {
				answer_unit.cause = items->cause;
            }

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type33;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type33));
				cp += sizeof(struct iec_type33);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_bits_wt(type33);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_ME_TD_1:
		{
			//Send M_ME_TD_1
			struct iec_type34 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type34 = (items+k)->iec_obj.o.type34;
			}

			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_ME_TD_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type34) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type34) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type34) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}

			get_iec_buf(&c);
						
			c->data_len = len;				
			cp = c->data;				

			if(items->cause == 0x14)
            {
                answer_unit.cause = 0x05; 	//request <5> decimale
            }
            else
            {
				answer_unit.cause = items->cause;
            }

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type34;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type34));
				cp += sizeof(struct iec_type34);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_mis_wt_short(type34);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_ME_TE_1:
		{
			//Send M_ME_TE_1
			struct iec_type35 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type35 = (items+k)->iec_obj.o.type35;
			}

			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_ME_TE_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type35) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type35) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type35) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}

			get_iec_buf(&c);
						
			c->data_len = len;				
			cp = c->data;				

			if(items->cause == 0x14)
            {
                answer_unit.cause = 0x05; 	//request <5> decimale
            }
            else
            {
				answer_unit.cause = items->cause;
            }

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type35;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type35));
				cp += sizeof(struct iec_type35);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_mis_wt_short(type35);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_ME_TF_1:
		{
			//Send M_ME_TF_1
			struct iec_type36 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type36 = (items+k)->iec_obj.o.type36;
			}

			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_ME_TF_1;
			answer_unit.num = n_items;				
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type36) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type36) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type36) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;				

			if(items->cause == 0x14)
            {
                answer_unit.cause = 0x05; 	//request <5> decimale
            }
            else
            {
				answer_unit.cause = items->cause;
            }

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type36;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type36));
				cp += sizeof(struct iec_type36);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_mis_wt_float(type36);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_IT_TB_1:
		{
			//Send M_IT_TB_1
			struct iec_type37 answer_type;		
			
			//Update database
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type37 = (items+k)->iec_obj.o.type37;
			}

			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_IT_TB_1;
			answer_unit.num = n_items;				
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type37) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type37) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type37) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;				

			if(items->cause == 0x14)
            {
                answer_unit.cause = 0x25; 	//general counter interrogation <37> decimale
            }
            else
            {
				answer_unit.cause = items->cause;
            }

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type37;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type37));
				cp += sizeof(struct iec_type37);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

			if(log_all_slave_messages)
			{
				slave_plot_info_counter_wt(type37);
			}
			//////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case M_ME_TN_1:
		{
			//Send M_ME_TN_1
			struct iec_type150 answer_type;
			
			//Update database;
			if(iec_items_table != NULL)
			{
				//for(k = 0; k < n_items; k++)
					//iec_items_table[(items+k)->hClient - 1].iec_obj.o.type150 = (items+k)->iec_obj.o.type150;
			}
			
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = M_ME_TN_1;
			answer_unit.num = n_items;
			answer_unit.sq = 0;
			answer_unit.pn = 0;				
			answer_unit.ca = common_address_of_asdu;
			answer_unit.t = 0;				
			answer_unit.originator = 0;				
			len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type150) + IOA_ADDRLEN) * answer_unit.num);
			if (len > IEC104_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type150) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type150) + IOA_ADDRLEN);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
                #else
				exit(EXIT_FAILURE);
                #endif
			}
			
			get_iec_buf(&c);
			
			c->data_len = len;				
			cp = c->data;				

			if(items->cause == 0x14)
            {
                answer_unit.cause = 0x05; 	//request <5> decimale
            }
            else
            {
				answer_unit.cause = items->cause;
            }

			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type150;
				ioa = (items + k)->iec_obj.ioa;

				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type150));
				cp += sizeof(struct iec_type150);
			}

			iecsock_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			fflush(stderr);

			IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
            //////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		case C_SC_NA_1://single command
		{
			if(s->type == IEC_SLAVE)
			{
				//We are in the SLAVE, here we send the activation confirmation or activation termination
                
				//Send C_SC_NA_1
				struct iec_type45 answer_type;

				//Send activation
				answer_unit.type = C_SC_NA_1;
				
				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type45) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type45) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type45) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
				
				for(k = 0; k < answer_unit.num; k++) 
				{
					answer_type = (items + k)->iec_obj.o.type45;
					ioa = (items + k)->iec_obj.ioa;

					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type45));
					cp += sizeof(struct iec_type45);
				}
				
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				if(state_tcp_link == LNK_CONNECTED) //non bufferizzo il comando!
				{
					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type45 answer_type;
										
					//Send activation 
					answer_unit.type = C_SC_NA_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type45) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type45) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type45) + IOA_ADDRLEN);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
					
//					for (k = 0; k < answer_unit.num; k++) 
//					{
						answer_type = items->iec_obj.o.type45;
						ioa = items->iec_obj.ioa;

						memcpy(cp, &ioa, IOA_ADDRLEN);
						cp += IOA_ADDRLEN;
						memcpy(cp, &answer_type, sizeof(struct iec_type45));
						cp += sizeof(struct iec_type45);
//					}
							
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

					//NOTA: Questa bufferizzazione potrebbe essere pericolosa.
					//Aviare qui un timer di 10 secondi ed azzerarlo alla
					//avvenuta esecuzione o presa in consegna del comando
					//da parte del livello inferiore

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			        fflush(stderr);

                    fprintf(stderr, "Value: IOA:%i qu:%i scs:%i se:%i\n",
					ioa,
					answer_type.qu, answer_type.scs, 
					answer_type.se);
					fflush(stderr);
				}
				else
				{
					//Comando scartato
					fprintf(stderr,"Comando scartato: link down\n");
					fflush(stderr);
				}
			}
		}
		break;
		case C_IC_NA_1://general interrogation command
		{
			if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER e questa richiesta di general interrogation arriva dallo SCADA superiore (es. operatore)

				//Verifico le condizioni per fare la GI

				//if(state_of_slave == SLAVE_INITIALIZED)  //SLAVE inizializzato
				//{
					if(state_general_interrogation == GI_IDLE) //non e' in corso un'altra GI nel MASTER
					{
						struct iec_type100 answer_type;

						answer_type = items->iec_obj.o.type100;
						ioa = items->iec_obj.ioa;
						//ioa = 0; //deve essere nullo 
						//answer_type.qoi = 0x14; //Station interrogation
						answer_unit.type = C_IC_NA_1;
						answer_unit.num = 1;				
						answer_unit.sq = 0;				
						answer_unit.pn = 0;	//positive
						answer_unit.ca = common_address_of_asdu;				
						answer_unit.t = 0;				
						answer_unit.originator = 0;				
						len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type100) + IOA_ADDRLEN) * answer_unit.num);				
						if (len > IEC104_ASDU_MAX)
						{
							fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
							fflush(stderr);
							fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type100) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type100) + IOA_ADDRLEN);
							fflush(stderr);
							#ifdef WIN32
							ExitProcess(0);
                            #else
    						exit(EXIT_FAILURE);
                            #endif
						}
						
						get_iec_buf(&c);
						
						c->data_len = len;				
						cp = c->data;				
						answer_unit.cause = 0x06; 	//activation
						memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
						cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
						memcpy(cp, &ioa, IOA_ADDRLEN);				
						cp += IOA_ADDRLEN;				
						memcpy(cp, &answer_type, sizeof(struct iec_type100));				
						cp += sizeof(struct iec_type100);				
						iecsock_prepare_iframe(c);				
						
						TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
						
						fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
						fflush(stderr);

						IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);

						state_general_interrogation = GI_ACTIVATION;
					}
				//}
			}
			else if(s->type == IEC_SLAVE)
			{
				///////OLD CODE///////////////////////////////////////////////////
				//quindi possiamo procedere al caricamento dei punti dal database
				//per essere inviato al MASTER come general interrogation
				//state_general_interrogation = GI_LOAD_POINTS;
				//////////////////////////END OLD CODE//////////////////////////////////

                if(items->cause == 0x07)
                {
			        //Send GI activation confirmation to MASTER

			        //int len;
			        //struct iec_unit_id answer_unit;
			        struct iec_type100 answer_type;
			        //u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
			        //u_char *cp;

			        //C_IC_NA_1 confirmation
			        //set the answer

			        ioa = 0;
			        answer_type.qoi = 0x14; //Station interrogation
			        answer_unit.type = C_IC_NA_1;

			        if(n_items != 1)
			        {
				        fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
				        fflush(stderr);
				        #ifdef WIN32
				        ExitProcess(0);
                        #else
				        exit(EXIT_FAILURE);
                        #endif
			        }

			        answer_unit.num = 1;				
			        answer_unit.sq = 0;

			        if(!items->is_neg)
					{
						answer_unit.pn = 0;	//positive
					}
					else
					{
                        answer_unit.pn = 1;	//negative
					}

			        answer_unit.ca = common_address_of_asdu;
			        answer_unit.t = 0;
			        answer_unit.originator = 0;
						        
			        len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type100) + IOA_ADDRLEN) * answer_unit.num);

			        if (len > IEC104_ASDU_MAX)
			        {
				        state_general_interrogation = GI_IDLE;
				        IT_EXIT;
				        return;
			        }

			        get_iec_buf(&c);
		        
			        c->data_len = len;

			        cp = c->data;
			        
			        answer_unit.cause = 0x07; //activation confirmation

			        memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
			        cp += IEC_TYPEID_LEN + COM_ADDRLEN;
			        			        
				    memcpy(cp, &ioa, IOA_ADDRLEN);
				    cp += IOA_ADDRLEN;
				    memcpy(cp, &answer_type, sizeof(struct iec_type100));
				    cp += sizeof(struct iec_type100);
			        					        
			        iecsock_prepare_iframe(c);

			        //apa+++ 21-11-2010 Very important to send confirmation immediately in s->high_priority_q queue
			        //otherwise the timer on MASTER runs over the MAX_GI_TIME_IN_SECONDS time
			        TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
                }
                else if(items->cause == 0x0A)
				{
                    //We are in the SLAVE and opc_client.exe or iec101master.exe has finished to GI 

					//Send GI activation termination to MASTER in spontaneous queue
					//int len;				
					//struct iec_buf *c;				
					//struct iec_unit_id answer_unit;
					struct iec_type100 answer_type;
					//u_int ioa;	//importante: ioa da 3 ottetti deve essere u_int e non u_short					
					//u_char *cp;	
					ioa = 0;
					answer_type.qoi = 0x14; //Station interrogation
					answer_unit.type = C_IC_NA_1;
					answer_unit.num = 1;				
					answer_unit.sq = 0;

					if(!items->is_neg)
					{
						answer_unit.pn = 0;	//positive
					}
					else
					{
                        answer_unit.pn = 1;	//negative
					}

					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;				
					answer_unit.originator = 0;				
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type100) + IOA_ADDRLEN) * answer_unit.num);
					if (len > IEC104_ASDU_MAX)
					{
						state_general_interrogation = GI_IDLE;
						IT_EXIT;
						return;
					}
					//Termination
					get_iec_buf(&c);
						
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x0A; //activation termination

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type100));
					cp += sizeof(struct iec_type100);
												
					iecsock_prepare_iframe(c);

					//apa+++ 19-11-2010 Very important to send termination at the end of GA messages in s->spontaneous_q queue
					TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);
					//////////////////////////////////////////////////////////////////////////////////////////////////
				}

				state_general_interrogation = GI_IDLE;
			}
		}
		break;
		case C_RP_NA_1: //Reset process command
		{
			if(s->type == IEC_MASTER)
			{
				//Se siamo nel MASTER, questa richiesta di reset process arriva dallo SCADA superiore (es. operatore)
				//Send reset process to SLAVE in control direction, se l'operatore ha bisogno di resettare lo SLAVE 
				struct iec_type105 answer_type;

				answer_type = items->iec_obj.o.type105;
				ioa = items->iec_obj.ioa;

				ioa = 0;  //ioa risposta C_RP_NA_1 deve essere nullo 
				answer_type.qrp = 1; //General reset process
			
				answer_unit.type = 105; //C_RP_NA_1
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type105) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type105) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type105) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//activation
				answer_unit.cause = 0x06;

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type105));
				cp += sizeof(struct iec_type105);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
				//////////////////////////////////////////////////////////////////////////////////////////////////

                fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
		        fflush(stderr);
			}
			else if(s->type == IEC_SLAVE)
			{
				//siamo nello SLAVE opc_client.exe ci chiede di resettare lo SLAVE

				//fprintf(stderr, "-----------Flushing queues----------\n");
				//fflush(stderr);

				//iecsock_flush_queues(s);// remove of old packets apa+++ 22-10-2010; commented out on 05-09-2011
				
				/*
				{
					char ftwd_conf[256 + 10];
					//Reload config database
					state_database = LOAD_DATABASE;

					//Copy file_configurazione to file transfer working directory
					
					strcpy(ftwd_conf, file_transf_working_dir);
					strcat(ftwd_conf, "\\file_configurazione.csv");

					//overwrite if the file exists
					if(!CopyFile(file_configurazione, ftwd_conf, FALSE))
					{
						//failed to copy the file
						fprintf(stderr, "Error in coping file from %s to %s", file_configurazione, ftwd_conf);
						fflush(stderr);
					}

					state_file_transfer = FT_SCAN_WORKING_DIRECTORY;
				}
				*/

				//opc_client_da.exe requests to "load" the database and later to send M_EI_NA_1

				state_database = LOAD_DATABASE; //apa +++ on 05-09-2011 only for Camel code
			}
		}
		break;
		case C_SC_TA_1: //single command with time stamp
		{
			if(s->type == IEC_SLAVE)
			{
				//Siamo nello SLAVE, qui invio la termination del comando al master
				//Send C_SC_TA_1
				struct iec_type58 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type58;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_SC_TA_1;
				
				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type58) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type58) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type58) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type58));
				cp += sizeof(struct iec_type58);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				//if((state_clock_synchronisation == CLK_STATIONS_SYNCRONIZED) && (state_tcp_link == LNK_CONNECTED))
				{
					//Inviare un comando di tipo C_SC_TA_1 - Single command with time tag
					//Quando l'RTU (OPC client) riceve questo comando, controlla che il comando
					//abbia vita <= 10 s, se si lo esegue altrimenti lo scarta.
					//Condizione per il funzionamento di questo controllo, e' che
					//IEC 104 MASTER e IEC 104 SLAVE stations siano sincronizzate sull'orologio
					//mediante il comando clock di sinc C_CS_NA_1.
					//Se le stazioni non sono sincronizzate

					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type58 answer_type;
										
					//Send activation
					//NOTE: the answer_type.time del tipo iec_type58 e' quella assegnata dallo SCADA superiore
					//con la get_utc_iec_time, nell'attimo in cui impacchetta il comando nella 
					//FIFO fifo_control_direction, cosi' quando si esegue il comando nella RTU (OPC client) ho il
					//tempo di vita del comando da quando lo prendo in carico a quando lo eseguo
									
					answer_type = items->iec_obj.o.type58;
					ioa = items->iec_obj.ioa;
					
					answer_unit.type = C_SC_TA_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type58) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type58) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type58) + IOA_ADDRLEN);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type58));
					cp += sizeof(struct iec_type58);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
    		        fflush(stderr);
				}
				//else
				//{
				//	//Comando scartato
				//	fprintf(stderr,"Command rejected: master and slave not sincronized or link down\n");
				//	fflush(stderr);
				//}
			}
		}
		break;
		case C_DC_NA_1:
		{
			if(s->type == IEC_SLAVE)
			{
				//Siamo nello SLAVE, qui invio la termination del comando al master
				//Send C_DC_NA_1
				struct iec_type46 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type46;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_DC_NA_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type46) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type46) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type46) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type46));
				cp += sizeof(struct iec_type46);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				if(state_tcp_link == LNK_CONNECTED) //non bufferizzo il comando!
				{
					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type46 answer_type;
										
					//Send activation 
					answer_type = items->iec_obj.o.type46;
					ioa = items->iec_obj.ioa;
					
					answer_unit.type = C_DC_NA_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type46) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type46) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type46) + IOA_ADDRLEN);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type46));
					cp += sizeof(struct iec_type46);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

					//NOTA: Questa bufferizzazione potrebbe essere pericolosa.
					//Aviare qui un timer di 10 secondi ed azzerarlo alla
					//avvenuta esecuzione o presa in consegna del comando
					//da parte del livello inferiore

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
			        fflush(stderr);

                    fprintf(stderr, "Value: IOA:%i qu:%i dcs:%i se:%i\n",
					ioa,
					answer_type.qu, answer_type.dcs, 
					answer_type.se);
					fflush(stderr);
				}
				else
				{
					//Comando scartato
					fprintf(stderr,"Comando scartato: link down\n");
					fflush(stderr);
				}
			}
		}
		break;
		case C_DC_TA_1:
		{
			if(s->type == IEC_SLAVE)
			{
				//Send C_DC_TA_1
				struct iec_type59 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type59;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_DC_TA_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type59) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type59) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type59) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type59));
				cp += sizeof(struct iec_type59);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				//if((state_clock_synchronisation == CLK_STATIONS_SYNCRONIZED) && (state_tcp_link == LNK_CONNECTED))
				{
					//Inviare un comando di tipo C_DC_TA_1 - Double command with time tag
					//Quando l'RTU (OPC client) riceve questo comando, controlla che il comando
					//abbia vita <= 10 s, se si lo esegue altrimenti lo scarta.
					//Condizione per il funzionamento di questo controllo, e' che
					//IEC 104 MASTER e IEC 104 SLAVE stations siano sincronizzate sull'orologio
					//mediante il comando clock di sinc C_CS_NA_1.
					//Se le stazioni non sono sincronizzate

					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type59 answer_type;
										
					//Send activation
					//NOTE: the answer_type.time del tipo iec_type59 e' quella assegnata dallo SCADA superiore
					//con la get_utc_iec_time, nell'attimo in cui impacchetta il comando nella 
					//FIFO fifo_control_direction, cosi' quando si esegue il comando nella RTU (OPC client) ho il
					//tempo di vita del comando da quando lo prendo in carico a quando lo eseguo
									
					answer_type = items->iec_obj.o.type59;
					ioa = items->iec_obj.ioa;
					
					answer_unit.type = C_DC_TA_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type59) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type59) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type59) + IOA_ADDRLEN);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type59));
					cp += sizeof(struct iec_type59);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
    		        fflush(stderr);
				}
				//else
				//{
				//	//Comando scartato
				//	fprintf(stderr,"Command rejected: master and slave not sincronized or  link down\n");
				//	fflush(stderr);
				//}
			}
		}
		break;
		case C_SE_NA_1:
		{
			if(s->type == IEC_SLAVE)
			{
				//Send C_SE_NA_1
				struct iec_type48 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type48;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_SE_NA_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type48) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type48) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type48) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type48));
				cp += sizeof(struct iec_type48);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				//if((state_clock_synchronisation == CLK_STATIONS_SYNCRONIZED) && (state_tcp_link == LNK_CONNECTED))
				{
					//Inviare un comando di tipo C_SE_NA_1
					//Quando l'RTU (OPC client) riceve questo comando, controlla che il comando
					//abbia vita <= 10 s, se si lo esegue altrimenti lo scarta.
					//Condizione per il funzionamento di questo controllo, e' che
					//IEC 104 MASTER e IEC 104 SLAVE stations siano sincronizzate sull'orologio
					//mediante il comando clock di sinc C_CS_NA_1.
					//Se le stazioni non sono sincronizzate

					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type48 answer_type;
										
					//Send activation
					//NOTE: the answer_type.time del tipo iec_type48 e' quella assegnata dallo SCADA superiore
					//con la get_utc_iec_time, nell'attimo in cui impacchetta il comando nella 
					//FIFO fifo_control_direction, cosi' quando si esegue il comando nella RTU (OPC client) ho il
					//tempo di vita del comando da quando lo prendo in carico a quando lo eseguo
									
					answer_type = items->iec_obj.o.type48;
					ioa = items->iec_obj.ioa;
					
					answer_unit.type = C_SE_NA_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type48) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type48) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type48) + IOA_ADDRLEN);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type48));
					cp += sizeof(struct iec_type48);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
    		        fflush(stderr);
				}
				//else
				//{
				//	//Comando scartato
				//	fprintf(stderr,"Command rejected: master and slave not sincronized or  link down\n");
				//	fflush(stderr);
				//}
			}
		}
		break;
		case C_SE_TA_1:
		{
			if(s->type == IEC_SLAVE)
			{
				//Send C_SE_TA_1
				struct iec_type61 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type61;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_SE_TA_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type61) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type61) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type61) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type61));
				cp += sizeof(struct iec_type61);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				//if((state_clock_synchronisation == CLK_STATIONS_SYNCRONIZED) && (state_tcp_link == LNK_CONNECTED))
				{
					//Inviare un comando di tipo C_SE_TA_1
					//Quando l'RTU (OPC client) riceve questo comando, controlla che il comando
					//abbia vita <= 10 s, se si lo esegue altrimenti lo scarta.
					//Condizione per il funzionamento di questo controllo, e' che
					//IEC 104 MASTER e IEC 104 SLAVE stations siano sincronizzate sull'orologio
					//mediante il comando clock di sinc C_CS_NA_1.
					//Se le stazioni non sono sincronizzate

					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type61 answer_type;
										
					//Send activation
					//NOTE: the answer_type.time del tipo iec_type61 e' quella assegnata dallo SCADA superiore
					//con la get_utc_iec_time, nell'attimo in cui impacchetta il comando nella 
					//FIFO fifo_control_direction, cosi' quando si esegue il comando nella RTU (OPC client) ho il
					//tempo di vita del comando da quando lo prendo in carico a quando lo eseguo
									
					answer_type = items->iec_obj.o.type61;
					ioa = items->iec_obj.ioa;
					
					answer_unit.type = C_SE_TA_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type61) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type61) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type61) + IOA_ADDRLEN);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type61));
					cp += sizeof(struct iec_type61);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
    		        fflush(stderr);
				}
				//else
				//{
				//	//Comando scartato
				//	fprintf(stderr,"Command rejected: master and slave not sincronized or  link down\n");
				//	fflush(stderr);
				//}
			}
		}
		break;
		case C_SE_NB_1:
		{
			if(s->type == IEC_SLAVE)
			{
				//Send C_SE_NB_1
				struct iec_type49 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type49;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_SE_NB_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type49) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type49) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type49) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
							
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type49));
				cp += sizeof(struct iec_type49);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				//if((state_clock_synchronisation == CLK_STATIONS_SYNCRONIZED) && (state_tcp_link == LNK_CONNECTED))
				{
					//Inviare un comando di tipo C_SE_NB_1
					//Quando l'RTU (OPC client) riceve questo comando, controlla che il comando
					//abbia vita <= 10 s, se si lo esegue altrimenti lo scarta.
					//Condizione per il funzionamento di questo controllo, e' che
					//IEC 104 MASTER e IEC 104 SLAVE stations siano sincronizzate sull'orologio
					//mediante il comando clock di sinc C_CS_NA_1.
					//Se le stazioni non sono sincronizzate

					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type49 answer_type;
										
					//Send activation
					//NOTE: the answer_type.time del tipo iec_type49 e' quella assegnata dallo SCADA superiore
					//con la get_utc_iec_time, nell'attimo in cui impacchetta il comando nella 
					//FIFO fifo_control_direction, cosi' quando si esegue il comando nella RTU (OPC client) ho il
					//tempo di vita del comando da quando lo prendo in carico a quando lo eseguo
									
					answer_type = items->iec_obj.o.type49;
					ioa = items->iec_obj.ioa;
					
					answer_unit.type = C_SE_NB_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type49) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type49) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type49) + IOA_ADDRLEN);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
									
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type49));
					cp += sizeof(struct iec_type49);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
	    	        fflush(stderr);
				}
				//else
				//{
				//	//Comando scartato
				//	fprintf(stderr,"Command rejected: master and slave not sincronized or  link down\n");
				//	fflush(stderr);
				//}
			}
		}
		break;
		case C_SE_TB_1:
		{
			if(s->type == IEC_SLAVE)
			{
				//Send C_SE_TB_1
				struct iec_type62 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type62;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_SE_TB_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type62) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type62) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type62) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
							
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type62));
				cp += sizeof(struct iec_type62);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				//if((state_clock_synchronisation == CLK_STATIONS_SYNCRONIZED) && (state_tcp_link == LNK_CONNECTED))
				{
					//Inviare un comando di tipo C_SE_TB_1
					//Quando l'RTU (OPC client) riceve questo comando, controlla che il comando
					//abbia vita <= 10 s, se si lo esegue altrimenti lo scarta.
					//Condizione per il funzionamento di questo controllo, e' che
					//IEC 104 MASTER e IEC 104 SLAVE stations siano sincronizzate sull'orologio
					//mediante il comando clock di sinc C_CS_NA_1.
					//Se le stazioni non sono sincronizzate

					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type62 answer_type;
										
					//Send activation
					//NOTE: the answer_type.time del tipo iec_type62 e' quella assegnata dallo SCADA superiore
					//con la get_utc_iec_time, nell'attimo in cui impacchetta il comando nella 
					//FIFO fifo_control_direction, cosi' quando si esegue il comando nella RTU (OPC client) ho il
					//tempo di vita del comando da quando lo prendo in carico a quando lo eseguo
									
					answer_type = items->iec_obj.o.type62;
					ioa = items->iec_obj.ioa;
					
					answer_unit.type = C_SE_TB_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type62) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type62) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type62) + IOA_ADDRLEN);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type62));
					cp += sizeof(struct iec_type62);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
    		        fflush(stderr);
				}
				//else
				//{
				//	//Comando scartato
				//	fprintf(stderr,"Command rejected: master and slave not sincronized or  link down\n");
				//	fflush(stderr);
				//}
			}
		}
		break;
		case C_SE_NC_1:
		{
			if(s->type == IEC_SLAVE)
			{
				//Send C_SE_NC_1
				struct iec_type50 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type50;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_SE_NC_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type50) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type50) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type50) + IOA_ADDRLEN);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type50));
				cp += sizeof(struct iec_type50);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				//if((state_clock_synchronisation == CLK_STATIONS_SYNCRONIZED) && (state_tcp_link == LNK_CONNECTED))
				{
					//Inviare un comando di tipo C_SE_NC_1
					//Quando l'RTU (OPC client) riceve questo comando, controlla che il comando
					//abbia vita <= 10 s, se si lo esegue altrimenti lo scarta.
					//Condizione per il funzionamento di questo controllo, e' che
					//IEC 104 MASTER e IEC 104 SLAVE stations siano sincronizzate sull'orologio
					//mediante il comando clock di sinc C_CS_NA_1.
					//Se le stazioni non sono sincronizzate

					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type50 answer_type;
										
					//Send activation
					//NOTE: the answer_type.time del tipo iec_type50 e' quella assegnata dallo SCADA superiore
					//con la get_utc_iec_time, nell'attimo in cui impacchetta il comando nella 
					//FIFO fifo_control_direction, cosi' quando si esegue il comando nella RTU (OPC client) ho il
					//tempo di vita del comando da quando lo prendo in carico a quando lo eseguo
									
					answer_type = items->iec_obj.o.type50;
					ioa = items->iec_obj.ioa;
					
					answer_unit.type = C_SE_NC_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type50) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						fprintf(stderr,"len = %d, answer_unit.num = %d, sizeof(struct iec_type50) + IOA_ADDRLEN = %d\n", len, answer_unit.num, sizeof(struct iec_type50) + IOA_ADDRLEN);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type50));
					cp += sizeof(struct iec_type50);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
        	        fflush(stderr);
				}
				//else
				//{
				//	//Comando scartato
				//	fprintf(stderr,"Command rejected: master and slave not sincronized or  link down\n");
				//	fflush(stderr);
				//}
			}
		}
		break;
		case C_SE_TC_1:
		{
			if(s->type == IEC_SLAVE)
			{
				//Send C_SE_TC_1
				struct iec_type63 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type63;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_SE_TC_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type63) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type63));
				cp += sizeof(struct iec_type63);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando da operatore
				//if((state_clock_synchronisation == CLK_STATIONS_SYNCRONIZED) && (state_tcp_link == LNK_CONNECTED))
				{
					//Inviare un comando di tipo C_SE_TC_1
					//Quando l'RTU (OPC client) riceve questo comando, controlla che il comando
					//abbia vita <= 10 s, se si lo esegue altrimenti lo scarta.
					//Condizione per il funzionamento di questo controllo, e' che
					//IEC 104 MASTER e IEC 104 SLAVE stations siano sincronizzate sull'orologio
					//mediante il comando clock di sinc C_CS_NA_1.
					//Se le stazioni non sono sincronizzate

					//Siamo nel MASTER, qui invio il comando, proveniente dall'operatore dello SCADA superiore, allo slave
					struct iec_type63 answer_type;
										
					//Send activation
					//NOTE: the answer_type.time del tipo iec_type63 e' quella assegnata dallo SCADA superiore
					//con la get_utc_iec_time, nell'attimo in cui impacchetta il comando nella 
					//FIFO fifo_control_direction, cosi' quando si esegue il comando nella RTU (OPC client) ho il
					//tempo di vita del comando da quando lo prendo in carico a quando lo eseguo
									
					answer_type = items->iec_obj.o.type63;
					ioa = items->iec_obj.ioa;
					
					answer_unit.type = C_SE_TC_1;
					answer_unit.num = 1;
					answer_unit.sq = 0;
					answer_unit.pn = 0; //positive
					answer_unit.ca = common_address_of_asdu;
					answer_unit.t = 0;
					answer_unit.originator = 0;
								
					len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type63) + IOA_ADDRLEN) * answer_unit.num);

					if (len > IEC104_ASDU_MAX)
					{
						fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
						fflush(stderr);
						#ifdef WIN32
						ExitProcess(0);
                        #else
						exit(EXIT_FAILURE);
                        #endif
					}

					get_iec_buf(&c);
			
					c->data_len = len;

					cp = c->data;
					
					answer_unit.cause = 0x06; //activation

					memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
					cp += IEC_TYPEID_LEN + COM_ADDRLEN;
										
					memcpy(cp, &ioa, IOA_ADDRLEN);
					cp += IOA_ADDRLEN;
					memcpy(cp, &answer_type, sizeof(struct iec_type63));
					cp += sizeof(struct iec_type63);
												
					iecsock_prepare_iframe(c);
					TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
    		        fflush(stderr);
				}
				//else
				//{
				//	//Comando scartato
				//	fprintf(stderr,"Command rejected: master and slave not sincronized or  link down\n");
				//	fflush(stderr);
				//}
			}
		}
		break;
		case C_CS_NA_1://clock synchronisation command
		{
			if(s->type == IEC_SLAVE)
			{
				//Send back to MASTER the activation confirmation received from field
				struct iec_type103 answer_type;

				answer_type = items->iec_obj.o.type103;
				ioa = items->iec_obj.ioa;
								
				//prepara la activation

				answer_unit.type = C_CS_NA_1;
				answer_unit.num = 1;
				answer_unit.sq = 0;

                if(items->is_neg)
                {
				    answer_unit.pn = 1; //negative
                }
                else
                {
                    answer_unit.pn = 0; //positive
                }

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type103) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;
				
				answer_unit.cause = 0x07; //activation confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type103));
				cp += sizeof(struct iec_type103);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando di clock sync da operatore
				struct iec_type103 answer_type;

				answer_type = items->iec_obj.o.type103;
				ioa = items->iec_obj.ioa;
								
				//prepara la activation

				answer_unit.type = C_CS_NA_1;
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type103) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;
				
				answer_unit.cause = 0x06; //activation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type103));
				cp += sizeof(struct iec_type103);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
		        fflush(stderr);
			}
		}
		break;
		case C_BO_NA_1:
		{
			if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, questo comando arriva dallo SCADA superiore (es. operatore)
				if(state_tcp_link == LNK_CONNECTED) //non bufferizzo il comando!
				{
					struct iec_type51 answer_type;
					int j;

					//items->opc_server_item_id e type51.stcd (che e' il valore del comando) e' inizializzato dallo SCADA superiore
					//in stcd hai 32 bit, ci puoi scrivere anche ACK per fare l'acknowlede degli allarmi
					
					answer_type = items->iec_obj.o.type51; 

					//Query: "select hClient in table file_configurazione.csv where opc_server_item_id == items->opc_server_item_id"

					ioa = 0;

					for(j = 0; j < num_iec_items; j++)
					{
						if(iec_items_table != NULL)
						{
                            /*
                            int rc;
							if((rc = strcmp(iec_items_table[j].opc_server_item_id, items->opc_server_item_id)) == 0)
							{
								ioa = iec_items_table[j].hClient;

								if(ioa)
								{
									fprintf(stderr,"Command for %s on ioa %d\n",iec_items_table[j].opc_server_item_id, ioa);
									fflush(stderr);

									answer_unit.type = C_BO_NA_1;
									answer_unit.num = 1;				
									answer_unit.sq = 0;				
									answer_unit.pn = 0;	//positive
									answer_unit.ca = common_address_of_asdu;				
									answer_unit.t = 0;				
									answer_unit.originator = 0;				
									len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type51) + IOA_ADDRLEN) * answer_unit.num);
									if (len > IEC104_ASDU_MAX)
									{
										fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
										fflush(stderr);
										#ifdef WIN32
					            		ExitProcess(0);
                                        #else
							            exit(EXIT_FAILURE);
                                        #endif
									}
									
									get_iec_buf(&c);
									
									c->data_len = len;				
									cp = c->data;				
									answer_unit.cause = 0x06; 	//activation
									memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
									cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
									memcpy(cp, &ioa, IOA_ADDRLEN);				
									cp += IOA_ADDRLEN;				
									memcpy(cp, &answer_type, sizeof(struct iec_type51));				
									cp += sizeof(struct iec_type51);				
									iecsock_prepare_iframe(c);				
									TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

									fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
									fflush(stderr);

									IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
								}

								break;
							}
                            */
						}
					}

					if(ioa == 0)
					{
						//Comando scartato
						fprintf(stderr,"Comando scartato: ItemID OPC not found\n");
						fflush(stderr);
					}
				}
				else
				{
					//Comando scartato
					fprintf(stderr,"Comando scartato: link down\n");
					fflush(stderr);
				}
			}
			else if(s->type == IEC_SLAVE)
			{
				//Siamo nello SLAVE opc_client.exe ha finito di scrivere un items
				//quindi, qui invio la termination del comando al master
				
				struct iec_type51 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type51;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_BO_NA_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type51) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = 0x0A; //activation termination
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type51));
				cp += sizeof(struct iec_type51);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
		}
		break;
		case C_BO_TA_1:
		{
			if(s->type == IEC_MASTER)
			{
				//Per simulare il MASTER con IEC-TEST impostare:
				//TK:64 SW digi_Zt 
				//$41 LL	'A'
				//$43 LH	'C'
				//$4B HL	'K'
				//$00 HH	'\0'
				//Valore intero: 4932417
				////////////////20-04-2010////remove ASAP////////////////
				//state_clock_synchronisation = CLK_STATIONS_SYNCRONIZED;
				////////////////04-07-2010////remove ASAP////////////////
				//int size = sizeof(struct iec_type64);
				//fprintf(stderr,"sizeof(struct iec_type64) = %d\n", sizeof(struct iec_type64));
				//fflush(stderr);
				///////////////////////end remove////////////////////////

				//Siamo nel MASTER, questo comando arriva dallo SCADA superiore (es. operatore)
				//if((state_clock_synchronisation == CLK_STATIONS_SYNCRONIZED) && (state_tcp_link == LNK_CONNECTED))
				{
					struct iec_type64 answer_type;
					int j;

					//items->opc_server_item_id e type64.stcd (che e' il valore del comando) e' inizializzato dallo SCADA superiore
					//in stcd hai 32 bit, ci puoi scrivere anche ACK per fare l'acknowlede degli allarmi
					
					answer_type = items->iec_obj.o.type64;

					//Query: "select hClient in table file_configurazione.csv where opc_server_item_id == items->opc_server_item_id"

					ioa = 0;

					for(j = 0; j < num_iec_items; j++)
					{
                        /*
                        int rc;
						if(iec_items_table != NULL)
						{
							//fprintf(stderr,"%s\n", iec_items_table[j].opc_server_item_id);
							//fflush(stderr);

							if((rc = strcmp(iec_items_table[j].opc_server_item_id, items->opc_server_item_id)) == 0)
							{
								ioa = iec_items_table[j].hClient;

								if(ioa)
								{
									fprintf(stderr,"Command for %s on ioa %d, parametro = %u\n",iec_items_table[j].opc_server_item_id, ioa, items->iec_obj.o.type64.stcd);
									fflush(stderr);

									answer_unit.type = C_BO_TA_1;
									answer_unit.num = 1;				
									answer_unit.sq = 0;				
									answer_unit.pn = 0;	//positive
									answer_unit.ca = common_address_of_asdu;				
									answer_unit.t = 0;				
									answer_unit.originator = 0;				
									len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type64) + IOA_ADDRLEN) * answer_unit.num);
									if (len > IEC104_ASDU_MAX)
									{
										fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
										fflush(stderr);
										#ifdef WIN32
					            		ExitProcess(0);
                                        #else
							            exit(EXIT_FAILURE);
                                        #endif
									}
									
									get_iec_buf(&c);
									
									c->data_len = len;				
									cp = c->data;				
									answer_unit.cause = 0x06; 	//activation
									memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
									cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
									memcpy(cp, &ioa, IOA_ADDRLEN);				
									cp += IOA_ADDRLEN;				
									memcpy(cp, &answer_type, sizeof(struct iec_type64));				
									cp += sizeof(struct iec_type64);				
									iecsock_prepare_iframe(c);				
									TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

									fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
									fflush(stderr);

									IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
								}

								break;
							}
						}
                        */
					}

					if(ioa == 0)
					{
						//Comando scartato
						fprintf(stderr,"Comando scartato: ItemID OPC not found\n");
						fflush(stderr);
					}
				}
				//else
				//{
				//	//Comando scartato
				//	fprintf(stderr,"Command rejected: master and slave not sincronized or  link down\n");
				//	fflush(stderr);
				//}
			}
			else if(s->type == IEC_SLAVE)
			{
				//Siamo nello SLAVE opc_client.exe ha finito di scrivere un items
				//quindi, qui invio la termination del comando al master
				
				struct iec_type64 answer_type;

				//Send activation termination
				answer_type = items->iec_obj.o.type64;
				ioa = items->iec_obj.ioa;

				answer_unit.type = C_BO_TA_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;				
				answer_unit.sq = 0;

				if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type64) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}
							
				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				answer_unit.cause = items->cause;
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type64));
				cp += sizeof(struct iec_type64);
								
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}
		}
		break;
		case M_EI_NA_1://end of initialization
		{
			if(s->type == IEC_SLAVE)
			{
                int len;				
				struct iec_buf *c;
				struct iec_unit_id answer_unit;				
				struct iec_type70 answer_type;				
				u_int ioa;	//importante: ioa da 3 ottetti deve essere u_int e non u_short					
				u_char *cp;	
				
				ioa = 0; //IOA deve essere nullo
				answer_type.coi_bs1 = 1;  //initialization after local change of parameters
				answer_type.coi_ui7 = 0; //local power switch on
				answer_unit.type = M_EI_NA_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;

                if(!items->is_neg)
				{
					answer_unit.pn = 0; //positive termination
				}
				else
				{
					answer_unit.pn = 1; //negative termination
				}

				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type70) + IOA_ADDRLEN) * answer_unit.num);				
				if (len > IEC104_ASDU_MAX)
				{
   					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);

				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x04; 	//initialised
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type70));				
				cp += sizeof(struct iec_type70);				
				iecsock_prepare_iframe(c);				
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

				fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
            }
            else if(s->type == IEC_MASTER)
            {
            
            }
		}
		break;
		case C_TS_TA_1://test command with time stamp
		{
			if(s->type == IEC_SLAVE)
			{
				//Siamo nello SLAVE: opc_client.exe ci ha inviato un replay del test command

				struct iec_unit_id answer_unit;
				struct iec_type107 answer_type;

				answer_type = items->iec_obj.o.type107;
				ioa = items->iec_obj.ioa;

				//prepara la activation confirmation

				answer_unit.type = C_TS_TA_1;

				if(n_items != 1)
				{
					fprintf(stderr,"n_items != 1 at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type107) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//confirmation
				answer_unit.cause = 0x07; //confirmation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type107));
				cp += sizeof(struct iec_type107);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

				
				fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				fflush(stderr);

				fprintf(stderr, "Value: IOA:%i fbp:%x ",
				ioa,
				answer_type.fbp);
				fflush(stderr);

				fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
				answer_type.time.hour,
				answer_type.time.min,
				answer_type.time.msec/1000,
				answer_type.time.msec%1000,
				answer_type.time.mday,
				answer_type.time.month,
				answer_type.time.year,
				answer_type.time.iv,
				answer_type.time.su);
				fflush(stderr);
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, questo test comand arriva dallo SCADA superiore
				int len;
				struct iec_unit_id answer_unit;
				struct iec_type107 answer_type;
				u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
				u_char *cp;
				
				answer_type = items->iec_obj.o.type107;
				ioa = items->iec_obj.ioa;

				//prepara la activation confirmation

				answer_unit.type = C_TS_TA_1;
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0; //positive
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;
				answer_unit.originator = 0;
							
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type107) + IOA_ADDRLEN) * answer_unit.num);

				if (len > IEC104_ASDU_MAX)
				{
					fprintf(stderr,"len > IEC104_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
					fflush(stderr);
					#ifdef WIN32
					ExitProcess(0);
                    #else
					exit(EXIT_FAILURE);
                    #endif
				}

				get_iec_buf(&c);
			
				c->data_len = len;

				cp = c->data;

				//activation
				answer_unit.cause = 0x06; //activation

				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;
								
				memcpy(cp, &ioa, IOA_ADDRLEN);
				cp += IOA_ADDRLEN;
				memcpy(cp, &answer_type, sizeof(struct iec_type107));
				cp += sizeof(struct iec_type107);
										
				iecsock_prepare_iframe(c);
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

                fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				fflush(stderr);

				fprintf(stderr, "Value: IOA:%i fbp:%x ",
				ioa,
				answer_type.fbp);
				fflush(stderr);

				fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
				answer_type.time.hour,
				answer_type.time.min,
				answer_type.time.msec/1000,
				answer_type.time.msec%1000,
				answer_type.time.mday,
				answer_type.time.month,
				answer_type.time.year,
				answer_type.time.iv,
				answer_type.time.su);
				fflush(stderr);
			}
		}
		break;
		case C_EX_IT_1:
		{
			fprintf(stderr,"Receiving exit process command\n");
			fflush(stderr);

			//exit process
			sprintf(err_msg, "Exit due to subif kill");
			iec_call_exit_handler(__LINE__,__FILE__, err_msg);
		}
		break;
		default:
		{
			fprintf(stderr,"Unsupported %d\n", items->iec_type);
			fflush(stderr);
		}
		break;
	}

	IT_EXIT;
}

//Since 23-11-2010 this function is no more used
void general_interrogation_load_points(struct iecsock *s)
{
	int points_loaded, error = 0;
	int len;
	struct iec_unit_id answer_unit;
	u_int ioa; //importante: ioa da 3 ottetti deve essere u_int e non u_short
	u_char *cp;
	struct iec_buf *c;

	IT_IT("general_interrogation_load_points");
	
	for(points_loaded = 0; point_to_load < num_iec_items; point_to_load++, points_loaded++)
	{
		if(points_loaded > points_to_load_in_a_scan)
		{
			break;
		}

		if(iec_items_table == NULL)
		{
			return;
		}

		switch(iec_items_table[point_to_load].iec_type)
		{
			case M_SP_NA_1:
			{
				//Send M_SP_NA_1
				struct iec_type1 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type1;
				
				answer_unit.type = M_SP_NA_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type1) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x14; 	//Interrogated by general interrogation <20> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type1));				
				cp += sizeof(struct iec_type1);				
				iecsock_prepare_iframe(c);				
				
				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);
				
				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_DP_NA_1:
			{
				//Send M_DP_NA_1
				struct iec_type3 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type3;

				answer_unit.type = M_DP_NA_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type3) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x14; 	//Interrogated by general interrogation <20> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type3));				
				cp += sizeof(struct iec_type3);				
				iecsock_prepare_iframe(c);				
				
				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_BO_NA_1:
			{
				//Send M_BO_NA_1
				struct iec_type7 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type7;
				
				answer_unit.type = M_BO_NA_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type7) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
				
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x14; 	//Interrogated by general interrogation <20> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type7));				
				cp += sizeof(struct iec_type7);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_ME_NA_1:
			{
				//Send M_ME_NA_1
				struct iec_type9 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type9;
				
				answer_unit.type = M_ME_NA_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type9) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x14; 	//Interrogated by general interrogation <20> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type9));				
				cp += sizeof(struct iec_type9);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_ME_NB_1:
			{
				//Send M_ME_NB_1
				struct iec_type11 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type11;
				
				answer_unit.type = M_ME_NB_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type11) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x14; 	//Interrogated by general interrogation <20> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type11));				
				cp += sizeof(struct iec_type11);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_ME_NC_1:
			{
				//Send M_ME_NC_1
				struct iec_type13 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type13;
				
				answer_unit.type = M_ME_NC_1;
				answer_unit.num = 1;
				answer_unit.sq = 0;
				answer_unit.pn = 0;
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type13) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x14; 	//Interrogated by general interrogation <20> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type13));				
				cp += sizeof(struct iec_type13);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_IT_NA_1:
			{
				//Send M_IT_NA_1
				struct iec_type15 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type15;
				
				answer_unit.type = M_IT_NA_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type15) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x14; 	//Interrogated by general interrogation <20> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type15));				
				cp += sizeof(struct iec_type15);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_SP_TB_1:
			{
				//Send M_SP_TB_1
				struct iec_type30 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type30;
											
				answer_unit.type = M_SP_TB_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type30) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x05; 	//request <5> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type30));				
				cp += sizeof(struct iec_type30);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_DP_TB_1:
			{
				//Send M_DP_TB_1
				struct iec_type31 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type31;
				
				answer_unit.type = M_DP_TB_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type31) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x05; 	//request <5> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type31));				
				cp += sizeof(struct iec_type31);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_BO_TB_1:
			{
				//Send M_BO_TB_1
				struct iec_type33 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type33;
				
				answer_unit.type = M_BO_TB_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type33) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x05; 	//request <5> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type33));				
				cp += sizeof(struct iec_type33);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_ME_TD_1:
			{
				//Send M_ME_TD_1
				struct iec_type34 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type34;
				
				answer_unit.type = M_ME_TD_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type34) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x05; 	//request <5> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type34));				
				cp += sizeof(struct iec_type34);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_ME_TE_1:
			{
				//Send M_ME_TE_1
				struct iec_type35 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type35;
				
				answer_unit.type = M_ME_TE_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type35) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x05; 	//request <5> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type35));				
				cp += sizeof(struct iec_type35);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_ME_TF_1:
			{
				//Send M_ME_TF_1
				struct iec_type36 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type36;
				
				answer_unit.type = M_ME_TF_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type36) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x05; 	//request <5> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type36));				
				cp += sizeof(struct iec_type36);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			case M_IT_TB_1:
			{
				//Send M_IT_TB_1
				struct iec_type37 answer_type;		
				
				ioa = iec_items_table[point_to_load].iec_obj.ioa;
				answer_type = iec_items_table[point_to_load].iec_obj.o.type37;
				
				answer_unit.type = M_IT_TB_1;
				answer_unit.num = 1;				
				answer_unit.sq = 0;
				answer_unit.pn = 0;				
				answer_unit.ca = common_address_of_asdu;
				answer_unit.t = 0;				
				answer_unit.originator = 0;				
				len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type37) + IOA_ADDRLEN) * answer_unit.num);
				if (len > IEC104_ASDU_MAX)
				{
					error = 1;
					break;				
				}

				get_iec_buf(&c);
			
				c->data_len = len;				
				cp = c->data;				
				answer_unit.cause = 0x25; 	//general counter interrogation <37> decimale
				memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);				
				cp += IEC_TYPEID_LEN + COM_ADDRLEN;				
				memcpy(cp, &ioa, IOA_ADDRLEN);				
				cp += IOA_ADDRLEN;				
				memcpy(cp, &answer_type, sizeof(struct iec_type37));				
				cp += sizeof(struct iec_type37);				
				iecsock_prepare_iframe(c);				

				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

				//fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//fflush(stderr);

				IT_COMMENT6("Send: Type=%d, CA=%d NUM=%i CAUSE=%i TEST=%i P/N=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause, answer_unit.t, answer_unit.pn);
				//////////////////////////////////////////////////////////////////////////////////////
			}
			break;
			default:
				fprintf(stderr,"Non supported type %d in General Interrogation\n", iec_items_table[point_to_load].iec_type);
				fflush(stderr);
			break;
		}
	}
	
	//If all point are loaded or error
	if((point_to_load == num_iec_items) || (error == 1))
	{
		//int len;				
		//struct iec_buf *c;				
		//struct iec_unit_id answer_unit;
		struct iec_type100 answer_type;
		//u_int ioa;	//importante: ioa da 3 ottetti deve essere u_int e non u_short					
		//u_char *cp;	
		ioa = 0;
		answer_type.qoi = 0x14; //Station interrogation
		answer_unit.type = C_IC_NA_1;
		answer_unit.num = 1;				
		answer_unit.sq = 0;
		if(error)
		{
			answer_unit.pn = 1;	//negative
		}
		else
		{
			answer_unit.pn = 0;	//positive
		}
		answer_unit.ca = common_address_of_asdu;				
		answer_unit.t = 0;				
		answer_unit.originator = 0;				
		len = IEC_TYPEID_LEN + COM_ADDRLEN + ((sizeof(struct iec_type100) + IOA_ADDRLEN) * answer_unit.num);
		if (len > IEC104_ASDU_MAX)
		{
			state_general_interrogation = GI_IDLE;
			IT_EXIT;
			return;
		}
		//Termination
		get_iec_buf(&c);
			
		c->data_len = len;

		cp = c->data;
		
		answer_unit.cause = 0x0A; //activation termination

		memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_ADDRLEN);
		cp += IEC_TYPEID_LEN + COM_ADDRLEN;
				
		memcpy(cp, &ioa, IOA_ADDRLEN);
		cp += IOA_ADDRLEN;
		memcpy(cp, &answer_type, sizeof(struct iec_type100));
		cp += sizeof(struct iec_type100);
						
		iecsock_prepare_iframe(c);

		TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

		//////////////////////////////////////////////////////////////////////////////////////////////////

		state_general_interrogation = GI_IDLE;
	}

	IT_EXIT;
}

int get_items_from_producer(struct iecsock *s)
{
	int items_loaded;
	struct iec_item queued_item[IEC_OBJECT_MAX];
	int i, len, jj, rc;
	const unsigned wait_limit_ms = 1;
	struct iec_item one_item;
		    
	IT_IT("get_items_from_producer");

	i = 0;

	memset(queued_item, 0x00, IEC_OBJECT_MAX*sizeof(struct iec_item));

	if(s->type == IEC_SLAVE)
	{
		//Siamo nello SLAVE, il producer dei messaggi o spontanee e' il campo, ovvero la RTU
		//Nella fifo ci sono oggetti di tipo struct iec_item
		
		items_loaded = 0;

		for(i = 0; (len = fifo_get(fifo_monitor_direction, (char*)&one_item, sizeof(struct iec_item), wait_limit_ms)) >= 0; i += 1)
		{ 
			fprintf(stderr,"Receiving %u th message \n", one_item.msg_id);
			fflush(stderr);

            //unsigned char* pt = (unsigned char*)&one_item;
			//for(j = 0; j < len; j++)
			//{ 
				//unsigned char c = *((unsigned char*)pt + j);
				//fprintf(stderr,"rx <--- 0x%02x-\n", c);
				//fflush(stderr);
				
				//IT_COMMENT1("fifo rx <--- 0x%02x-", c);
			//}

			rc = clearCrc((unsigned char *)&one_item, sizeof(struct iec_item));
			if(rc != 0)
			{
				sprintf(err_msg, "Message checksum error");
				iec_call_exit_handler(__LINE__,__FILE__,err_msg);
			}

			memcpy(queued_item + items_loaded, &one_item, sizeof(struct iec_item));

			items_loaded += 1;

			if(items_loaded >= points_to_load_in_a_scan)
			{
				break;
			}
		}
		
		if(items_loaded > IEC_OBJECT_MAX)
		{
			//error
			iec_call_exit_handler(__LINE__,__FILE__,err_msg);
		}

/*
		//This send only one item at a time

		for(jj = 0; jj < items_loaded; jj++)
		{
			send_items(s, queued_item + jj, 1);
		}
*/
		
		//Started on 25-10-2011
		//We need the pointer at the begginning of the block and the quantity of items
		//Max 16 items of M_ME_TF_1
		//Max 21 items of M_SP_TB_1
		//Max 21 items of M_DP_TB_1
		//M_IT_TB_1

		//Send all items
		{
			int groups;
			int resto;
			int kk;
			int to_send = 0;
			int current_type = 0;
			int current_cause = 0; //apa+++ 03-06-2014
			int end = 0;
			int start = 0;
			int hh;
			int max_el = 16; //apa+++ 03-06-2014

			for( ;end < items_loaded; )
			{
				for(jj = end, start = end, hh = 0; jj < items_loaded; jj++, hh++)
				{
					if(hh == 0)
					{
						current_type = (queued_item + jj)->iec_type;
						current_cause = (queued_item + jj)->cause;
						to_send = 0;
					}

					if(	((queued_item + jj)->iec_type == current_type) &&
						((queued_item + jj)->cause == current_cause)) //apa+++ 03-06-2014
					{
						to_send++;
					}
					else
					{
						break;
					}
				}

				if(current_type == M_SP_NA_1)
				{
					max_el = (int)((IEC104_ASDU_MAX - IEC_TYPEID_LEN - COM_ADDRLEN)/(sizeof(struct iec_type1) + IOA_ADDRLEN));

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == M_DP_NA_1)
				{
					max_el = (int)((IEC104_ASDU_MAX - IEC_TYPEID_LEN - COM_ADDRLEN)/(sizeof(struct iec_type3) + IOA_ADDRLEN));

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == M_ME_NC_1)
				{
					max_el = (int)((IEC104_ASDU_MAX - IEC_TYPEID_LEN - COM_ADDRLEN)/(sizeof(struct iec_type13) + IOA_ADDRLEN));

					max_el = max_el - 2; //to avoid buffer overun
				}	
				else if(current_type == M_IT_NA_1)
				{
					max_el = (int)((IEC104_ASDU_MAX - IEC_TYPEID_LEN - COM_ADDRLEN)/(sizeof(struct iec_type15) + IOA_ADDRLEN));

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == M_SP_TB_1)
				{
					max_el = (int)((IEC104_ASDU_MAX - IEC_TYPEID_LEN - COM_ADDRLEN)/(sizeof(struct iec_type30) + IOA_ADDRLEN));

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == M_DP_TB_1)
				{
					max_el = (int)((IEC104_ASDU_MAX - IEC_TYPEID_LEN - COM_ADDRLEN)/(sizeof(struct iec_type31) + IOA_ADDRLEN));

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == M_ME_TF_1)
				{
					max_el = (int)((IEC104_ASDU_MAX - IEC_TYPEID_LEN - COM_ADDRLEN)/(sizeof(struct iec_type36) + IOA_ADDRLEN));

					max_el = max_el - 2; //to avoid buffer overun
				}	
				else if(current_type == M_IT_TB_1)
				{
					max_el = (int)((IEC104_ASDU_MAX - IEC_TYPEID_LEN - COM_ADDRLEN)/(sizeof(struct iec_type37) + IOA_ADDRLEN));

					max_el = max_el - 2; //to avoid buffer overun
				}
				else
				{
					max_el = 16;
				}

				groups = (int)to_send/max_el;
				resto = to_send%max_el;

				//Send groups
				for(kk = 0; kk < groups; kk++)
				{
					send_items(s, queued_item + start, max_el);
					start = start + max_el;
				}

				//Send resto
				if(resto)
				{
					send_items(s, queued_item + start, resto);
				}

				end = start + resto;
			}
		}
	}
	else if(s->type == IEC_MASTER)
	{
		//Siamo nel MASTER, il producer dei comandi o attivazioni general interrogation e' lo SCADA superiore
		//Nella fifo ci sono oggetti di tipo struct iec_item inizializzati a comandi e activations di general interrogation.
		//Fare la fifo_put di comandi e GI in un thread o processo dello scada superiore
		for(i = 0; (len = fifo_get(fifo_control_direction, (char*)&one_item, sizeof(struct iec_item), wait_limit_ms)) >= 0; i += 1)
		{ 
			fprintf(stderr,"Receiving %d th message (command)\n", one_item.msg_id);
			fflush(stderr);

            //unsigned char* pt = (unsigned char*)&one_item;
			//for(j = 0; j < len; j++)
			//{ 
				//unsigned char c = *((unsigned char*)pt + j);
				//fprintf(stderr,"rx <--- 0x%02x-\n", c);
				//fflush(stderr);
				//IT_COMMENT1("fifo rx <--- 0x%02x-", c);
			//}

			rc = clearCrc((unsigned char *)&one_item, sizeof(struct iec_item));
			if(rc != 0)
			{
				sprintf(err_msg, "Message checksum error");
				iec_call_exit_handler(__LINE__,__FILE__,err_msg);
			}

			memcpy(queued_item, &one_item, sizeof(struct iec_item));

			//In the MASTER we send only one (1) item with one call to send_items
			send_items(s, queued_item, 1);
		}
	}

	IT_EXIT;
	return i;
}

#include <signal.h>

char* get_date_time()
{
	static char sz[128];
	time_t t = time(NULL);
	struct tm *ptm = localtime(&t);
	
	strftime(sz, sizeof(sz)-2, "%m/%d/%y %H:%M:%S", ptm);

	strcat(sz, "|");
	return sz;
}

void iec_call_exit_handler(int line, char* file, char* reason)
{
	FILE* fp;
	char program_path[_MAX_PATH];
	char log_file[_MAX_FNAME+_MAX_PATH];
	IT_IT("iec_call_exit_handler");

	program_path[0] = '\0';
#ifdef WIN32
	if(GetModuleFileName(NULL, program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\bin off path
    }
#elif __unix__
	if(getcwd(program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '/')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '/')) = '\0';        // Strip \\bin off path
    }
#endif

	strcpy(log_file, program_path);

#ifdef WIN32
	strcat(log_file, "\\logs\\iec104.log");
#elif __unix__
	strcat(log_file, "/logs/iec104.log");	
#endif

	fp = fopen(log_file, "a");

	if(fp)
	{
		if(line && file && reason)
		{
			fprintf(fp, "PID:%d time:%s exit process at line: %d, file %s, reason:%s\n", GetCurrentProcessId, get_date_time(), line, file, reason);
		}
		else if(line && file)
		{
			fprintf(fp, "PID:%d time:%s exit process at line: %d, file %s\n", GetCurrentProcessId, get_date_time(), line, file);
		}
		else if(reason)
		{
			fprintf(fp, "PID:%d time:%s exit process for reason %s\n", GetCurrentProcessId, get_date_time(), reason);
		}

		fflush(fp);
		fclose(fp);
	}

	raise(SIGABRT);   //raise abort signal which in turn starts automatically a separete thread and call iec104SignalHandler

	IT_EXIT;
}

void log_message(char* msg)
{
	FILE* fp;
	char program_path[_MAX_PATH];
	char log_file[_MAX_FNAME+_MAX_PATH];
	IT_IT("log_message");

	program_path[0] = '\0';
#ifdef WIN32
	if(GetModuleFileName(NULL, program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\bin off path
    }
#elif __unix__
	if(getcwd(program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '/')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '/')) = '\0';        // Strip \\bin off path
    }
#endif

	strcpy(log_file, program_path);

#ifdef WIN32
	strcat(log_file, "\\logs\\iec104.log");
#elif __unix__
	strcat(log_file, "/logs/iec104.log");	
#endif

	fp = fopen(log_file, "a");

	if(fp)
	{
		fprintf(fp, "PID:%d time:%s %s\n", GetCurrentProcessId, get_date_time(), msg);
		fflush(fp);
		fclose(fp);
	}

	IT_EXIT;
}

void __cdecl iec104SignalHandler(int signal)
{
	struct iec_item item_to_send;
	IT_IT("iec104SignalHandler");

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//send a packet to superior SCADA using FIFO fifo_monitor_direction to indicate that MASTER is closing
	memset(&item_to_send,0x00, sizeof(struct iec_item));
	item_to_send.iec_type = C_LO_ST_1; //Tell consumer (Control Center) we are closing
	send_item_to_superior_scada(&item_to_send);
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	switch(signal)
	{
		case SIGTERM:
			event_loopbreak(); //ferma immeditamente il loop iec 104 se è attivo
			Sleep(2000);
			deinitialize_win32_socket();
			fifo_close(fifo_monitor_direction);
			fifo_close(fifo_control_direction);
			IT_COMMENT("Termination signal received!");
		break;
#ifdef WIN32
		case SIGBREAK: ////Ctrl-Break
			event_loopbreak(); //ferma immeditamente il loop iec 104 se è attivo
			Sleep(2000);
			deinitialize_win32_socket();
			fifo_close(fifo_monitor_direction);
			fifo_close(fifo_control_direction);
			IT_COMMENT("Break signal received!");
		break;
#endif
		case SIGABRT: //generato da una chimata di abort(); raise();
			event_loopbreak(); //ferma immeditamente il loop iec 104 se è attivo
			Sleep(2000);
			deinitialize_win32_socket();
			fifo_close(fifo_monitor_direction);
			fifo_close(fifo_control_direction);
			IT_COMMENT("Abort signal received!");
		break;
		case SIGFPE:
			IT_COMMENT("Floating point error signal received!");
		break;
		case SIGSEGV:
			IT_COMMENT("Segmentation fault error signal received!");
		break;
		case SIGILL:
			IT_COMMENT("Segmentation fault error signal received!");
		break;
		case SIGINT: //Ctrl-C
			event_loopbreak(); //ferma immeditamente il loop iec 104 se è attivo
			deinitialize_win32_socket();
			Sleep(3000);
			fifo_close(fifo_monitor_direction);
			fifo_close(fifo_control_direction);
			IT_COMMENT("Ctrl - C signal");
		break;
		default:
			IT_COMMENT("Unknown signal received!");
		break;
	}

	#ifdef WIN32
	ExitProcess(0); //apa+++ 05-12-2010 
    #else
	exit(EXIT_FAILURE);
    #endif 

	IT_EXIT;
}

int load_config_ini(char* numero_linea)
{
	char pInitFile[500 + 1];
	char pItem[500+1];
	int ret;
	char CommonAddressOfAsdu[80];
//	char SelectBeforeOperate[80];
	char program_path[_MAX_PATH];
#ifdef __unix__
                  gchar * gc = NULL;
	GKeyFile *gfile = NULL;
                  GKeyFileFlags flags;
#endif

	IT_IT("load_config_ini");

	program_path[0] = '\0';

#ifdef WIN32
	if(GetModuleFileName(NULL, program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\bin off path
    }
	strcpy(pInitFile, program_path);
	strcat(pInitFile, "\\cfg\\config.ini");

	if(strlen(numero_linea) == 0)
	{
		//we are on SLAVE side
		strcpy(file_configurazione, program_path);
		strcat(file_configurazione,"\\cfg\\file_configurazione.csv");

		strcpy(punti_opc_pg, program_path);
		strcat(punti_opc_pg, "\\cfg\\punti_opc_pg_da.csv");

		strcpy(file_trasferito, program_path);
		strcat(file_trasferito, "\\cfg\\file_trasferito.csv");

		strcpy(file_transf_working_dir, program_path);
		strcat(file_transf_working_dir, "\\ftwd");
    }
	else
	{
		//we are on MASTER side
		strcat(program_path, numero_linea);
				
		strcpy(file_configurazione, program_path);
		strcat(file_configurazione,"\\cfg\\file_configurazione.csv");

		strcpy(punti_opc_pg, program_path);
		strcat(punti_opc_pg, "\\cfg\\punti_opc_pg_da.csv");

		strcpy(file_trasferito, program_path);
		strcat(file_trasferito, "\\cfg\\file_trasferito.csv");

		strcpy(file_transf_working_dir, program_path);
		strcat(file_transf_working_dir, "\\ftwd");
    }
#endif
#ifdef __unix__
	getcwd(program_path, _MAX_PATH);
        
	strcpy(pInitFile, program_path);
	strcat(pInitFile, "/cfg/config.ini");

	if(strlen(numero_linea) == 0)
	{
		//we are on SLAVE side
		strcpy(file_configurazione, program_path);
		strcat(file_configurazione,"/cfg/file_configurazione.csv");

		strcpy(punti_opc_pg, program_path);
		strcat(punti_opc_pg, "/cfg/punti_opc_pg_da.csv");

		strcpy(file_trasferito, program_path);
		strcat(file_trasferito, "/cfg/file_trasferito.csv");

		strcpy(file_transf_working_dir, program_path);
		strcat(file_transf_working_dir, "/ftwd");
    }
	else
	{
		//we are on MASTER side
		strcat(program_path, numero_linea);
				
		strcpy(file_configurazione, program_path);
		strcat(file_configurazione,"/cfg/file_configurazione.csv");

		strcpy(punti_opc_pg, program_path);
		strcat(punti_opc_pg, "/cfg/punti_opc_pg_da.csv");

		strcpy(file_trasferito, program_path);
		strcat(file_trasferito, "/cfg/file_trasferito.csv");

		strcpy(file_transf_working_dir, program_path);
		strcat(file_transf_working_dir, "/ftwd");
    }
#endif


	sprintf(pItem,"Settings");
	
#ifdef WIN32
	ret = GetPrivateProfileString(pItem,"CommonAddressOfAsdu","",CommonAddressOfAsdu,79,pInitFile);
//	ret = GetPrivateProfileString(pItem,"SelectBeforeOperate","",SelectBeforeOperate,79,pInitFile);
#endif
#ifdef __unix__
                  gfile = g_key_file_new();
                  flags = G_KEY_FILE_KEEP_COMMENTS;
                  
	fprintf(stderr,"Ini-File: %s\n", pInitFile);
	fflush(stderr);
	
                  ret = g_key_file_load_from_file(gfile, pInitFile, flags, NULL);
                  gc = g_key_file_get_string(gfile, pItem, "CommonAddressOfAsdu", NULL);
                  strcat(CommonAddressOfAsdu, gc);
#endif

	if(ret == 0 || strlen(CommonAddressOfAsdu) == 0)
	{
		fprintf(stderr,"File '%s' not found\n", pInitFile);
		fflush(stderr);
		IT_EXIT;
		return 1;
	}

//	common_address_of_asdu = atoi(CommonAddressOfAsdu); //Since 17-12-2011 CASDU is loaded as command line parameter
//	select_before_operate = atoi(SelectBeforeOperate);

	IT_EXIT;
	return 0;
}


int alloc_queues(void)
{
	int i;

	IT_IT("alloc_queues");

	for(i = 0; i < IEC_104_MAX_EVENTS; i++)
	{
		v_iec_buf[i].c = (struct iec_buf*)calloc(1, IEC104_BUF_LEN);

		if(v_iec_buf[i].c == NULL)
		{
			fprintf(stderr, "Failed to allocate iec 104 queues\n");
			fflush(stderr);
			IT_EXIT;
			return 1;
		}

		v_iec_buf[i].used = 0;
	}

	IT_EXIT;
	return 0;
}


void send_item_to_superior_scada(struct iec_item* p_queued_item)
{
	IT_IT("send_item_to_superior_scada");
	
    //Send message to superior SCADA
    p_queued_item->checksum = clearCrc((unsigned char *)p_queued_item, sizeof(struct iec_item));
	fifo_put(fifo_monitor_direction, (char *)p_queued_item, sizeof(struct iec_item));
	//////////////////////////////////////////////////////////////////////////////
	IT_EXIT;
}

void reset_state_machines(void)
{
	//apa 08-05-10 init all state machines///////
	state_database = DB_STARTUP; //Forza un caricamento del database alla riconnessione
	state_file_transfer = FT_IDLE;
	state_gateway = GW_STARTUP;
	state_general_interrogation = GI_IDLE;
	state_monitoring_direction = MSG_IDLE;
	state_control_direction = CMD_IDLE;
	state_clock_synchronisation = CLK_IDLE;
	state_tcp_link = LNK_IDLE;
	state_of_slave = SLAVE_NOT_INITIALIZED;
	//////////////////////////////////////////////
}

void set_link_state(void)
{
	state_tcp_link = LNK_CONNECTED;
}

void clearing_ftwd(void)
{
	char ftwd_conf[_MAX_PATH + _MAX_FNAME];

	IT_IT("clearing_ftwd");

	//Remove file_configurazione in file transfer working directory, if present
	fprintf(stderr,"Clearing ftwd directory\n");
	fflush(stderr);
	
	strcpy(ftwd_conf, file_transf_working_dir);
#ifdef WIN32
	strcat(ftwd_conf, "\\file_configurazione.csv");
#endif
#ifdef __unix__
	strcat(ftwd_conf, "/file_configurazione.csv");
#endif

	if(remove(ftwd_conf) == -1)
	{
		fprintf(stderr,"Unable to remove file %s\n", ftwd_conf);
		fflush(stderr);
	}
	else
	{
		fprintf(stderr,"Removed file %s\n", ftwd_conf);
		fflush(stderr);
	}

	IT_EXIT;
}

void clearing_logs(void)
{
	char file_configurazione[_MAX_PATH + _MAX_FNAME];
	char program_path[_MAX_PATH];
	IT_IT("clearing_ftwd");

	program_path[0] = '\0';
#ifdef WIN32
	if(GetModuleFileName(NULL, program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\bin off path
    }
#elif __unix__
	getcwd(program_path, _MAX_PATH);
#endif

	//Remove file_configurazione in file transfer working directory, if present
	fprintf(stderr,"Clearing cfg directory\n");
	fflush(stderr);

	strcpy(file_configurazione, program_path);
#ifdef WIN32
	strcat(file_configurazione,"\\cfg\\file_configurazione.csv");
#elif __unix__
	strcat(file_configurazione,"/cfg/file_configurazione.csv");
#endif

	if(remove(file_configurazione) == -1)
	{
		fprintf(stderr,"Unable to remove file %s\n", file_configurazione);
		fflush(stderr);
	}
	else
	{
		fprintf(stderr,"Removed file %s\n", file_configurazione);
		fflush(stderr);
	}

	IT_EXIT;
}

void send_lost_packet(void)
{
	struct iec_item item_to_send;
	IT_IT("send_lost_packet");

	//send a packet to superior SCADA using FIFO fifo_monitor_direction to indicate that MASTER is disconnected or any other problem (misconfiguration)
	memset(&item_to_send,0x00, sizeof(struct iec_item));
	item_to_send.iec_type = C_LO_ST_1; //Tell consumer (Control Center) we have LOST connection with SLAVE
	send_item_to_superior_scada(&item_to_send);

	IT_EXIT;
	return;
}

void set_fifo_monitor_dir_handle(fifo_h f_m_d)
{
	fifo_monitor_direction = f_m_d;
}

void set_fifo_control_dir_handle(fifo_h f_c_d)
{
	fifo_control_direction = f_c_d;
}

/*
int init_fifos(char* fifo_monitor_direction_name, char* fifo_control_direction_name)
{
	int rt;
	
	IT_IT("init_fifos");

	//Init thread shared fifos
												//default names
	rt = strlen(fifo_monitor_direction_name);	//"fifo_monitor_direction"
	rt += strlen(fifo_control_direction_name);	//"fifo_control_direction"

	if(rt == 0)
	{
		fprintf(stderr,"Fifo names not known\n");
		fflush(stderr);
		IT_EXIT;
		return 1;
	}

	if(strlen(fifo_monitor_direction_name) > 0)
	{
		fifo_monitor_direction = fifo_open(fifo_monitor_direction_name, MAX_FIFO_SIZE_MONITOR_DIRECTION, iec_call_exit_handler);

		if(fifo_monitor_direction == NULL)
		{
			fprintf(stderr,"fifo_monitor_direction == NULL\n");
			fflush(stderr);
			IT_EXIT;
			return 1;
		}
	}

	fifo_control_direction = fifo_open(fifo_control_direction_name, MAX_FIFO_SIZE_CONTROL_DIRECTION, iec_call_exit_handler);

	if(fifo_control_direction == NULL)
	{
		fprintf(stderr,"fifo_control_direction == NULL\n");
		fflush(stderr);
		IT_EXIT;
		return 1;
	}

	IT_EXIT;
	return 0;
}
*/