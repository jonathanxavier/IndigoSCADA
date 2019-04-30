/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2014 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#include <glib-2.0/glib.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
#include "queue.h"
#include "clear_crc_eight.h"
#include "iec103.h"
#include "iecserial.h"
#include "iec103_types.h"
#include "iec_103_item.h"
#include "itrace.h"
#include <sys/stat.h>
#include "link_unbalance_master.h"
#include "fifoc.h"

#ifdef WIN32
#include <io.h>
#endif
#ifdef __unix__
#include <sys/io.h>
#include <dirent.h>
#include <sys/time.h>
#include <portable.h>
#endif

//Control direction: SCADA-------------> RTU
//Monitor direction: SCADA<------------- RTU


/////globals//////////////////////////////////////////////////////////////////////////////////
u_char gl_common_address_of_asdu = 0;
u_char gl_link_address = 0;
u_short gl_master_polling_time_in_milliseconds = 500;
u_short gl_slave_polling_time_in_milliseconds = 10;
int gl_read_timeout_ms = 2000;
char slave_port_name[80];
char err_msg[100];
FILE *iec_protocol_log_stream = NULL;
char iec_optional_log_file_name[80] = {0};
int gl_rtsOnTime = 0; //ms
int gl_rtsOffTime = 0; //ms
double measurand_min = 0.0; //minimum in engineering unit for all measurands type 3 and type 9
double measurand_max = 0.0; //maximum in engineering unit for all measurands type 3 and type 9
double disturbance_min = 0.0; //minimum of disturbance values in engineering unit
double disturbance_max = 0.0; //maximum of disturbance values in engineering unit

//Interface to field//////////////////////////////////////////////////////////////////////////
//Multithread shared fifo
static fifo_h fifo_control_direction; //fifo in control direction: SCADA-------------> RTU DA server if s->type == IEC_SLAVE
static fifo_h fifo_monitor_direction; //fifo in monitor direction: SCADA<------------- RTU

//////////////////State machines///////////////////////////////////////////
static int state_database = DB_STARTUP; //if we NEED to load the database in the iec 103 slave
static int state_file_transfer = FT_IDLE;
static int state_gateway = GW_STARTUP;
static int state_general_interrogation = GI_IDLE;
static int state_monitoring_direction = MSG_IDLE;
static int state_control_direction = CMD_IDLE;
static int state_clock_synchronisation = CLK_IDLE;
static int state_cyclic = CY_IDLE;
int state_iec_103_link = LNK_IDLE;
int state_of_slave = SLAVE_NOT_INITIALIZED;
static int state_global_logs = LOGS_STARTUP;
static int gi_scan_number = 0;
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

////////////////Database//////////////////////////////////////////////////
static time_t db_start_epoch = 0;
static time_t db_finish_epoch = 0;
#define DB_COUNT_DOWN_TIME_IN_SECONDS 3

static struct iec_103_item* iec_items_table = NULL;
static int num_iec_items = 0;
static char file_configurazione[_MAX_PATH + _MAX_FNAME];
static char punti_opc_pg[_MAX_PATH + _MAX_FNAME];

///////////////Disturbance data log//////////////////////////////////////////////////////////////////
FILE *disturbance_log_file  = NULL;
char disturbance_log_file_name[_MAX_FNAME+_MAX_PATH];

////////////Logs check timeout/////////////////////
static time_t logs_start_epoch = 0;
static time_t logs_finish_epoch = 0;
static cp56time2a logs_finish;
static cp56time2a logs_start;
static time_t logs_delta;
#define TIME_IN_SECONDS_BETWEEN_CHECKS 3600
///////////////////////////////////////////////

////////////Cyclic data/////////////////////
static time_t cyclic_start_epoch = 0;
static time_t cyclic_finish_epoch = 0;
static cp56time2a cyclic_finish;
static cp56time2a cyclic_start;
static time_t cyclic_delta;
#define TIME_IN_SECONDS_CYCLIC_PERIOD 30 // 30 s
///////////////////////////////////////////////


#define IECASDU_PARSE_FUNC(name)					\
int name(struct iec_103_object *obj, u_char *com_addr, int *n,		\
	u_char *cause, unsigned char *buf, size_t buflen)

#define iecasdu_parse_type(objp, com_addrp, np, causep, \
		bufp, buflen, type_name, struct_name) 	\
	int i;						\
	u_char *add_ioa_p;	\
	struct iec_103_unit_id *unitp;					\
	struct type_name *typep;					\
	IT_IT("iecasdu_parse_type");\
	unitp = (struct iec_103_unit_id *) bufp;		\
	*com_addrp = (*unitp).ca;	\
	add_ioa_p = (u_char *) ((u_char *) unitp + COM_103_ADDRLEN + IEC_TYPEID_LEN);	\
	typep = (struct type_name *) ((u_char *) unitp + COM_103_ADDRLEN + IEC_TYPEID_LEN + FUNC_INF);	\
	*np = unitp->num;						\
	*causep = unitp->cause;						\
	if (unitp->sq) {						\
		for (i = 0; i < unitp->num; i++, objp++, typep++) { \
            objp->fun_type = *add_ioa_p; \
            objp->inf_num = *(add_ioa_p + 1); \
			objp->o.struct_name = *typep;			\
		}							\
	} else {							\
		for (i = 0; i < unitp->num; i++, objp++) {		\
            objp->fun_type = *add_ioa_p; \
            objp->inf_num = *(add_ioa_p + 1); \
            objp->o.struct_name = *typep;			\
			add_ioa_p = (u_char *) ((u_char *) typep + sizeof(struct type_name));\
			typep = (struct type_name *) ((u_char *) typep + sizeof(struct type_name));\
		}							\
	}								


IECASDU_PARSE_FUNC(iecasdu_parse_type1)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type1, type1);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type2)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type2, type2);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type3)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type3, type3);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type4)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type4, type4);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type5)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type5, type5);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type6)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type6, type6);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type7)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type7, type7);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type8)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type8, type8);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type9)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type9, type9);

	IT_EXIT;
	return 0;
}

/*
IECASDU_PARSE_FUNC(iecasdu_parse_type10)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type10, type10);

	IT_EXIT;
	return 0;
}
*/

/*

rx <--- 0x68 START BYTE 1
rx <--- 0x25 LENGTH
rx <--- 0x25 LENGTH
rx <--- 0x68 START BYTE 2
rx <--- 0x08 LINK CONTROL FIELD
rx <--- 0x01 Link address 1
rx <--- 0x0a Type 10---------------buf+0
rx <--- 0x81 SQ 1 OBject 1 VSQ		   1
rx <--- 0x02 COT 2					   2
rx <--- 0x01 CASDU 1				   3
rx <--- 0xfe FUN 254				   4
rx <--- 0xf4 INFO 244				   5
rx <--- 0x00 RII                       6--------------
rx <--- 0x03 NGD No 3                  7
rx <--- 0x00 GIN GROUP                 8--------------
rx <--- 0x01 GIN ENTRY                 9
rx <--- 0x01 KOD 1                     10
rx <--- 0x03 GDD dataType UI           11
rx <--- 0x01 GDD dataSize 1            12
rx <--- 0x01 GDD Number 1              13
rx <--- 0x03 GID                       14--------------
rx <--- 0x01 GIN GROUP                 15--------------
rx <--- 0x01 GIN ENTRY                 16
rx <--- 0x01 KOD 1
rx <--- 0x07 GDD dataType 7 
rx <--- 0x04 GDD dataSize 4
rx <--- 0x01 GDD Number 1
rx <--- 0x05 GID
rx <--- 0x3f GID
rx <--- 0x00 GID
rx <--- 0x00 GID
rx <--- 0x02 GIN GROUP
rx <--- 0x01 GIN ENTRY
rx <--- 0x01 KOD 1
rx <--- 0x07 GDD dataType 7 
rx <--- 0x04 GDD dataSize 4
rx <--- 0x01 GDD Number 1
rx <--- 0x00 GID
rx <--- 0x05 GID
rx <--- 0x00 GID
rx <--- 0x00 GID
rx <--- 0xfe CRC
rx <--- 0x16 END


*/

int iecasdu_parse_type10(struct iec_103_object *obj, u_char *com_addr, int *n,
	u_char *cause, unsigned char *buf, size_t buflen)
{
	int j;
	u_char *add_fun_p;	
	struct iec_103_unit_id *unitp;					

	RII_NGD * rii_ngd;
	u_int gid_size_in_bytes;
	u_char* p;
	IT_IT("iecasdu_parse_type");
	unitp = (struct iec_103_unit_id *) buf;		

	*com_addr = (*unitp).ca;	

	add_fun_p  = (u_char *) ((u_char *) unitp + COM_103_ADDRLEN + IEC_TYPEID_LEN);	

	rii_ngd = (RII_NGD *) ((u_char *) unitp + COM_103_ADDRLEN + IEC_TYPEID_LEN + FUNC_INF);	
	
	*n = unitp->num; //always 1

	*cause = unitp->cause;						
        
	obj->fun_type = *add_fun_p; 
    obj->inf_num = *(add_fun_p + 1); 
	obj->o.type10.rii_ngd = *rii_ngd;

	p = buf + COM_103_ADDRLEN + IEC_TYPEID_LEN + FUNC_INF + sizeof(RII_NGD);

	for (j = 0; j < MAX_DATA_SET_IN_TYPE10; j++)
	{
		obj->o.type10.m_data_set[j].m_GID = NULL;
	}
				
	for (j = 0; j < obj->o.type10.rii_ngd.m_NGD.ngd.No; j++)
	{
		obj->o.type10.m_data_set[j].m_GIN.gin.GROUP = *p;
		obj->o.type10.m_data_set[j].m_GIN.gin.ENTRY = *(p + 1);
		obj->o.type10.m_data_set[j].m_KOD = *(p + 2);
		obj->o.type10.m_data_set[j].m_GDD.DataType = *(p + 3); 
		obj->o.type10.m_data_set[j].m_GDD.DataSize = *(p + 4); 
		obj->o.type10.m_data_set[j].m_GDD.Number = *(p + 5)&0x7F; 
		obj->o.type10.m_data_set[j].m_GDD.Cont = *(p + 5)&0x80;

		p = p + sizeof(GIN) + sizeof(u_char) + sizeof(GDD);

		//fprintf(stderr, "parsing ENTRY:%i GROUP:%i KOD:%i DataType:%i DataSize:%i Number:%i Cont:%i\n",
        //obj->o.type10.m_data_set[j].m_GIN.gin.ENTRY,
		//obj->o.type10.m_data_set[j].m_GIN.gin.GROUP,
        //obj->o.type10.m_data_set[j].m_KOD,
        //obj->o.type10.m_data_set[j].m_GDD.DataType,
        //obj->o.type10.m_data_set[j].m_GDD.DataSize,
        //obj->o.type10.m_data_set[j].m_GDD.Number,
        //obj->o.type10.m_data_set[j].m_GDD.Cont);

		gid_size_in_bytes = (obj->o.type10.m_data_set[j].m_GDD.Number)*
			(obj->o.type10.m_data_set[j].m_GDD.DataSize);
		
		obj->o.type10.m_data_set[j].m_GID = (u_char*)malloc(gid_size_in_bytes);
													
		memcpy(obj->o.type10.m_data_set[j].m_GID, p, gid_size_in_bytes);

		p = p + gid_size_in_bytes;

		//////////////////////////////////////////////test/////////////////////
		//if(obj->o.type10.m_data_set[j].m_GDD.DataType == 7)
		//{
		//	typedef union tagReal_
		//	{
		//		float val;
		//		u_char byte[4];
		//	}real_;
        //
		//	real_ a;
        //
		//	a.val = 23.123456f;
        //
		//	memcpy(obj->o.type10.m_data_set[j].m_GID, a.byte, gid_size_in_bytes);
		//}
		////////////////////////////////////////////////////////////////////////
	}

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type11)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type11, type11);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type20)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type20, type20);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type21)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type21, type21);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type23)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type23, type23);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type24)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type24, type24);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type25)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type25, type25);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type26)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type26, type26);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type27)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type27, type27);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type28)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type28, type28);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type29)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type29, type29);

	IT_EXIT;
	return 0;
}

int iecasdu_parse_type30(struct iec_103_object *obj, u_char *com_addr, int *n,
	u_char *cause, unsigned char *buf, size_t buflen)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type30, type30);

	IT_EXIT;
	return 0;
}

IECASDU_PARSE_FUNC(iecasdu_parse_type31)
{
	iecasdu_parse_type(obj, com_addr, n, cause, buf, buflen, iec_103_type31, type31);

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

int iecasdu_parse(struct iec_103_object *obj, u_char *type, u_char *com_addr, 
	int *cnt, u_char *cause, u_char *buf, size_t buflen)
{
	int ret = 0;
	struct iec_103_unit_id *unitp;

	IT_IT("iecasdu_parse");
	
	unitp = (struct iec_103_unit_id *) buf;

	switch(unitp->type) 
	{
		case TYPE_1:
			*type = TYPE_1;
			ret = iecasdu_parse_type1(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
   		case TYPE_2:
			*type = TYPE_2;
			ret = iecasdu_parse_type2(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
		case TYPE_3:
			*type = TYPE_3;
			ret = iecasdu_parse_type3(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_4:
			*type = TYPE_4;
			ret = iecasdu_parse_type4(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_5:
			*type = TYPE_5;
			ret = iecasdu_parse_type5(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_6:
			*type = TYPE_6;
			ret = iecasdu_parse_type6(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_7:
			*type = TYPE_7;
			ret = iecasdu_parse_type7(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_8:
			*type = TYPE_8;
			ret = iecasdu_parse_type8(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_9:
			*type = TYPE_9;
			ret = iecasdu_parse_type9(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_10:
			*type = TYPE_10;
			ret = iecasdu_parse_type10(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_11:
			*type = TYPE_11;
			ret = iecasdu_parse_type11(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_20:
			*type = TYPE_20;
			ret = iecasdu_parse_type20(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_21:
			*type = TYPE_21;
			ret = iecasdu_parse_type21(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_23:
			*type = TYPE_23;
			ret = iecasdu_parse_type23(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_24:
			*type = TYPE_24;
			ret = iecasdu_parse_type24(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_25:
			*type = TYPE_25;
			ret = iecasdu_parse_type25(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_26:
			*type = TYPE_26;
			ret = iecasdu_parse_type26(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_27:
			*type = TYPE_27;
			ret = iecasdu_parse_type27(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_28:
			*type = TYPE_28;
			ret = iecasdu_parse_type28(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_29:
			*type = TYPE_29;
			ret = iecasdu_parse_type29(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_30:
			*type = TYPE_30;
			ret = iecasdu_parse_type30(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
        case TYPE_31:
			*type = TYPE_31;
			ret = iecasdu_parse_type31(obj, com_addr, cnt, cause,
				buf, buflen);
		break;
		default:
			*type = unitp->type;
			ret = 2;
		break;
	}

	IT_EXIT;
	return(ret);
}

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
struct alloc_iec_buf v_iec_buf[IEC_103_MAX_EVENTS];
////////////////////////////////////////////////////////////////////////////////////////////////////////

void get_iec_buf(struct iec_buf **c)
{
	int q_i;
	
	for(q_i = 0; q_i < IEC_103_MAX_EVENTS; q_i++)
	{
		if(v_iec_buf[q_i].used == 0)
		{
			memset(v_iec_buf[q_i].c, 0x00, IEC103_BUF_LEN);
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

void general_interrogation_load_points(struct iecserial *s)
{
	int points_loaded, error = 0;

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
			case TYPE_1:
			{
		        struct iec_103_item item_to_send;
				//Azzero tutta la struttura e setto solo i campi significativi
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));

				item_to_send.iec_type = TYPE_1;
				item_to_send.cause = 0x09; 	//Interrogated by general interrogation 9 decimale
				item_to_send.iec_obj.fun_type = iec_items_table[point_to_load].iec_obj.fun_type;
				item_to_send.iec_obj.inf_num = iec_items_table[point_to_load].iec_obj.inf_num;
				        
				send_items(s, &item_to_send, 1);
			}
			break;
			case TYPE_2:
			{
				struct iec_103_item item_to_send;
				//Azzero tutta la struttura e setto solo i campi significativi
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));

				item_to_send.iec_type = TYPE_2;
				item_to_send.cause = 0x09; 	//Interrogated by general interrogation 9 decimale
				item_to_send.iec_obj.fun_type = iec_items_table[point_to_load].iec_obj.fun_type;
				item_to_send.iec_obj.inf_num = iec_items_table[point_to_load].iec_obj.inf_num;
				        
				send_items(s, &item_to_send, 1);
			}
			break;
			default:
				fprintf(stderr,"Non supported type %d in General Interrogation\n", iec_items_table[point_to_load].iec_type);
				fflush(stderr);
			break;
		}
	}
	
	//If all point are loaded or error
	if(point_to_load == num_iec_items)
	{
        struct iec_103_item item_to_send;
        //Azzero tutta la struttura e setto solo i campi significativi
	    memset(&item_to_send,0x00, sizeof(struct iec_103_item));

        item_to_send.iec_type = TYPE_8;
        item_to_send.cause = 10;
        item_to_send.iec_obj.fun_type = 255;
        item_to_send.iec_obj.inf_num = 0;
		item_to_send.iec_obj.o.type8.m_SCN = gi_scan_number;
        
		send_items(s, &item_to_send, 1);
		
		state_general_interrogation = GI_TERMINATION;
	}

	IT_EXIT;
}

static int test_val = 0;

void process_timer_send_frame(struct iecserial *s, void *arg)
{
	IT_IT("process_timer_send_frame");

	//fprintf(stderr,"state_cyclic = %d\n", state_cyclic);
	//fflush(stderr);

	switch(state_cyclic)
	{
		case CY_IDLE:
		{
			//if(s->type == IEC_SLAVE)
			{
				if(state_general_interrogation == GI_TERMINATION)
				{
					state_cyclic = CY_RESET_TIMER;
				}
			}
		}
		break;
		case CY_RESET_TIMER:
		{
			//if(s->type == IEC_SLAVE)
			{
				//Get UTC time
				get_utc_iec_time(&cyclic_start);
				cyclic_start_epoch = Epoch_from_cp56time2a(&cyclic_start);
				state_cyclic = CY_WAIT_FOR;	
			}
		}
		break;
		case CY_WAIT_FOR:
		{
			get_utc_iec_time(&cyclic_finish);

			cyclic_finish_epoch = Epoch_from_cp56time2a(&cyclic_finish);

			//fprintf(stderr,"finish_epoch = %lu\n", finish_epoch);
			//fflush(stderr);
			
			cyclic_delta = cyclic_finish_epoch - cyclic_start_epoch;

			//fprintf(stderr,"delta = %lu\n", delta);
			//fflush(stderr);

			if(cyclic_delta > TIME_IN_SECONDS_CYCLIC_PERIOD)
			{
				state_cyclic = CY_SEND_CYCLIC_DATA;
			}
		}
		break;
		case CY_SEND_CYCLIC_DATA:
		{
			if(s->type == IEC_SLAVE)
			{
				//Send Type 10 cyclically

				////////////////Type10//////////////////////
				///////////////////////from database/////////////////
				char string[21];
				u_short Gin[4];
				u_char data_type[4];
				u_char data_size[4];
				u_char number_types[4];
				u_char m_kod[4];
				u_char cont[4];

				static u_short valore_us = 0;
				static float valore_f = 23.123456f;
				/////////////////////////////////////////////////////

                struct iec_103_item item_to_send;
				int j, k, gid_size_in_bytes;
                //Azzero tutta la struttura e setto solo i campi significativi
	            memset(&item_to_send,0x00, sizeof(struct iec_103_item));
        
                item_to_send.iec_type = TYPE_10;
                item_to_send.cause = 2; //Cyclic
                item_to_send.iec_obj.fun_type = 254;
                item_to_send.iec_obj.inf_num = 244;
                item_to_send.iec_obj.o.type10.rii_ngd.m_RII = 0;
				item_to_send.iec_obj.o.type10.rii_ngd.m_NGD.ngd.No = 4;
				item_to_send.iec_obj.o.type10.rii_ngd.m_NGD.ngd.Cont = 0;
				item_to_send.iec_obj.o.type10.rii_ngd.m_NGD.ngd.Count = 0;
				
				///////////////////////from database/////////////////
				strcpy(string, "Ciao come va oggi?..");
				Gin[0] = 0x0100;
				Gin[1] = 0x0101;
				Gin[2] = 0x0102;
				Gin[3] = 0x0103;

				data_type[0] = 3; //__UI
				data_type[1] = 7; //FLOAT
				data_type[2] = 7;
				data_type[3] = 1; //ASCII

				data_size[0] = 2;
				data_size[1] = 4;
				data_size[2] = 4;
				data_size[3] = 20;

				number_types[0] = 1;
				number_types[1] = 1;
				number_types[2] = 1;
				number_types[3] = 1;

				m_kod[0] = 1;
				m_kod[1] = 1;
				m_kod[2] = 1;
				m_kod[3] = 1;

				cont[0] = 0;
				cont[1] = 0;
				cont[2] = 0;
				cont[3] = 0;
				/////////////////////////////////////////////////////

				for (j = 0; j < item_to_send.iec_obj.o.type10.rii_ngd.m_NGD.ngd.No; j++)
				{
					item_to_send.iec_obj.o.type10.m_data_set[j].m_GIN._gin = Gin[j];
					item_to_send.iec_obj.o.type10.m_data_set[j].m_KOD = m_kod[j];
					item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.DataType = data_type[j];
					item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.DataSize = data_size[j];
					item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.Number = number_types[j];
					item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.Cont = cont[j];
					
					gid_size_in_bytes = (item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.Number)*
						(item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.DataSize);
					
					item_to_send.iec_obj.o.type10.m_data_set[j].m_GID = (u_char*)malloc(gid_size_in_bytes);

					if(item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.DataType == 1)
					{
						char *p = (char *)item_to_send.iec_obj.o.type10.m_data_set[j].m_GID;

						for(k = 0; k < item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.Number*item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.DataSize; k++, p++)
						{
							*p = string[k];
						}
					}
					else if(item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.DataType == 3)
					{
						if(item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.DataSize == 1)
						{
							u_char *p = (u_char *)item_to_send.iec_obj.o.type10.m_data_set[j].m_GID;

							for(k = 0; k < item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.Number; k++, p++)
							{
								*p = 3;
							}
						}
						else if(item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.DataSize == 2)
						{
							u_short *p = (u_short *)item_to_send.iec_obj.o.type10.m_data_set[j].m_GID;

							for(k = 0; k < item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.Number; k++, p++)
							{
								valore_us += 1;
								*p = valore_us;
							}
						}
					}
					else if(item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.DataType == 7)
					{
						float *p =	(float *)item_to_send.iec_obj.o.type10.m_data_set[j].m_GID;
						
						for(k = 0; k < item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.Number; k++, p++)
						{
							valore_f += 1.0f;
							*p = valore_f; 	
						}
						/*
						for(k = 0; k < item_to_send.iec_obj.o.type10.m_data_set[j].m_GDD.Number; k++)
						{
							typedef union tagReal_
							{
								float val;
								u_char byte[4];
							}real_;
						
							real_ a;
						
							a.val = 23.123456f;
						
							memcpy(item_to_send.iec_obj.o.type10.m_data_set[j].m_GID + k*4, a.byte, 4);
						}
						*/
					}
				}
                
			    send_items(s, &item_to_send, 1);
			
				////////////////////////////////////////////
				state_cyclic = CY_RESET_TIMER;
			}
			
			if(s->type == IEC_SLAVE)
			{
				//Send Type 3, 9 cyclically
				int i;
				#define MAX_MEASURANDS_IN_TYPE_3 4
				#define MAX_MEASURANDS_IN_TYPE_9 9

				struct iec_103_item measurand_9_item[MAX_MEASURANDS_IN_TYPE_9]; //Max 9 items
				struct iec_103_item measurand_3_item[MAX_MEASURANDS_IN_TYPE_3]; //Max 4 items

				memset(measurand_9_item, 0x00, MAX_MEASURANDS_IN_TYPE_9*sizeof(struct iec_103_item));
				memset(measurand_3_item, 0x00, MAX_MEASURANDS_IN_TYPE_3*sizeof(struct iec_103_item));

				for(i = 0; i <MAX_MEASURANDS_IN_TYPE_9; i++ )
				{
					measurand_9_item[i].iec_type = TYPE_9;
					measurand_9_item[i].cause = 0x02; 	//Cyclic
					measurand_9_item[i].iec_obj.fun_type = 160;
					measurand_9_item[i].iec_obj.inf_num = 145;
					measurand_9_item[i].iec_obj.o.type9.M.mea.MVAL = test_val%4096;
				}

				send_items(s, measurand_9_item, 9);

				for(i = 0; i <MAX_MEASURANDS_IN_TYPE_3; i++ )
				{
					measurand_3_item[i].iec_type = TYPE_3;
					measurand_3_item[i].cause = 0x02; 	//Cyclic
					measurand_3_item[i].iec_obj.fun_type = 160;
					measurand_3_item[i].iec_obj.inf_num = 144;
					measurand_3_item[i].iec_obj.o.type3.M.mea.MVAL = test_val%4096;
				}

				send_items(s, measurand_3_item, 4);

				test_val++;
			}
		}
		break;
		default:
		break;
	}

	switch(state_global_logs)
	{
		case LOGS_STARTUP:
		{
			//Get UTC time
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
                        char program_path[_MAX_PATH];

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

                        strcpy(log_file_name, program_path);
                        #ifdef WIN32
                        strcat(log_file_name, "\\logs\\");
                        #elif __unix__
                        strcat(log_file_name, "/logs/");
                        #endif
						strcat(log_file_name, iec_optional_log_file_name);
						strcat(log_file_name, log_time_stamp);
						strcat(log_file_name, ".log");
						
						iec_protocol_log_stream = freopen(log_file_name, "w", stderr);

						//Another process should remove log files older than 1 month
					}
				} 
			}

			state_global_logs = LOGS_STARTUP;
		}
		break;
		default:
		break;
	}

	//fprintf(stderr,"state_general_interrogation = %d\n", state_general_interrogation);
	//fflush(stderr);

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
		/*
		case GI_REQUEST_GI_TO_OPC:
		{
			if(s->type == IEC_SLAVE)
			{
				//The SLAVE sends to opc_client.exe the message C_IC_NA_1 (general interrogation command)

				//Send C_IC_NA_1 general interrogation command 
				char buf[sizeof(struct iec_item)];
				struct iec_item item_to_send;
				struct iec_item* p_item;
				u_int message_checksum = 0;
				int kk;

				memset(&item_to_send,0x00, sizeof(struct iec_item));

				item_to_send.iec_type = TYPE_7; //general interrogation command
				
				//Send message to ocp_client.exe ///////////////////////////////////////////////////////////////////
				memcpy(buf, &item_to_send, sizeof(struct iec_item));
				//////calculate checksum with checsum byte set to value zero////
									
				for(kk = 0;kk < sizeof(struct iec_item); kk++)
				{
					message_checksum = message_checksum + buf[kk];
				}
				p_item = (struct iec_item*)buf;
				p_item->checksum = message_checksum%256;
				////////////////////////////////////////////////////////////////
				fifo_put(fifo_control_direction, buf, sizeof(struct iec_item));
				//////////////////////////////////////////////////////////////////////////////

				state_general_interrogation = GI_REQUEST_GI_TO_OPC_DONE;
			}
		}
		break;
		*/
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
				//Get UTC time of GI activation
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
			//if(s->type == IEC_MASTER)
			//{
			//	start_epoch = 0;
			//	state_general_interrogation = GI_IDLE;
			//}
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
			
			//The SLAVE remains in the state of loading variations form field
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
			
			//The MASTER remains in the state of sending commands
		}
		break;
		default:
		break;
	}

	//fprintf(stderr,"state_database = %d\n", state_database);
	//fflush(stderr);

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
					state_database = LOAD_DATABASE;
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
								
				db = fopen(file_configurazione,"rb");

				if(db == NULL)
				{
					//Il database non e' presente sul gateway
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
					//Il database non e' stato generato correttamente
					fprintf(stderr,"Number of records is < 2\n");
					fflush(stderr);
					state_database = DB_STARTUP;
					error = 1;
					goto error_LOAD_DATABASE;
				}
				
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
				//Siamo nello SLAVE, che, ad avvenuto caricamento del database, invia una TYPE_5 al MASTER
				//Send TYPE_5
				
				if(error == 0)
				{
					struct iec_103_item item_to_send;
					//Azzero tutta la struttura e setto solo i campi significativi
					memset(&item_to_send,0x00, sizeof(struct iec_103_item));
        
					item_to_send.iec_type = TYPE_5;
					item_to_send.cause = 4; 
					item_to_send.iec_obj.fun_type = 255;
					item_to_send.iec_obj.inf_num = 3;
					item_to_send.iec_obj.o.type5.m_COL = 4;
					memcpy((char*)&item_to_send.iec_obj.o.type5.m_ch1, "Enscada", 7);
                
					send_items(s, &item_to_send, 1);

					state_of_slave = SLAVE_INITIALIZED;
				}
				//////////////////////////////////////////////////////////////////////////////////////
				
			}
			
			if(error == 0)
			{
				state_database = DATABASE_LOADED;
			}
		}
		break;
		case DATABASE_LOADED:
		{

		}
		break;
		default:
		break;
	}
	
	IT_EXIT;
}


#define send_all_items(struct_name) \
		for (i = 0; i < n_ioa_in_asdu; i++) \
		{	\
			item_to_send.iec_type = type; \
            if(i == 0){ \
			item_to_send.iec_obj.fun_type = obj[i].fun_type; \
            item_to_send.iec_obj.inf_num = obj[i].inf_num; \
            } \
			item_to_send.iec_obj.o.struct_name = obj[i].o.struct_name; \
			item_to_send.msg_id = msg_sent_to_superior_scada++; \
            if(i == 0){ \
			item_to_send.func = obj[i].fun_type; \
            item_to_send.info = obj[i].inf_num; \
            } \
			item_to_send.casdu = gl_common_address_of_asdu; \
			item_to_send.cause = cause; \
			item_to_send.checksum = 0; \
			send_item_to_superior_scada(&item_to_send); \
		}

/*
 fcmp
 Copyright (c) 1998-2000 Theodore C. Belding
 University of Michigan Center for the Study of Complex Systems
 <mailto:Ted.Belding@umich.edu>
 <http://www-personal.umich.edu/~streak/>		

 This file is part of the fcmp distribution. fcmp is free software;
 you can redistribute and modify it under the terms of the GNU Library
 General Public License (LGPL), version 2 or later.  This software
 comes with absolutely no warranty. See the file COPYING for details
 and terms of copying.

 File: fcmp.c

 Description: see fcmp.h and README files.
*/

#include <math.h>

int fcmp(double x1, double x2, double epsilon) {
  int exponent;
  double delta;
  double difference;
  
  /* Get exponent(max(fabs(x1), fabs(x2))) and store it in exponent. */

  /* If neither x1 nor x2 is 0, */
  /* this is equivalent to max(exponent(x1), exponent(x2)). */

  /* If either x1 or x2 is 0, its exponent returned by frexp would be 0, */
  /* which is much larger than the exponents of numbers close to 0 in */
  /* magnitude. But the exponent of 0 should be less than any number */
  /* whose magnitude is greater than 0. */
  
  /* So we only want to set exponent to 0 if both x1 and */
  /* x2 are 0. Hence, the following works for all x1 and x2. */

  frexp(fabs(x1) > fabs(x2) ? x1 : x2, &exponent);

  /* Do the comparison. */

  /* delta = epsilon * pow(2, exponent) */

  /* Form a neighborhood around x2 of size delta in either direction. */
  /* If x1 is within this delta neighborhood of x2, x1 == x2. */
  /* Otherwise x1 > x2 or x1 < x2, depending on which side of */
  /* the neighborhood x1 is on. */
  
  delta = ldexp(epsilon, exponent); 
  
  difference = x1 - x2;

  if (difference > delta)
    return 1; /* x1 > x2 */
  else if (difference < -delta) 
    return -1;  /* x1 < x2 */
  else /* -delta <= difference <= delta */
    return 0;  /* x1 == x2 */
}

#define _EPSILON_ ((double)(2.220446E-16))

static short rescale_eng_value_to_f13(double V, double Vmin, double Vmax, int* error, int* overflow)
{
	double Amin;
	double Amax;
	double r;
	//double V; is the observed value in engineering units
	double A = 0.0; //Calculate scaled value between Amin = -4096 and Amax = 4095
	double denomin;

	IT_IT("rescale_eng_value_to_f13");

	*error = 0;

	Amin = -4096.0;
	Amax = 4095.0;

	if(((V - Vmin) > 0.0) && ((V - Vmax) < 0.0))
	{
		denomin = Vmax - Vmin;

		if(denomin > 0.0)
		{
			r = (Amax - Amin)/denomin;
			A = r*(V - Vmin) + Amin;
		}
		else
		{
			*error = 1;
		}
	}
	else if(((V - Vmin) < 0.0))
	{
		A = Amin;
		*overflow = 1;
	}
	else if(!fcmp(V, Vmin, _EPSILON_))
	{
		A = Amin;
	}
	else if(((V - Vmax) > 0.0))
	{
		A = Amax;
		*overflow = 1;
	}
	else if(!fcmp(V, Vmax, _EPSILON_))
	{
		A = Amax;
	}
	
	IT_COMMENT4("V = %lf, Vmin = %lf, Vmax = %lf, A = %lf", V, Vmin, Vmax, A);

	IT_EXIT;

	return (short)A;
}

static double get_eng_value_from_f13(double A, double Vmin, double Vmax, int* error)
{
	double Amin;
	double Amax;
	double r;
	double V; //Calculated value in engineering unit
	//double A = 0.0; //Given a scaled value between Amin = -4096 and Amax = 4095 from type3 MVAL or type9 MVAL
	double denomin;

	IT_IT("get_eng_value_from_f13");

	*error = 0;

	Amin = -4096.0;
	Amax = 4095.0;

	denomin = Vmax - Vmin;

	if(denomin > 0.0)
	{
		r = (Amax - Amin)/denomin;
		V = Vmin + (A - Amin)/r;
	}
	else
	{
		*error = 1;
	}
		
	IT_COMMENT4("V = %lf, Vmin = %lf, Vmax = %lf, A = %lf", V, Vmin, Vmax, A);

	IT_EXIT;

	return V;
}

static double from_fixed_13(short val)
{
    double d = val/4096.0; //2^12

    return d;
}

static short rescale_eng_value_to_f16(double V, double Vmin, double Vmax, int* error, int* overflow)
{
	double Amin;
	double Amax;
	double r;
	//double V; is the observed value in engineering units
	double A = 0.0; //Calculate scaled value between Amin = -32768 and Amax = 32767
	double denomin;

	IT_IT("rescale_eng_value_to_f16");

	*error = 0;

	Amin = -32768.0;
	Amax = 32767.0;

	if(((V - Vmin) > 0.0) && ((V - Vmax) < 0.0))
	{
		denomin = Vmax - Vmin;

		if(denomin > 0.0)
		{
			r = (Amax - Amin)/denomin;
			A = r*(V - Vmin) + Amin;
		}
		else
		{
			*error = 1;
		}
	}
	else if(((V - Vmin) < 0.0))
	{
		A = Amin;
		*overflow = 1;
	}
	else if(!fcmp(V, Vmin, _EPSILON_))
	{
		A = Amin;
	}
	else if(((V - Vmax) > 0.0))
	{
		A = Amax;
		*overflow = 1;
	}
	else if(!fcmp(V, Vmax, _EPSILON_))
	{
		A = Amax;
	}
	
	IT_COMMENT4("V = %lf, Vmin = %lf, Vmax = %lf, A = %lf", V, Vmin, Vmax, A);

	IT_EXIT;

	return (short)A;
}

static double get_eng_value_from_f16(double A, double Vmin, double Vmax, int* error)
{
	double Amin;
	double Amax;
	double r;
	double V; //Calculated value in engineering unit
	//double A = 0.0; //Given a scaled value between Amin = -32768 and Amax = 32767 from type30 SDV
	double denomin;

	IT_IT("get_eng_value_from_f16");

	*error = 0;

	Amin = -32768.0;
	Amax = 32767.0;

	denomin = Vmax - Vmin;

	if(denomin > 0.0)
	{
		r = (Amax - Amin)/denomin;
		V = Vmin + (A - Amin)/r;
	}
	else
	{
		*error = 1;
	}
		
	IT_COMMENT4("V = %lf, Vmin = %lf, Vmax = %lf, A = %lf", V, Vmin, Vmax, A);

	IT_EXIT;

	return V;
}


static double from_fixed_16(short val)
{
    double d = val/32768.0; //2^15

    return d;
}

static u_int msg_sent_to_superior_scada = 0;
static u_int msg_sent_in_control_direction = 0;

static time_t sbo_epoch = 0;
static time_t command_arrive_epoch = 0;

void process_data_received_hook(struct iecserial *s, struct iec_buf *b)
{
	int ret, i;
	int n_ioa_in_asdu; 
	u_char caddr;
	u_char cause, type;
	struct iec_103_object obj[IEC_OBJECT_MAX];

    struct iec_103_item item_to_send;
			
	IT_IT("process_data_received_hook");

//	fprintf(stderr, "%s: data_len=%d\n", "data_received_hook", b->data_len);
//	fflush(stderr);

	IT_COMMENT2("%s: ASDU length=%d ", "data_received_hook", b->data_len);
	
	memset(&obj, 0, IEC_OBJECT_MAX*sizeof(struct iec_103_object));
		
	ret = iecasdu_parse(obj, &type, &caddr, &n_ioa_in_asdu, &cause, 
		b->data, b->data_len);

	if(ret == 1 || ret == 2)
	{
		if(ret == 1)
		{
			fprintf(stderr,"ret %d for type %d\n", ret, type);
			fflush(stderr);
			sprintf(err_msg, "ret %d for type %d\n", ret, type);
			//iec_call_exit_handler(__LINE__,__FILE__,err_msg);
		}

		if(ret == 2)
		{
			fprintf(stderr,"type %d NOT supported!\n", type);
			fflush(stderr);
		}

		return;
	}

	fprintf(stderr, "Rec. Type=%d, CA=%d NUM=%i CAUSE=%i\n", type, caddr, n_ioa_in_asdu, cause);
	fflush(stderr);

	switch(type)
	{
		case TYPE_1:
		{
	        for (i = 0; i < n_ioa_in_asdu; i++)
            {	
		        fprintf(stderr, "Value: FUN:%i INFO:%i SIN:%i dpi:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type1.m_SIN,
                obj[i].o.type1.m_Dpi.dpi.dpi);
		        fflush(stderr);

                fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i iv %i, su %i\n",
		        obj[i].o.type1.time.hour,
		        obj[i].o.type1.time.min,
		        obj[i].o.type1.time.msec/1000,
		        obj[i].o.type1.time.msec%1000,
		        obj[i].o.type1.time.iv,
		        obj[i].o.type1.time.su);
                fflush(stderr);
	        }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type1);

                /*
                for (i = 0; i < n_ioa_in_asdu; i++) 
		        {	
			        item_to_send.iec_type = type; 
                    if(i == 0){ 
			        item_to_send.iec_obj.fun_type = obj[i].fun_type; 
                    item_to_send.iec_obj.inf_num = obj[i].inf_num; 
                    } 
			        item_to_send.iec_obj.o.type1 = obj[i].o.type1; 
			        item_to_send.msg_id = msg_sent_to_superior_scada++; 
                    if(i == 0){ 
			        item_to_send.func = obj[i].fun_type; 
                    item_to_send.info = obj[i].inf_num; 
                    } 
			        item_to_send.casdu = gl_common_address_of_asdu; 
			        item_to_send.cause = cause; 
			        item_to_send.checksum = 0; 
			        send_item_to_superior_scada(&item_to_send); 
		        }
                */
			}
		}
		break;
        case TYPE_2:
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i dpi:%i RET:%i FAN:%i SIN:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type2.m_Dpi.dpi.dpi,
                obj[i].o.type2.m_RET,
                obj[i].o.type2.m_FAN,
                obj[i].o.type2.m_SIN);
		        fflush(stderr);

                fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i iv %i, su %i\n",
		        obj[i].o.type2.time.hour,
		        obj[i].o.type2.time.min,
		        obj[i].o.type2.time.msec/1000,
		        obj[i].o.type2.time.msec%1000,
		        obj[i].o.type2.time.iv,
		        obj[i].o.type2.time.su);
                fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type2);
			}
		}
		break;
        case TYPE_3:
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {	
                if(i == 0)
                {
		            fprintf(stderr, "FUN:%i INFO:%i\n",
		            obj[i].fun_type,
                    obj[i].inf_num);
                }

				{
					//MVAL is fixed point
					//so conver to double floating point
					//double d = from_fixed_13(obj[i].o.type3.M.mea.MVAL);
					//fprintf(stderr, "MVAL:%.02lf\n", d);
					//fprintf(stderr, "MVAL:%lf\n", d);

					if(measurand_max > 0.0)
					{
						int error = 0;
						
						double V = get_eng_value_from_f13(obj[i].o.type3.M.mea.MVAL,measurand_min,measurand_max, &error);

						if(!error)
						{
							fprintf(stderr, "Value%i: ER:%i MVAL:%lf OV:%i\n",
							i + 1,
							obj[i].o.type3.M.mea.ER,
							V,
							obj[i].o.type3.M.mea.OV);
						}
						else
						{
							fprintf(stderr, "Value%i: ER:%i MVAL:error OV:%i\n",
							i + 1,
							obj[i].o.type3.M.mea.ER,
							obj[i].o.type3.M.mea.OV);
						}
					}
					else
					{
						fprintf(stderr, "Value%i: ER:%i MVAL:%d OV:%i\n",
						i + 1,
						obj[i].o.type3.M.mea.ER,
						obj[i].o.type3.M.mea.MVAL,
						obj[i].o.type3.M.mea.OV);
					}

					fflush(stderr);
				}
	        }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type3);
			}
		}
		break;
        case TYPE_4:
        {
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i SCL:%f RET:%i FAN:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type4.m_SCL, // Short circuit location
                obj[i].o.type4.m_RET, //Relative time
                obj[i].o.type4.m_FAN); //Fault number
		        fflush(stderr);

                fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i iv %i, su %i\n",
		        obj[i].o.type4.time.hour,
		        obj[i].o.type4.time.min,
		        obj[i].o.type4.time.msec/1000,
		        obj[i].o.type4.time.msec%1000,
		        obj[i].o.type4.time.iv,
		        obj[i].o.type4.time.su);
                fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type4);
			}
		}			
		break;
        case TYPE_5:
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i COL:%i %c%c%c%c%c%c%c%c%c%c%c%c\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type5.m_COL, // Compatibility level
                obj[i].o.type5.m_ch1, 
                obj[i].o.type5.m_ch2,
                obj[i].o.type5.m_ch3,
                obj[i].o.type5.m_ch4,
                obj[i].o.type5.m_ch5,
                obj[i].o.type5.m_ch6,
                obj[i].o.type5.m_ch7,
                obj[i].o.type5.m_ch8,
                obj[i].o.type5.m_ch9,
                obj[i].o.type5.m_ch10,
                obj[i].o.type5.m_ch11,
                obj[i].o.type5.m_ch12);
		        fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type5);
			}

            //PROVVISORIO
            if(s->type == IEC_MASTER)
            {   //Request General Interrogation
                //struct iec_103_item item_to_send;
                //Azzero tutta la struttura e setto solo i campi significativi
	            memset(&item_to_send,0x00, sizeof(struct iec_103_item));
        
                item_to_send.iec_type = TYPE_7;
                item_to_send.cause = 9; //GI
                item_to_send.iec_obj.fun_type = obj[i].fun_type;
                item_to_send.iec_obj.inf_num = obj[i].inf_num;
                item_to_send.iec_obj.o.type7.m_SCN = 123;
                
			    send_items(s, &item_to_send, 1);
            }
            
            //PROVVISORIO
            /*
            {
                struct iec_103_item item_to_send;
                //Azzero tutta la struttura e setto solo i campi significativi
	            memset(&item_to_send,0x00, sizeof(struct iec_103_item));
        
                item_to_send.iec_type = TYPE_20;
                item_to_send.cause = 20; //Command
                item_to_send.iec_obj.fun_type = 128;
                item_to_send.iec_obj.inf_num = 24;
                item_to_send.iec_obj.o.type20.m_DCO = 2; //Command state
                item_to_send.iec_obj.o.type20.m_RII = 33; //Return information identifier

			    send_items(s, &item_to_send, 1);
            }
            */
		}			
		break;
        case TYPE_6://clock synchronisation command
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
	            obj[i].o.type6.time.hour,
	            obj[i].o.type6.time.min,
	            obj[i].o.type6.time.msec/1000,
	            obj[i].o.type6.time.msec%1000,
	            obj[i].o.type6.time.mday,
	            obj[i].o.type6.time.month,
	            obj[i].o.type6.time.year,
	            obj[i].o.type6.time.iv,
	            obj[i].o.type6.time.su);
	            fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type6);
			}
		}			
		break;
		case TYPE_7://general interrogation command
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i SCN:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type7.m_SCN); //Scan number
	            fflush(stderr);

				gi_scan_number = obj[i].o.type7.m_SCN;
            }
			
			state_general_interrogation = GI_LOAD_POINTS;
		}			
		break;
		case TYPE_8: //End of general interrogation
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i SCN:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type8.m_SCN); //Scan number
	            fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type8);

				state_general_interrogation = GI_TERMINATION;
			}
		}			
		break;
        case TYPE_9:
		{
	        for (i = 0; i < n_ioa_in_asdu; i++)
            {	
                if(i == 0)
                {
		            fprintf(stderr, "FUN:%i INFO:%i\n",
		            obj[i].fun_type,
                    obj[i].inf_num);
                }

				{
					//MVAL is fixed point
					//so conver to double floating point
					//double d = from_fixed_13(obj[i].o.type9.M.mea.MVAL);
					//fprintf(stderr, "MVAL:%.02lf\n", d);
					//fprintf(stderr, "MVAL:%lf\n", d);

					if(measurand_max > 0.0)
					{
						int error = 0;
						
						double V = get_eng_value_from_f13(obj[i].o.type9.M.mea.MVAL,measurand_min,measurand_max, &error);

						if(!error)
						{
							fprintf(stderr, "Value%i: ER:%i MVAL:%lf OV:%i\n",
							i + 1,
							obj[i].o.type9.M.mea.ER,
							V,
							obj[i].o.type9.M.mea.OV);
						}
						else
						{
							fprintf(stderr, "Value%i: ER:%i MVAL:error OV:%i\n",
							i + 1,
							obj[i].o.type9.M.mea.ER,
							obj[i].o.type9.M.mea.OV);
						}
					}
					else
					{
						fprintf(stderr, "Value%i: ER:%i MVAL:%d OV:%i\n",
						i + 1,
						obj[i].o.type9.M.mea.ER,
						obj[i].o.type9.M.mea.MVAL,
						obj[i].o.type9.M.mea.OV);
					}

					fflush(stderr);
				}
	        }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type9);
			}
		}
		break;
        case TYPE_10: //Generic data
		{
            int j;
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i RII:%i Cont:%i Count:%i No:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type10.rii_ngd.m_RII,
                obj[i].o.type10.rii_ngd.m_NGD.ngd.Cont,
                obj[i].o.type10.rii_ngd.m_NGD.ngd.Count,
                obj[i].o.type10.rii_ngd.m_NGD.ngd.No);
	            fflush(stderr);

				//cfr 7.2.6.32 GDD or generic data description

				#define __NO_DATA 0
				#define __OS8ASCII 1
				#define __PACKEDBITSTRING 2
				#define __UI 3
				#define __I 4
				#define __UF 5
				#define __F 6
				#define __R32_23 7
				#define __R64_53 8
				#define __DOUBLE_POINT_INFORMATION 9
				#define __SINGLE_POINT_INFORMATION 10
				#define __DOUBLE_POINT_INFORMATION_WITH_TRANSIENT_AND_ERROR 11
				#define __MEASURAND_WITH_QUALITY_DESCRIPTOR 12
				#define __BINARY_TIME 14
				#define __GENERIC_IDENTIFICATION_NUMBER 15
				#define __RELATIVE_TIME 16
				#define __FUNCTION_TYPE_AND_INFORMATION_NUMBER 17
				#define __TIME_TAGGED_MESSAGE 18
				#define __TIME_TAGGED_MESSAGE_WITH_RELATIVE_TIME 19
				#define __TIME_TAGGED_MEASURAND_WITH_RELATIVE_TIME 20
				#define __EXTERNAL_TEXT_NUMBER 21
				#define __GENERIC_REPLY_CODE 22
				#define __DATA_STRUCTURE 23
				#define __INDEX 24
								               				
                for (j = 0; j < obj[i].o.type10.rii_ngd.m_NGD.ngd.No; j++)
                {
                    //fprintf(stderr, "_gin:0x%04x GIN:0x%02x%02x KOD:%i DataType:%i DataSize:%i Number:%i Cont:%i\n",
					fprintf(stderr, "GIN:0x%04x KOD:%i DataType:%i DataSize:%i Number:%i Cont:%i\n",
					obj[i].o.type10.m_data_set[j].m_GIN._gin,
                    //obj[i].o.type10.m_data_set[j].m_GIN.gin.ENTRY,
					//obj[i].o.type10.m_data_set[j].m_GIN.gin.GROUP,
                    obj[i].o.type10.m_data_set[j].m_KOD,
                    obj[i].o.type10.m_data_set[j].m_GDD.DataType,
                    obj[i].o.type10.m_data_set[j].m_GDD.DataSize,
                    obj[i].o.type10.m_data_set[j].m_GDD.Number,
                    obj[i].o.type10.m_data_set[j].m_GDD.Cont);

					switch(obj[i].o.type10.m_data_set[j].m_GDD.DataType)
					{
						case __NO_DATA:
						break;
						case __OS8ASCII:
						{
							int k;

							char *p = (char *)obj[i].o.type10.m_data_set[j].m_GID;

							for(k = 0; k < obj[i].o.type10.m_data_set[j].m_GDD.Number*obj[i].o.type10.m_data_set[j].m_GDD.DataSize; k++, p++)
							{
								fprintf(stderr, "%c", *p);
								fflush(stderr);
							}

							fprintf(stderr, "\n");
							fflush(stderr);

							free(obj[i].o.type10.m_data_set[j].m_GID);
						}
						break;
						case __UI:
						{
							int k;

							if(obj[i].o.type10.m_data_set[j].m_GDD.DataSize == 1)
							{
								u_char *p = (u_char *)obj[i].o.type10.m_data_set[j].m_GID;

								for(k = 0; k < obj[i].o.type10.m_data_set[j].m_GDD.Number; k++, p++)
								{
									fprintf(stderr, "%i ", *p);
									fflush(stderr);
								}

								fprintf(stderr, "\n");
								fflush(stderr);
							}
							else if(obj[i].o.type10.m_data_set[j].m_GDD.DataSize == 2)
							{
								u_short *p = (u_short *)obj[i].o.type10.m_data_set[j].m_GID;

								for(k = 0; k < obj[i].o.type10.m_data_set[j].m_GDD.Number; k++, p++)
								{
									fprintf(stderr, "%i ", *p);
									fflush(stderr);
								}

								fprintf(stderr, "\n");
								fflush(stderr);
							}

							free(obj[i].o.type10.m_data_set[j].m_GID);
						}
						break;
						case __R32_23:
						{
							int k;

							//typedef union tagReal_
							//{
							//	float val;
							//	u_char byte[4];
							//}real_;

							//real_ a;

							float *p =	(float *)obj[i].o.type10.m_data_set[j].m_GID;
							//u_char *p =	obj[i].o.type10.m_data_set[j].m_GID;

							for(k = 0; k < obj[i].o.type10.m_data_set[j].m_GDD.Number; k++, p++)
							//for(k = 0; k < obj[i].o.type10.m_data_set[j].m_GDD.Number*obj[i].o.type10.m_data_set[j].m_GDD.DataSize; k++, p++)
							{
								//memcpy(a.byte,p, 4);
																
								//fprintf(stderr, "%f ", a.val);
								//fflush(stderr);

								fprintf(stderr, "%f ", *p);
								fflush(stderr);
							}

							fprintf(stderr, "\n");
							fflush(stderr);

							free(obj[i].o.type10.m_data_set[j].m_GID);
						}
						break;
						/*
						case __GENERIC_IDENTIFICATION_NUMBER:
						{

						}
						break;
						*/
						case __TIME_TAGGED_MESSAGE:
						{
							int k;

							struct time_tagged_message *p =	(struct time_tagged_message *)obj[i].o.type10.m_data_set[j].m_GID;

							for(k = 0; k < obj[i].o.type10.m_data_set[j].m_GDD.Number; k++, p++)
							{
								fprintf(stderr, "Value%d: SIN:%i dpi:%i\n",
								k+1,
								p->m_SIN,
								p->m_Dpi.dpi.dpi);
								fflush(stderr);

								fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i iv %i, su %i\n",
								p->time.hour,
								p->time.min,
								p->time.msec/1000,
								p->time.msec%1000,
								p->time.iv,
								p->time.su);
								fflush(stderr);
		
							}

							free(obj[i].o.type10.m_data_set[j].m_GID);
						}
						break;
						case __TIME_TAGGED_MESSAGE_WITH_RELATIVE_TIME:
						{
							int k;

							struct time_tagged_message_with_rel_time *p = (struct time_tagged_message_with_rel_time *)obj[i].o.type10.m_data_set[j].m_GID;

							for(k = 0; k < obj[i].o.type10.m_data_set[j].m_GDD.Number; k++, p++)
							{
								fprintf(stderr, "Value%d: SIN:%i FAN:%i RET:%i dpi:%i\n",
								k+1,
								p->m_SIN,
								p->m_FAN,
								p->m_RET,
								p->m_Dpi.dpi.dpi);
								fflush(stderr);

								fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i iv %i, su %i\n",
								p->time.hour,
								p->time.min,
								p->time.msec/1000,
								p->time.msec%1000,
								p->time.iv,
								p->time.su);
								fflush(stderr);
							}

							free(obj[i].o.type10.m_data_set[j].m_GID);
						}
						break;
						case __TIME_TAGGED_MEASURAND_WITH_RELATIVE_TIME:
						{
							int k;

							struct time_tagged_measurand_with_rel_time *p =	(struct time_tagged_measurand_with_rel_time *)obj[i].o.type10.m_data_set[j].m_GID;

							for(k = 0; k < obj[i].o.type10.m_data_set[j].m_GDD.Number; k++, p++)
							{
								fprintf(stderr, "Value%d: VAL:%f FAN:%i RET:%i\n",
								k+1,
								p->val,
								p->m_FAN,
								p->m_RET);
								fflush(stderr);

								fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i iv %i, su %i\n",
								p->time.hour,
								p->time.min,
								p->time.msec/1000,
								p->time.msec%1000,
								p->time.iv,
								p->time.su);
								fflush(stderr);
							}

							free(obj[i].o.type10.m_data_set[j].m_GID);
						}
						break;
						default:
						{
							if(obj[i].o.type10.m_data_set[j].m_GID)
							{
								free(obj[i].o.type10.m_data_set[j].m_GID);
							}

							fprintf(stderr, "not supported generic data type\n");
							fflush(stderr);
						}
						break;
					}
                }
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type10);
			}
		}			
		break;
        case TYPE_11: //Generic identification
		{
            //int j;
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i RII:%i GIN:0x%02x%02x Cont:%i Count:%i No:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type11.m_RII,
                obj[i].o.type11.m_GIN.gin.ENTRY,
				obj[i].o.type11.m_GIN.gin.GROUP,
                obj[i].o.type11.m_NDE.nde.Cont,
                obj[i].o.type11.m_NDE.nde.Count,
                obj[i].o.type11.m_NDE.nde.No);
	            fflush(stderr);
               
				/*
                for (j = 0; j < obj[i].o.type11.m_NDE.nde.No; j++)
                {
                    fprintf(stderr, "KOD:%i GID:%i DataType:%i DataSize:%i Number:%i Cont:%i\n",
                    obj[i].o.type11.desc_element[j].m_KOD,
                    obj[i].o.type11.desc_element[j].m_GID,
                    obj[i].o.type11.desc_element[j].m_GDD.gdd.DataType,
                    obj[i].o.type11.desc_element[j].m_GDD.gdd.DataSize,
                    obj[i].o.type11.desc_element[j].m_GDD.gdd.Number,
                    obj[i].o.type11.desc_element[j].m_GDD.gdd.Cont);
                }
				*/
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type11);
			}
		}			
		break;
        case TYPE_20: //General command
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i DCO:%i RII:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type20.m_DCO,  //Command state
                obj[i].o.type20.m_RII); //Return information identifier
	            fflush(stderr);
/*
                //PROVVISORIO
                {
                    struct iec_103_item item_to_send;
                    //Azzero tutta la struttura e setto solo i campi significativi
	                memset(&item_to_send,0x00, sizeof(struct iec_103_item));
            
                    item_to_send.iec_type = TYPE_20;
                    item_to_send.cause = 20; //Command
                    item_to_send.iec_obj.fun_type = obj[i].fun_type;
                    item_to_send.iec_obj.inf_num = obj[i].inf_num;
                    item_to_send.iec_obj.o.type20.m_DCO = obj[i].o.type20.m_DCO; //Command state
                    item_to_send.iec_obj.o.type20.m_RII = obj[i].o.type20.m_RII; //Return information identifier

			        send_items(s, &item_to_send, 1);
                }
*/
            }
		}			
		break;
        case TYPE_21: //Generic command
		{
            int j;
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i RII:%i NOG:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type21.m_RII,
                obj[i].o.type21.m_NOG);
	            fflush(stderr);
               
                for (j = 0; j < obj[i].o.type21.m_NOG; j++)
                {
                    fprintf(stderr, "GIN:0x%04x KOD:%i\n",
                    obj[i].o.type21.m_data_set[j].m_GIN._gin,
                    obj[i].o.type21.m_data_set[j].m_KOD);
                }
            }
		}			
		break;
        case TYPE_23:
		{
	        for (i = 0; i < n_ioa_in_asdu; i++)
            {	
                if(i == 0)
                {
		            fprintf(stderr, "FUN:%i INFO:%i\n",
		            obj[i].fun_type,
                    obj[i].inf_num);
                }

                fprintf(stderr, "Value%i: FAN:%i OTEV:%i TEST:%i TM:%i TP:%i\n",
                i + 1,
                obj[i].o.type23.m_FAN,
                obj[i].o.type23.m_SOF.sof.OTEV,
                obj[i].o.type23.m_SOF.sof.TEST,
                obj[i].o.type23.m_SOF.sof.TM,
                obj[i].o.type23.m_SOF.sof.TP);

                fflush(stderr);

                fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
	            obj[i].o.type23.time.hour,
	            obj[i].o.type23.time.min,
	            obj[i].o.type23.time.msec/1000,
	            obj[i].o.type23.time.msec%1000,
	            obj[i].o.type23.time.mday,
	            obj[i].o.type23.time.month,
	            obj[i].o.type23.time.year,
	            obj[i].o.type23.time.iv,
	            obj[i].o.type23.time.su);
	            fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type23);
			}

            if(n_ioa_in_asdu > 0) //start file transfer with first FAN (i = 0)
            {
                char program_path[_MAX_PATH];
                int year;
                //struct iec_103_item item_to_send;
                i = 0;
                memset(&item_to_send,0x00, sizeof(struct iec_103_item));
		
	            item_to_send.iec_type = TYPE_24; //Order for disturbance data transmission

                item_to_send.cause = 31; //Transmission
                item_to_send.iec_obj.fun_type = obj[i].fun_type;
                item_to_send.iec_obj.inf_num = obj[i].inf_num;
                item_to_send.iec_obj.o.type24.m_ACC = 0; //Actual channel
                item_to_send.iec_obj.o.type24.m_FAN = obj[i].o.type23.m_FAN; //Fault number
                item_to_send.iec_obj.o.type24.m_TOO = 1; //Type of order: selection of fault
                item_to_send.iec_obj.o.type24.m_TOV = 1; //Type of disturbance values

                send_items(s, &item_to_send, 1);

                /////////////////////////////////////////////////////////////////////////////

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
                
                    if(obj[i].o.type23.time.year > 80)
                        year = 1900 + obj[i].o.type23.time.year;
                    else
                        year = 2000 + obj[i].o.type23.time.year;

                #ifdef WIN32
	                sprintf(disturbance_log_file_name, "%s\\logs\\Disturbance-%d-%d_%d_%d_%d_%d_%d_%d.log", 
                        program_path, 
                        obj[i].o.type23.m_FAN,
                        obj[i].o.type23.time.hour,
	                    obj[i].o.type23.time.min,
	                    obj[i].o.type23.time.msec/1000,
	                    obj[i].o.type23.time.msec%1000,
	                    year,
                        obj[i].o.type23.time.month,
                        obj[i].o.type23.time.mday);

                #elif __unix__
	                sprintf(disturbance_log_file_name, "%s/logs/Disturbance-%d-%d_%d_%d_%d_%d_%d_%d.log", 
                        program_path, 
                        obj[i].o.type23.m_FAN,
                        obj[i].o.type23.time.hour,
	                    obj[i].o.type23.time.min,
	                    obj[i].o.type23.time.msec/1000,
	                    obj[i].o.type23.time.msec%1000,
	                    year,
                        obj[i].o.type23.time.month,
                        obj[i].o.type23.time.mday);

                #endif

	                if(disturbance_log_file)
                    {
                        fclose(disturbance_log_file);
                    }
                
                    disturbance_log_file = fopen(disturbance_log_file_name, "w");

                    fprintf(disturbance_log_file, "******* Start of Disturbance Data Dump %d ********\n", obj[i].o.type23.m_FAN);
		            fflush(disturbance_log_file);

                    //TimeStamp: hh:mm:ss.uuuu YYYY-MM-DD

                    fprintf(disturbance_log_file,"TimeStamp: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
                    obj[i].o.type23.time.hour,
                    obj[i].o.type23.time.min,
                    obj[i].o.type23.time.msec/1000,
                    obj[i].o.type23.time.msec%1000,
                    obj[i].o.type23.time.mday,
                    obj[i].o.type23.time.month,
                    obj[i].o.type23.time.year,
                    obj[i].o.type23.time.iv,
                    obj[i].o.type23.time.su);
                    fflush(disturbance_log_file);

                    //Status: TP[#]TM[#]TEST[#]OTEV[#]

                    fprintf(disturbance_log_file, "Status: FAN:%i OTEV:%i TEST:%i TM:%i TP:%i\n",
                    obj[i].o.type23.m_FAN,
                    obj[i].o.type23.m_SOF.sof.OTEV,
                    obj[i].o.type23.m_SOF.sof.TEST,
                    obj[i].o.type23.m_SOF.sof.TM,
                    obj[i].o.type23.m_SOF.sof.TP);

                    fflush(disturbance_log_file);
            }
		}
		break;
        case TYPE_24: //Order for disturbance data transmission
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i TOO:%i TOV:%i FAN:%i ACC:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type24.m_TOO, //Type of order
                obj[i].o.type24.m_TOV, //Type of disturbance values
                obj[i].o.type24.m_FAN, //Fault number
                obj[i].o.type24.m_ACC);//Actual channel

	            fflush(stderr);
            }
		}			
		break;
        case TYPE_25: //Acknowledgement for disturbance data transmission
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i TOO:%i TOV:%i FAN:%i ACC:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type25.m_TOO, //Type of order
                obj[i].o.type25.m_TOV, //Type of disturbance values
                obj[i].o.type25.m_FAN, //Fault number
                obj[i].o.type25.m_ACC);//Actual channel

	            fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type25);
			}
		}			
		break;
        case TYPE_26: //Ready for transmission of disturbance data
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i TOV:%i FAN:%i NOF:%i NOC:%i NOE:%i INT:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type26.m_TOV,
	            obj[i].o.type26.m_FAN,
	            obj[i].o.type26.m_NOF,
	            obj[i].o.type26.m_NOC,
	            obj[i].o.type26.m_NOE,
	            obj[i].o.type26.m_INT);
		        fflush(stderr);

                fprintf(stderr,"Time: h:%i m:%i s:%i ms:%i iv %i, su %i\n",
		        obj[i].o.type26.time.hour,
		        obj[i].o.type26.time.min,
		        obj[i].o.type26.time.msec/1000,
		        obj[i].o.type26.time.msec%1000,
		        obj[i].o.type26.time.iv,
		        obj[i].o.type26.time.su);
                fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type26);
			}

            {
                //struct iec_103_item item_to_send;
                i = 0;
                memset(&item_to_send,0x00, sizeof(struct iec_103_item));
		
	            item_to_send.iec_type = TYPE_24; //Order for disturbance data transmission

                item_to_send.cause = 31; //Transmission
                item_to_send.iec_obj.fun_type = obj[i].fun_type;
                item_to_send.iec_obj.inf_num = obj[i].inf_num;
                item_to_send.iec_obj.o.type24.m_ACC = 0; //Actual channel
                item_to_send.iec_obj.o.type24.m_FAN = obj[i].o.type26.m_FAN; //Fault number
                item_to_send.iec_obj.o.type24.m_TOO = 2; //Type of order: request for disturbance data
                item_to_send.iec_obj.o.type24.m_TOV = obj[i].o.type26.m_TOV; //Type of disturbance values

                send_items(s, &item_to_send, 1);
            
                ////////////////////////////////////////////////////////////////////////////////
	        
	            if(disturbance_log_file)
	            {
                    fprintf(disturbance_log_file, "Type: -- Grid Fault %d: %d -- No. Of Channel: %d -- Element per Channel: %d -- Interval: %d microseconds\n",
	                obj[i].o.type26.m_FAN,
	                obj[i].o.type26.m_NOF,
	                obj[i].o.type26.m_NOC,
	                obj[i].o.type26.m_NOE,
                    obj[i].o.type26.m_INT);
                    fflush(disturbance_log_file);
	            
                    fprintf(disturbance_log_file, "Time of First Data: h:%i m:%i s:%i ms:%i iv %i, su %i\n",
		            obj[i].o.type26.time.hour,
		            obj[i].o.type26.time.min,
		            obj[i].o.type26.time.msec/1000,
		            obj[i].o.type26.time.msec%1000,
		            obj[i].o.type26.time.iv,
		            obj[i].o.type26.time.su);
                    fflush(disturbance_log_file);
	            }
            }
		}
		break;
        case TYPE_27:  //Ready for transmission of a channel
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i TOV:%i FAN:%i ACC:%i RFA:%f RPV:%f RSV:%f\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type27.m_TOV,
	            obj[i].o.type27.m_FAN,
	            obj[i].o.type27.ACC,
                obj[i].o.type27.RFA,
                obj[i].o.type27.RPV,
                obj[i].o.type27.RSV);
              
                fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type27);
			}

            {
                //struct iec_103_item item_to_send;
                i = 0;
                memset(&item_to_send,0x00, sizeof(struct iec_103_item));
		
	            item_to_send.iec_type = TYPE_24; //Order for disturbance data transmission

                item_to_send.cause = 31; //Transmission
                item_to_send.iec_obj.fun_type = obj[i].fun_type;
                item_to_send.iec_obj.inf_num = obj[i].inf_num;
                item_to_send.iec_obj.o.type24.m_ACC = obj[i].o.type27.ACC; //Actual channel
                item_to_send.iec_obj.o.type24.m_FAN = obj[i].o.type27.m_FAN; //Fault number
                item_to_send.iec_obj.o.type24.m_TOO = 8; //Type of order: Request for channell
                item_to_send.iec_obj.o.type24.m_TOV = obj[i].o.type27.m_TOV; //Type of disturbance values

                send_items(s, &item_to_send, 1);
                
                fprintf(disturbance_log_file, "**** Start of Channel %d Dump Raw Value ****\n", item_to_send.iec_obj.o.type24.m_ACC);
                fflush(disturbance_log_file);

                fprintf(disturbance_log_file, "TOV: %i -- Channel: %i -- RPV: %f -- RSV: %f -- RFA: %f\n",
                obj[i].o.type27.m_TOV,
                obj[i].o.type27.ACC,
                obj[i].o.type27.RPV,
                obj[i].o.type27.RSV,
                obj[i].o.type27.RFA);

                fflush(disturbance_log_file);
            }

            state_file_transfer = FT_CHANNEL_TRANSMISSION;
		}
		break;
        case TYPE_28: //Ready for transmission of tags
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i FAN:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type28.m_FAN);
              
                fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type28);
			}

            {
                //struct iec_103_item item_to_send;
                i = 0;
                memset(&item_to_send,0x00, sizeof(struct iec_103_item));
		
	            item_to_send.iec_type = TYPE_24; //Order for disturbance data transmission

                item_to_send.cause = 31; //Transmission
                item_to_send.iec_obj.fun_type = obj[i].fun_type;
                item_to_send.iec_obj.inf_num = obj[i].inf_num;
                item_to_send.iec_obj.o.type24.m_ACC = 0; //Actual channel
                item_to_send.iec_obj.o.type24.m_FAN = obj[i].o.type28.m_FAN; //Fault number
                item_to_send.iec_obj.o.type24.m_TOO = 16; //Type of order: request for tags
                item_to_send.iec_obj.o.type24.m_TOV = 1; //Type of disturbance values

                send_items(s, &item_to_send, 1);

                fprintf(disturbance_log_file, "**** Start of Tags Dump ****\n");
                fflush(disturbance_log_file);
            }

            state_file_transfer = FT_TAGS_TRANSMISSION;
		}
		break;
        case TYPE_29: //Transmission of tags
		{
            int j;
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i FAN:%i NOT:%i TAP:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type29.m_FAN,
                obj[i].o.type29.m_NOT,
                obj[i].o.type29.m_TAP);
	            fflush(stderr);
               
                for (j = 0; j < obj[i].o.type29.m_NOT; j++)
                {
                    fprintf(stderr, "FUN:%i INF:%i DPI:%i\n",
                    obj[i].o.type29.m_tag[j].data.m_FUN,
                    obj[i].o.type29.m_tag[j].data.m_INF,
                    obj[i].o.type29.m_tag[j].data.m_DPI.dpi.dpi);
                    fflush(stderr);

                    fprintf(disturbance_log_file, "FUN:%i INF:%i DPI:%i\n",
                    obj[i].o.type29.m_tag[j].data.m_FUN,
                    obj[i].o.type29.m_tag[j].data.m_INF,
                    obj[i].o.type29.m_tag[j].data.m_DPI.dpi.dpi);

                    fflush(disturbance_log_file);
                }
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type29);
			}
		}			
		break;
        case TYPE_30: //Transmission of disturbance values
		{
            int j;
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                //TODO on 17-05-2014 Check NFE: should start at 0 in the first block
				
				fprintf(stderr, "Value: FUN:%i INFO:%i TOV:%i FAN:%i ACC:%i NDV:%i NFE:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type30.m_TOV,
                obj[i].o.type30.m_FAN,
                obj[i].o.type30.m_ACC,
	            obj[i].o.type30.m_NDV,
	            obj[i].o.type30.m_NFE);
	            fflush(stderr);
               
                for (j = 0; j < obj[i].o.type30.m_NDV; j++)
                {
                    //SDV is fixed point
                    //so conver to double floating point
                    //double d = from_fixed_16(obj[i].o.type30.m_SDV[j]);
                    //fprintf(stderr, "SDV:%.02lf\n", d);
                    //fprintf(stderr, "SDV:%lf\n", d);
                    
					if(disturbance_max > 0.0)
					{
						int error = 0;
						
						double V = get_eng_value_from_f16(obj[i].o.type30.m_SDV[j],disturbance_min,disturbance_max, &error);

						fprintf(stderr, "SDV:%lf\n", V);
						fflush(stderr);

						fprintf(disturbance_log_file, "%lf\n", V);
						fflush(disturbance_log_file);

						if(error)
						{
							fprintf(stderr, "SDV:error\n");
							fflush(stderr);

							fprintf(disturbance_log_file, "error\n");
							fflush(disturbance_log_file);
						}
					}
					else
					{
						fprintf(stderr, "SDV:%d\n", obj[i].o.type30.m_SDV[j]);
						fflush(stderr);

						fprintf(disturbance_log_file, "%d\n", obj[i].o.type30.m_SDV[j]);
						fflush(disturbance_log_file);
					}
                }
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type30);
			}
		}			
		break;
        case TYPE_31: //End of transmission
		{
            for (i = 0; i < n_ioa_in_asdu; i++)
            {
                fprintf(stderr, "Value: FUN:%i INFO:%i TOO:%i TOV:%i FAN:%i ACC:%i\n",
		        obj[i].fun_type,
                obj[i].inf_num,
                obj[i].o.type31.m_TOO,
	            obj[i].o.type31.m_TOV,
                obj[i].o.type31.m_FAN,
                obj[i].o.type31.m_ACC);
              
                fflush(stderr);
            }

            if(s->type == IEC_MASTER)
			{
				//Send data to superior SCADA using FIFO fifo_monitor_direction
				memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				send_all_items(type31);
			}
            
            
            {
                //struct iec_103_item item_to_send;
                i = 0;

                if(obj[i].o.type31.m_TOO == 32) //End of disturbance data
                {
                    state_file_transfer = FT_END_DISTURBANCE_DATA;
                }

                //Azzero tutta la struttura e setto solo i campi significativi
	            memset(&item_to_send,0x00, sizeof(struct iec_103_item));
        
                item_to_send.iec_type = TYPE_25; //ACK for disturbance data
                item_to_send.cause = 31; //Transmission
                item_to_send.iec_obj.fun_type = obj[i].fun_type;
                item_to_send.iec_obj.inf_num = obj[i].inf_num;
                item_to_send.iec_obj.o.type25.m_ACC = obj[i].o.type31.m_ACC; //Actual channel
                item_to_send.iec_obj.o.type25.m_FAN = obj[i].o.type31.m_FAN; //Fault number

                if(state_file_transfer == FT_TAGS_TRANSMISSION)
                {
                    item_to_send.iec_obj.o.type25.m_TOO = 68; //Type of order: Tags positive

                    fprintf(disturbance_log_file, "**** End of Tags Dump ****\n");
                    fflush(disturbance_log_file);
                }
                else if(state_file_transfer == FT_CHANNEL_TRANSMISSION)
                {
                    item_to_send.iec_obj.o.type25.m_TOO = 66; //Type of order: Channell transmitted successfully

                    fprintf(disturbance_log_file, "**** End of Channel Data Dump ****\n");
                    fflush(disturbance_log_file);
                }
                else if(state_file_transfer == FT_END_DISTURBANCE_DATA)
                {
                    item_to_send.iec_obj.o.type25.m_TOO = 64; //Type of order: Disurbance data transmitted successfully

                    fprintf(disturbance_log_file, "**** End of Disturbance Data Dump ****\n");
                    fflush(disturbance_log_file);

                    fclose(disturbance_log_file);

                    disturbance_log_file = NULL;
                }

                item_to_send.iec_obj.o.type25.m_TOV = obj[i].o.type31.m_TOV; //Type of disturbance values
				                
			    send_items(s, &item_to_send, 1);
            }
            
		}
		break;
		default:
		break;
	}

	IT_EXIT;
}


void send_items(struct iecserial *s, struct iec_103_item* items, int n_items)
{
	int len, k;
	struct iec_103_unit_id answer_unit;
    unsigned char func;
    unsigned char info;
	u_char *cp;
	struct iec_buf *c;

	IT_IT("send_items");

	fprintf(stderr,"Asdu type = %d\n", items->iec_type);
	fflush(stderr);

	fprintf(stderr,"items->cause = %d\n", items->cause);
	fflush(stderr);

    //0x03 Spontaneous <3> decimal
    //0x09 Interrogated by general interrogation <9> decimal

    //I messaggi della GI si accodano nella STESSA coda dei messaggi spontanei
	//quindi rimangono cronologicamente ordianti dal caricamento stesso della coda spontaneous_q.
	//Ordine cronologico di caricamento della spontaneous_q
	//Vengono inviati prima le variazioni (o spontanee) piu' vecchie, poi i messaggi GI e poi variazioni (o spontanee) piu' recenti

	switch(items->iec_type)
	{
		case TYPE_1:
		{
			//Send TYPE_1
			struct iec_103_type1 answer_type;
		
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = TYPE_1;
			answer_unit.num = n_items; //Only one object
			answer_unit.sq = 1; //Not packed
			answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
											
			len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + sizeof(struct iec_103_type1) * answer_unit.num;
			if (len > IEC103_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type1;

                if(k == 0)
                {
				    func = (items + k)->iec_obj.fun_type;
                    info = (items + k)->iec_obj.inf_num;
                    memcpy(cp, &func, 1);				
                    cp += 1;
                    memcpy(cp, &info, 1);				
				    cp += 1;
                }

				memcpy(cp, &answer_type, sizeof(struct iec_103_type1));
				cp += sizeof(struct iec_103_type1);
			}

			iecserial_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			fflush(stderr);

			IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);

			//slave_plot_info_sp(type1);
		}
		break;
		case TYPE_2:
		{
			//Send TYPE_2
			struct iec_103_type2 answer_type;
		
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = TYPE_2;
			answer_unit.num = n_items; //Only one object
			answer_unit.sq = 1; //Not packed
			answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
											
			len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + sizeof(struct iec_103_type2) * answer_unit.num;
			if (len > IEC103_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type2;

                if(k == 0)
                {
				    func = (items + k)->iec_obj.fun_type;
                    info = (items + k)->iec_obj.inf_num;
                    memcpy(cp, &func, 1);				
                    cp += 1;
                    memcpy(cp, &info, 1);				
				    cp += 1;
                }

				memcpy(cp, &answer_type, sizeof(struct iec_103_type2));
				cp += sizeof(struct iec_103_type2);
			}

			iecserial_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			fflush(stderr);

			IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);

			//slave_plot_info_sp(type2);
		}
		break;
		case TYPE_3:
		{
			//Send TYPE_3
			struct iec_103_type3 answer_type;
					
			answer_unit.type = TYPE_3;
			answer_unit.num = n_items; //Max 4 objects
			answer_unit.sq = 0; //In sequence!
			answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
							
							
			len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + sizeof(struct iec_103_type3) * answer_unit.num;
			if (len > IEC103_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type3;

				if(k == 0)
                {
				    func = (items + k)->iec_obj.fun_type;
                    info = (items + k)->iec_obj.inf_num;
                    memcpy(cp, &func, 1);				
                    cp += 1;
                    memcpy(cp, &info, 1);				
				    cp += 1;
                }

				memcpy(cp, &answer_type, sizeof(struct iec_103_type3));
				cp += sizeof(struct iec_103_type3);
			}

			iecserial_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			fflush(stderr);

			IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			
			//slave_plot_info_sp(type3);
			
		}
		break;
		case TYPE_9:
		{
			//Send TYPE_9
			struct iec_103_type9 answer_type;
					
			answer_unit.type = TYPE_9;
			answer_unit.num = n_items; //Max 9 objects
			answer_unit.sq = 0; //In sequence!
			answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
							
			len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + sizeof(struct iec_103_type9) * answer_unit.num;
			if (len > IEC103_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type9;

				if(k == 0)
                {
				    func = (items + k)->iec_obj.fun_type;
                    info = (items + k)->iec_obj.inf_num;
                    memcpy(cp, &func, 1);				
                    cp += 1;
                    memcpy(cp, &info, 1);				
				    cp += 1;
                }

				memcpy(cp, &answer_type, sizeof(struct iec_103_type9));
				cp += sizeof(struct iec_103_type9);
			}

			iecserial_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			fflush(stderr);

			IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			
			//slave_plot_info_sp(type9);
		}
		break;
		case TYPE_20: //General command
		{
			//Send TYPE_20
			struct iec_103_type20 answer_type;
		
			////////////////////////////////////////////////////////////////////
			//
			answer_unit.type = TYPE_20;
			answer_unit.num = n_items; //Only one object
			answer_unit.sq = 1; //Not packed
			answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
							
			len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + (sizeof(struct iec_103_type20) * answer_unit.num);
			if (len > IEC103_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			for(k = 0; k < answer_unit.num; k++)
			{
				answer_type = (items + k)->iec_obj.o.type20;

				if(k == 0)
                {
				    func = (items + k)->iec_obj.fun_type;
                    info = (items + k)->iec_obj.inf_num;
                    memcpy(cp, &func, 1);				
                    cp += 1;
                    memcpy(cp, &info, 1);				
				    cp += 1;
                }

				memcpy(cp, &answer_type, sizeof(struct iec_103_type20));
				cp += sizeof(struct iec_103_type20);
			}

			iecserial_prepare_iframe(c);				
			TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			fflush(stderr);

			IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			
			//slave_plot_info_sp(type20);
		}
		break;
        case TYPE_6://clock synchronisation command
		{
			if(s->type == IEC_SLAVE)
			{
				//Nothing to do
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando di clock sync da operatore
				//Send TYPE_6
			    struct iec_103_type6 answer_type;
		    
			    ////////////////////////////////////////////////////////////////////
			    //
			    answer_unit.type = TYPE_6;
			    answer_unit.num = n_items; //Only one object
			    answer_unit.sq = 1; //Not packed
			    answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
							    
							    
			    len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + (sizeof(struct iec_103_type6) * answer_unit.num);
			    if (len > IEC103_ASDU_MAX)
			    {
				    fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			    memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			    cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			    for(k = 0; k < answer_unit.num; k++)
			    {
				    answer_type = (items + k)->iec_obj.o.type6;

                    if(k == 0)
                    {
				        func = (items + k)->iec_obj.fun_type;
                        info = (items + k)->iec_obj.inf_num;
                        memcpy(cp, &func, 1);				
                        cp += 1;
                        memcpy(cp, &info, 1);				
				        cp += 1;
                    }

				    memcpy(cp, &answer_type, sizeof(struct iec_103_type6));
				    cp += sizeof(struct iec_103_type6);
			    }

			    iecserial_prepare_iframe(c);				
			    TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

			    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			    fflush(stderr);

			    IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);

			    //slave_plot_info_sp(type6);
			    
			    //////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
		case TYPE_5://initialization
		{
			if(s->type == IEC_SLAVE)
			{
				//Send TYPE_5
			    struct iec_103_type5 answer_type;
		    
			    ////////////////////////////////////////////////////////////////////
			    //
			    answer_unit.type = TYPE_5;
			    answer_unit.num = n_items; //Only one object
			    answer_unit.sq = 1; //Not packed
			    answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
							    
							    
			    len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + sizeof(struct iec_103_type5) * answer_unit.num;
			    if (len > IEC103_ASDU_MAX)
			    {
				    fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			    memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			    cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			    for(k = 0; k < answer_unit.num; k++)
			    {
				    answer_type = (items + k)->iec_obj.o.type5;

                    if(k == 0)
                    {
				        func = (items + k)->iec_obj.fun_type;
                        info = (items + k)->iec_obj.inf_num;
                        memcpy(cp, &func, 1);				
                        cp += 1;
                        memcpy(cp, &info, 1);				
				        cp += 1;
                    }

				    memcpy(cp, &answer_type, sizeof(struct iec_103_type5));
				    cp += sizeof(struct iec_103_type5);
			    }

			    iecserial_prepare_iframe(c);				
			    TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

			    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			    fflush(stderr);

			    IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);

			    //slave_plot_info_sp(type5);
			    
			    //////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
        case TYPE_7://GI initialization
		{
			if(s->type == IEC_SLAVE)
			{
				
			}
			else if(s->type == IEC_MASTER)
			{
				//Siamo nel MASTER, ricevo comando di GI da operatore
				//Send TYPE_7
			    struct iec_103_type7 answer_type;
		    
			    ////////////////////////////////////////////////////////////////////
			    //
			    answer_unit.type = TYPE_7;
			    answer_unit.num = n_items; //Only one object
			    answer_unit.sq = 1; //Not packed
			    answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
							    
							    
			    len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + sizeof(struct iec_103_type7) * answer_unit.num;
			    if (len > IEC103_ASDU_MAX)
			    {
				    fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			    memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			    cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			    for(k = 0; k < answer_unit.num; k++)
			    {
				    answer_type = (items + k)->iec_obj.o.type7;

                    if(k == 0)
                    {
				        func = (items + k)->iec_obj.fun_type;
                        info = (items + k)->iec_obj.inf_num;
                        memcpy(cp, &func, 1);				
                        cp += 1;
                        memcpy(cp, &info, 1);				
				        cp += 1;
                    }

				    memcpy(cp, &answer_type, sizeof(struct iec_103_type7));
				    cp += sizeof(struct iec_103_type7);
			    }

			    iecserial_prepare_iframe(c);				
			    TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

			    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			    fflush(stderr);

			    IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);

			    //slave_plot_info_sp(type7);
			    
			    //////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
		case TYPE_8://GI termination
		{
			if(s->type == IEC_SLAVE)
			{
				//Send TYPE_8
			    struct iec_103_type8 answer_type;
		    
			    ////////////////////////////////////////////////////////////////////
			    //
			    answer_unit.type = TYPE_8;
			    answer_unit.num = n_items; //Only one object
			    answer_unit.sq = 1; //Not packed
			    answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
											    
			    len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + sizeof(struct iec_103_type8) * answer_unit.num;
			    if (len > IEC103_ASDU_MAX)
			    {
				    fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			    memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			    cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			    for(k = 0; k < answer_unit.num; k++)
			    {
				    answer_type = (items + k)->iec_obj.o.type8;

                    if(k == 0)
                    {
				        func = (items + k)->iec_obj.fun_type;
                        info = (items + k)->iec_obj.inf_num;
                        memcpy(cp, &func, 1);				
                        cp += 1;
                        memcpy(cp, &info, 1);				
				        cp += 1;
                    }

				    memcpy(cp, &answer_type, sizeof(struct iec_103_type8));
				    cp += sizeof(struct iec_103_type8);
			    }

			    iecserial_prepare_iframe(c);				
			    TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);

			    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			    fflush(stderr);

			    IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);

			    //slave_plot_info_sp(type8);
			    
			    //////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
		case TYPE_10://Generic data
		{
			//We are in the MASTER or SLAVE, send generic data
			//Send TYPE_10
			RII_NGD rii_ngd;
			int j, gid_size_in_bytes;
		
			answer_unit.type = TYPE_10;
			answer_unit.num = 1; //Only one object
			answer_unit.sq = 1; //Not packed
			answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
							
			get_iec_buf(&c);
							
			cp = c->data;				
			answer_unit.cause = items->cause;
			memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;
			                
			func = (items + 0)->iec_obj.fun_type;
            info = (items + 0)->iec_obj.inf_num;
            memcpy(cp, &func, 1);				
            cp += 1;
            memcpy(cp, &info, 1);				
			cp += 1;

			rii_ngd = (items + 0)->iec_obj.o.type10.rii_ngd;

			memcpy(cp, &rii_ngd, sizeof(RII_NGD));
			cp += sizeof(RII_NGD);

			len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + sizeof(RII_NGD);

			for (j = 0; j < (items + 0)->iec_obj.o.type10.rii_ngd.m_NGD.ngd.No; j++)
			{
				memcpy(cp, &((items + 0)->iec_obj.o.type10.m_data_set[j].m_GIN), sizeof(GIN));
				cp += sizeof(GIN);
				len += sizeof(GIN);

				memcpy(cp, &((items + 0)->iec_obj.o.type10.m_data_set[j].m_KOD), sizeof(u_char));
				cp += sizeof(u_char);
				len += sizeof(u_char);

				memcpy(cp, &((items + 0)->iec_obj.o.type10.m_data_set[j].m_GDD), sizeof(GDD));
				cp += sizeof(GDD);
				len += sizeof(GDD);

				gid_size_in_bytes = ((items + 0)->iec_obj.o.type10.m_data_set[j].m_GDD.Number)*
					((items + 0)->iec_obj.o.type10.m_data_set[j].m_GDD.DataSize);
				
				memcpy(cp, (items + 0)->iec_obj.o.type10.m_data_set[j].m_GID, gid_size_in_bytes);

				////////test////////////////////////////////////////////////////////
				//if((items + 0)->iec_obj.o.type10.m_data_set[j].m_GDD.DataType == 7)
				//{
				//	float* f = (float*)(items + 0)->iec_obj.o.type10.m_data_set[j].m_GID;
				//	fprintf(stderr, "test invio float %f\n", *f);
				//	fflush(stderr);
//
//						{
//						float* ff = (float*)cp;
//						fprintf(stderr, "test invio float %f\n", *ff);
//						fflush(stderr);
//						}
//
//					}
				////////////////////////////////////////////////////////////////////

				free((items + 0)->iec_obj.o.type10.m_data_set[j].m_GID);

				cp += gid_size_in_bytes;
				len += gid_size_in_bytes;
			}

			if (len > IEC103_ASDU_MAX)
			{
				fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
				fflush(stderr);
				#ifdef WIN32
				ExitProcess(0);
				#else
				exit(EXIT_FAILURE);
				#endif
			}

			c->data_len = len;				
			
			iecserial_prepare_iframe(c);

			if(s->type == IEC_SLAVE)
			{
				TAILQ_INSERT_TAIL(&s->spontaneous_q, c, head);
			}
			else if(s->type == IEC_MASTER)
			{
				TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);
			}

			fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			fflush(stderr);

			IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);

			//slave_plot_info_sp(type10);
		}
		break;
        case TYPE_24://Order for disturbance data transmission
		{
			if(s->type == IEC_SLAVE)
			{
				//Nothing to do
			}
			else if(s->type == IEC_MASTER)
			{
				//Send TYPE_24
			    struct iec_103_type24 answer_type;
		    
			    ////////////////////////////////////////////////////////////////////
			    //
			    answer_unit.type = TYPE_24;
			    answer_unit.num = n_items; //Only one object
			    answer_unit.sq = 1; //Not packed
			    answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
							    
							    
			    len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + (sizeof(struct iec_103_type24) * answer_unit.num);
			    if (len > IEC103_ASDU_MAX)
			    {
				    fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			    memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			    cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			    for(k = 0; k < answer_unit.num; k++)
			    {
				    answer_type = (items + k)->iec_obj.o.type24;

                    if(k == 0)
                    {
				        func = (items + k)->iec_obj.fun_type;
                        info = (items + k)->iec_obj.inf_num;
                        memcpy(cp, &func, 1);				
                        cp += 1;
                        memcpy(cp, &info, 1);				
				        cp += 1;
                    }

				    memcpy(cp, &answer_type, sizeof(struct iec_103_type24));
				    cp += sizeof(struct iec_103_type24);
			    }

			    iecserial_prepare_iframe(c);				
			    TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

			    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			    fflush(stderr);

			    IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);

			    //slave_plot_info_sp(type24);
			    
			    //////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
        case TYPE_25://Acknowledgement for disturbance data transmission
		{
			if(s->type == IEC_SLAVE)
			{
				//Nothing to do
			}
			else if(s->type == IEC_MASTER)
			{
				//Send TYPE_25
			    struct iec_103_type25 answer_type;
		    
			    ////////////////////////////////////////////////////////////////////
			    //
			    answer_unit.type = TYPE_25;
			    answer_unit.num = n_items; //Only one object
			    answer_unit.sq = 1; //Not packed
			    answer_unit.ca = gl_common_address_of_asdu; //== Link address in iec 103
							    
							    
			    len = IEC_TYPEID_LEN + COM_103_ADDRLEN + FUNC_INF + (sizeof(struct iec_103_type25) * answer_unit.num);
			    if (len > IEC103_ASDU_MAX)
			    {
				    fprintf(stderr,"len > IEC103_ASDU_MAX at line %d in file %s", __LINE__, __FILE__);
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
			    memcpy(cp, &answer_unit, IEC_TYPEID_LEN + COM_103_ADDRLEN);				
			    cp += IEC_TYPEID_LEN + COM_103_ADDRLEN;

			    for(k = 0; k < answer_unit.num; k++)
			    {
				    answer_type = (items + k)->iec_obj.o.type25;

                    if(k == 0)
                    {
				        func = (items + k)->iec_obj.fun_type;
                        info = (items + k)->iec_obj.inf_num;
                        memcpy(cp, &func, 1);				
                        cp += 1;
                        memcpy(cp, &info, 1);				
				        cp += 1;
                    }

				    memcpy(cp, &answer_type, sizeof(struct iec_103_type25));
				    cp += sizeof(struct iec_103_type25);
			    }

			    iecserial_prepare_iframe(c);				
			    TAILQ_INSERT_TAIL(&s->high_priority_q, c, head);

			    fprintf(stderr, "Send: Type=%d, CA=%d NUM=%i CAUSE=%i\n", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);
			    fflush(stderr);

			    IT_COMMENT4("Send: Type=%d, CA=%d NUM=%i CAUSE=%i", answer_unit.type, answer_unit.ca, answer_unit.num, answer_unit.cause);

			    //slave_plot_info_sp(type25);
			    
                //////////////////////////////////////////////////////////////////////////////////////
			}
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

int get_items_from_producer(struct iecserial *s)
{
	int items_loaded;
	struct iec_103_item queued_item[IEC_OBJECT_MAX];
	int i, len, jj, rc;
	const unsigned wait_limit_ms = 1;
	unsigned char buf[sizeof(struct iec_103_item)];
	struct iec_103_item* p_item;
		    
	IT_IT("get_items_from_producer");

	i = 0;

	memset(queued_item, 0x00, IEC_OBJECT_MAX*sizeof(struct iec_103_item));

	if(s->type == IEC_SLAVE)
	{
		//Siamo nello SLAVE, il producer dei messaggi o spontanee e' il campo, ovvero la RTU
		//Nella fifo ci sono oggetti di tipo struct iec_103_item
		
		items_loaded = 0;

		for(i = 0; (len = fifo_get(fifo_monitor_direction, (char*)buf, sizeof(struct iec_103_item), wait_limit_ms)) >= 0; i += 1)
		{ 
			p_item = (struct iec_103_item*)buf;

			fprintf(stderr,"Receiving %u th message \n", p_item->msg_id);
			fflush(stderr);

			//for(j = 0; j < len; j++)
			//{ 
				//unsigned char c = *((unsigned char*)buf + j);
				//fprintf(stderr,"rx <--- 0x%02x-\n", c);
				//fflush(stderr);
				
				//IT_COMMENT1("fifo rx <--- 0x%02x-", c);
			//}

			rc = clearCrc((unsigned char *)buf, sizeof(struct iec_103_item));
			if(rc != 0)
			{
				fprintf(stderr, "CRC8 = %d\n", rc);
				fflush(stderr);
				ExitProcess(0);
			}

			memcpy(queued_item + items_loaded, buf, sizeof(struct iec_103_item));

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


		//This send only one item at a time
//
//		for(jj = 0; jj < items_loaded; jj++)
//		{
//			send_items(s, queued_item + jj, 1);
//		}
//
		
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

				if(current_type == TYPE_1)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type1);
					
					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_2)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type2);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_3)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type3);

					max_el = max_el - 2; //to avoid buffer overun
				}	
				else if(current_type == TYPE_4)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type4);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_5)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type5);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_6)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type6);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_7)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type6);

					max_el = max_el - 2; //to avoid buffer overun
				}	
				else if(current_type == TYPE_8)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type8);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_9)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type9);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_10)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type10);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_11)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type11);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_20)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type20);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_21)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type21);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_23)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type23);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_24)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type24);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_25)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type25);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_26)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type26);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_27)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type27);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_28)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type28);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_29)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type29);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_30)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type30);

					max_el = max_el - 2; //to avoid buffer overun
				}
				else if(current_type == TYPE_31)
				{
					max_el = (int)(IEC103_ASDU_MAX - IEC_TYPEID_LEN - COM_103_ADDRLEN - FUNC_INF)/sizeof(struct iec_103_type31);

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
		//We are in the MASTER, the producer of commands or general interrogations is the superior SCADA
		//In the fifo there are objects of type struct iec_item initialized to commands or general interrogations
		
		for(i = 0; (len = fifo_get(fifo_control_direction, (char*)buf, sizeof(struct iec_103_item), wait_limit_ms)) >= 0; i += 1)
		{ 
			p_item = (struct iec_103_item*)buf;
			fprintf(stderr,"Receiving in asdu %d th message (command)\n", p_item->msg_id);
			fflush(stderr);

			//for(j = 0; j < len; j++)
			//{ 
				//unsigned char c = *((unsigned char*)buf + j);
				//fprintf(stderr,"rx <--- 0x%02x-\n", c);
				//fflush(stderr);
				//IT_COMMENT1("fifo rx <--- 0x%02x-", c);
			//}

			rc = clearCrc((unsigned char *)buf, sizeof(struct iec_103_item));
			if(rc != 0)
			{
				fprintf(stderr, "CRC8 = %d\n", rc);
				fflush(stderr);
				ExitProcess(0);
			}

			memcpy(queued_item, buf, sizeof(struct iec_103_item));
            
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
	strcat(log_file, "\\logs\\iec103.log");
#elif __unix__
	strcat(log_file, "/logs/iec103.log");	
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

	raise(SIGABRT);   //raise abort signal which in turn starts automatically a separete thread and call iec103SignalHandler

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
	strcat(log_file, "\\logs\\iec103.log");
#elif __unix__
	strcat(log_file, "/logs/iec103.log");	
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

#include "serial.h"

void __cdecl iec103SignalHandler(int signal)
{
//	struct iec_item item_to_send;
	IT_IT("iec103SignalHandler");

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//send a packet to superior SCADA using FIFO fifo_monitor_direction to indicate that MASTER is closing
	//memset(&item_to_send,0x00, sizeof(struct iec_item));
	//item_to_send.iec_type = C_LO_ST_1; //Tell consumer (Control Center) we are closing
	//send_item_to_superior_scada(&item_to_send);
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	switch(signal)
	{
		case SIGTERM:
			CloseLink();
			Sleep(2000);
			fifo_close(fifo_monitor_direction);
			fifo_close(fifo_control_direction);
			close_port((int)fd);
			IT_COMMENT("Termination signal received!");
		break;
#ifdef WIN32
        case SIGBREAK: ////Ctrl-Break
			CloseLink();
			Sleep(2000);
			fifo_close(fifo_monitor_direction);
			fifo_close(fifo_control_direction);
			close_port((int)fd);
			IT_COMMENT("Break signal received!");
		break;
#endif
		case SIGABRT: //generato da una chimata di abort(); raise();
			CloseLink();
			Sleep(2000);
			fifo_close(fifo_monitor_direction);
			fifo_close(fifo_control_direction);
			close_port((int)fd);
			IT_COMMENT("Abort signal received!");
		break;
		case SIGFPE:
			IT_COMMENT("Floating point error signal received!");
			close_port((int)fd);
		break;
		case SIGSEGV:
			IT_COMMENT("Segmentation fault error signal received!");
			close_port((int)fd);
		break;
		case SIGILL:
			IT_COMMENT("Segmentation fault error signal received!");
			close_port((int)fd);
		break;
		case SIGINT: //Ctrl-C
			CloseLink();
			Sleep(3000);
			fifo_close(fifo_monitor_direction);
			fifo_close(fifo_control_direction);
			close_port((int)fd);
			IT_COMMENT("Ctrl - C signal");
		break;
		default:
			IT_COMMENT("Unknown signal received!");
		break;
	}

	#ifdef WIN32
ExitProcess(0);
#else
exit(EXIT_FAILURE);
#endif //apa+++ 05-12-2010 

	IT_EXIT;
}

#define MAX_FIFO_SIZE 65535

int alloc_queues(void)
{
	int i;

	IT_IT("alloc_queues");

	for(i = 0; i < IEC_103_MAX_EVENTS; i++)
	{
		v_iec_buf[i].c = (struct iec_buf*)calloc(1, IEC103_BUF_LEN);

		if(v_iec_buf[i].c == NULL)
		{
			fprintf(stderr, "Failed to allocate iec 103 queues\n");
			fflush(stderr);
			IT_EXIT;
			return 1;
		}

		v_iec_buf[i].used = 0;
	}

	IT_EXIT;
	return 0;
}


void send_item_to_superior_scada(struct iec_103_item* p_queued_item)
{
	IT_IT("send_item_to_superior_scada");
		
	p_queued_item->checksum = clearCrc((unsigned char *)p_queued_item, sizeof(struct iec_103_item));
	/////////////////////////////////////////////////////////////////////////////////////
	fifo_put(fifo_monitor_direction, (char *)p_queued_item, sizeof(struct iec_103_item));
	/////////////////////////////////////////////////////////////////////////////////////

	IT_EXIT;
}

void reset_state_machines(void)
{
    IT_IT("reset_state_machines");

	//apa 08-05-10 init all state machines///////
	state_database = DB_STARTUP; //Forza un caricamento del database alla riconnessione
	state_file_transfer = FT_IDLE;
	state_gateway = GW_STARTUP;
	state_general_interrogation = GI_IDLE;
	state_monitoring_direction = MSG_IDLE;
	state_control_direction = CMD_IDLE;
	state_clock_synchronisation = CLK_IDLE;
	state_iec_103_link = LNK_IDLE;
	state_of_slave = SLAVE_NOT_INITIALIZED;
	//////////////////////////////////////////////

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
//		fprintf(stderr,"Unable to remove file %s\n", file_configurazione);
//		fflush(stderr);
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
//	struct iec_item item_to_send;
	IT_IT("send_lost_packet");

	//send a packet to superior SCADA using FIFO fifo_monitor_direction to indicate that MASTER is disconnected or any other problem (misconfiguration)
//	memset(&item_to_send,0x00, sizeof(struct iec_item));
//	item_to_send.iec_type = C_LO_ST_1; //Tell consumer (Control Center) we have LOST connection with SLAVE
//	send_item_to_superior_scada(&item_to_send);

	IT_EXIT;
	return;
}

void iec103_run_send_queues(struct iecserial *s)
{
	struct iec_buf *n1;

	IT_IT("iec103_run_send_queues");

	if(s->type == IEC_MASTER)
	{
		while(!TAILQ_EMPTY(&s->high_priority_q)) 
		{
			n1 = TAILQ_FIRST(&s->high_priority_q);
			
			TAILQ_REMOVE(&s->high_priority_q, n1, head);
						
			TAILQ_INSERT_TAIL(&s->write_q, n1, head);
		}
	}

	IT_EXIT;
}


/*
Example of single point type 30

rx <--- 0x68-
rx <--- 0x12-
rx <--- 0x12-
rx <--- 0x68-
rx <--- 0x08-
rx <--- 0x01-
rx <--- 0x1e-
rx <--- 0x01-
rx <--- 0x03-
rx <--- 0x01-
rx <--- 0x00-
rx <--- 0x64-
rx <--- 0x00-
rx <--- 0x00-
rx <--- 0x01-
rx <--- 0xd3-
rx <--- 0x51-
rx <--- 0x14-
rx <--- 0x14-
rx <--- 0x0c-
rx <--- 0x0c-
rx <--- 0x0b-
rx <--- 0x00-
rx <--- 0x16-
*/

void set_fifo_monitor_dir_handle(fifo_h f_m_d)
{
	fifo_monitor_direction = f_m_d;
}

void set_fifo_control_dir_handle(fifo_h f_c_d)
{
	fifo_control_direction = f_c_d;
}