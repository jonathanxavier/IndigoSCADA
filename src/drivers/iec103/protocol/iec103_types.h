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
#ifndef __IEC103_TYPES_H
#define __IEC103_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack(1)             /* Byte Alignment   */
#endif

#ifndef WIN32
#include "portable.h"
#endif
 
//<0> not defined
#define TYPE_1	1
#define TYPE_2	2
#define TYPE_3	3
#define TYPE_4	4
#define TYPE_5	5
#define TYPE_6	6
#define TYPE_7	7
#define TYPE_8	8
#define TYPE_9	9
#define TYPE_10	10
#define TYPE_11	11
#define TYPE_20	20
#define TYPE_21	21
#define TYPE_23	23
#define TYPE_24	24
#define TYPE_25	25
#define TYPE_26	26
#define TYPE_27	27
#define TYPE_28	28
#define TYPE_29	29
#define TYPE_30	30
#define TYPE_31	31

#define LINK_103_ADDRLEN 1	
#define COM_103_ADDRLEN	1 //Common address of ASDU length
#define FUNC_INF 2

/*
 * Basic system type definitions, taken from the BSD file sys/types.h.
 */
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
typedef unsigned __int64 u_int64;


#ifndef IEC_TIME_STRUCTS
#define IEC_TIME_STRUCTS

	/* CP56Time2a timestamp */
	typedef struct cp56time2a {
		u_short		msec;
		u_char		min	:6;
		u_char		res1	:1;
		u_char		iv	:1;
		u_char		hour	:5;
		u_char		res2	:2;
		u_char		su	:1;
		u_char		mday	:5;
		u_char		wday	:3;
		u_char		month	:4;
		u_char		res3	:4;
		u_char		year	:7;
		u_char		res4	:1;
	#ifndef WIN32
	} __attribute__((__packed__)) cp56time2a;
	#else
	}cp56time2a;
	#endif

	/* CP16time2a timestamp*/
	typedef struct cp16time2a {
		u_short		msec; /* milliseconds upto seconds */
	#ifndef WIN32
	} __attribute__((__packed__)) cp16time2a;
	#else
	}cp16time2a;
	#endif

#endif //IEC_TIME_STRUCTS

	/* CP32time2a timestamp*/
	typedef struct cp32time2a {
		u_short		msec;
		u_char		min	:6;
		u_char		res1	:1;
		u_char		iv	:1;
		u_char		hour	:5;
		u_char		res2	:2;
		u_char		su	:1;
	#ifndef WIN32
	} __attribute__((__packed__)) cp32time2a;
	#else
	}cp32time2a;
	#endif


typedef union tagNDE 
{	
	struct _tagnde
	{
		u_char No	:6;
		u_char Count:1;
		u_char Cont	:1;
	}nde;
	u_char byte;
}NDE;

typedef union tagNGD
{
	struct _tagngd
	{
		u_char No	:6;
		u_char Count:1;
		u_char Cont	:1;
	}ngd;
	u_char byte;
}NGD;

typedef struct tagGDD// P22 7.2.6.32
{
	u_char DataType;
	u_char DataSize;
	u_char Number	:7;
	u_char Cont	:1;
}GDD;

typedef union tagDPI  //7.2.6.5
{
	struct _tagDPI
	{
		u_char dpi:2;
		u_char res:6;
	}dpi;
	u_char byte;
}DPI;

typedef union tagGIN //7.2.6.31
{
	struct _tagGIN
	{
		u_char GROUP;
		u_char ENTRY;
	}gin;
	u_short _gin;
}GIN;

//7.2.6.8 Measurand with quality descriptor

//MVAL := F13[4..16]<-1..+1-2^-12>
//Letter F means positive or negative fixed point number
//F13 means 13 bits in size 

//The maximum value of a fixed-point type is 
//simply the largest value that can be represented 
//in the underlying integer type, 
//multiplied by the scaling factor; and similarly for the minimum value. 

//For example, consider a fixed-point type represented as a binary 
//integer with b=13 bits in two's complement format, 
//with a scaling factor of 1/2^f where f = 12 
//(that is, the last f=12 bits are fraction bits): 
//the minimum representable value is -2^(b-1)/2^f = -1 and
//the maximum value is (2^(b-1)-1)/2^f = +1-2^-12 .
//Min value -2^(13-1)/2^12 = -4096/4096 = -1
//Max value (2^(13-1)-1)/2^12 = 4095/4096 = 1 - 2^-12

typedef union tagMEA
{
	struct _tagMEA
	{
		u_short OV:1;
		u_short ER:1;
		u_short res:1;
		short MVAL:13; //It is a Fixed Point number F13 with sign
	}mea;
	u_short word;
}MEA;

typedef union tagSOF //7.2.6.24
{
	struct _tagSOF
	{
		u_char TP	:1;
		u_char TM	:1;
		u_char TEST	:1;
		u_char OTEV	:1;
		u_char RES	:4;
	}sof;
	u_char byte;
}SOF;

typedef struct tagDistrubDataTable
{
	u_char Addr;
	u_short FAN;
	u_char SOF;
	cp56time2a Cp56Time;
	int  TryTimes;
}DistrubDataTable;

//In monitoring direction

//Time-tagged message
struct iec_103_type1 {
    DPI     m_Dpi;
    cp32time2a time;
    u_char  m_SIN;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Time-tagged message with relative time
struct iec_103_type2 {
	DPI     m_Dpi;
	u_short m_RET;
    u_short m_FAN;
    cp32time2a time;
    u_char  m_SIN;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Measurands I
struct iec_103_type3 {
    MEA     M;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Time-tagged measurands with relative time
struct iec_103_type4 {
    float   m_SCL;
    u_short m_RET;
    u_short m_FAN;
    cp32time2a time;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Identification
struct iec_103_type5 {
    u_char  m_COL;
    char m_ch1;
    char m_ch2;
    char m_ch3;
    char m_ch4;
    char m_ch5;
    char m_ch6;
    char m_ch7;
    char m_ch8;
    char m_ch9;
    char m_ch10;
    char m_ch11;
    char m_ch12;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//End of general interrogation
struct iec_103_type8 {
	u_char  m_SCN;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Measurands II
struct iec_103_type9 {
    MEA     M;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Descriptive element cfr. 7.3.1.10
typedef struct tagDescElemAsdu11 {
    u_char  m_KOD;
    GDD     m_GDD;
    u_char*  m_GID;
}DescElemAsdu11;

#define MAX_DATA_SET_IN_TYPE11 50

//Generic identification
struct iec_103_type11 {
	u_char	m_RII;
	GIN		m_GIN;
	NDE		m_NDE;
    DescElemAsdu11 desc_element[MAX_DATA_SET_IN_TYPE11];
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//List of recorded disturbances
struct iec_103_type23 {
	u_short m_FAN;
	SOF		m_SOF;
	cp56time2a time;	
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Ready for transmission of disturbance data
struct iec_103_type26 {
	u_char	not_used;
	u_char	m_TOV;
	u_short	m_FAN;
	u_short	m_NOF;
	u_char	m_NOC;
	u_short	m_NOE;
	u_short	m_INT;
	cp32time2a time;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Ready for transmission of a channel
struct iec_103_type27 {
	u_char	not_used;
	u_char	m_TOV;
	u_short	m_FAN;
	u_char  ACC;
	float RPV;
	float RSV;
	float RFA;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Ready for transmission of tags
struct iec_103_type28 {
	u_char	not_used1;
	u_char	not_used2;
	u_short	m_FAN;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

typedef union tagTagDataAsdu29
{
	struct _tagdata
	{
		u_char m_FUN;
		u_char m_INF;
		DPI    m_DPI;
	}data;
	u_char byte[3]; 
}TagDataAsdu29;

//Transmission of tags
struct iec_103_type29 {
	u_short	m_FAN;
	u_char	m_NOT;
	u_short m_TAP;
    TagDataAsdu29 m_tag[26]; //Limited to 25 by iec 103 norm
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//SDV := F16[1..16]<-1..+1-2^-15>
//Letter F means positive or negative fixed point number
//F16 means 16 bits in size 

//The maximum value of a fixed-point type is 
//simply the largest value that can be represented 
//in the underlying integer type, 
//multiplied by the scaling factor; and similarly for the minimum value. 

//For example, consider a fixed-point type represented as a binary 
//integer with b=16 bits in two's complement format, 
//with a scaling factor of 1/2^f where f = 15 
//(that is, the last f=15 bits are fraction bits): 
//the minimum representable value is -2^(b-1)/2^f = -1 and
//the maximum value is (2^(b-1)-1)/2^f = +1-2^-15 .

//Min value -2^(16-1)/2^15 = -32768/32768 = -1
//Max value (2^(16-1)-1)/2^15 = 32767/32768 = 1 - 2^-15


//Transmission of disturbance values
struct iec_103_type30 {
	u_char	not_used;
	u_char	m_TOV;
	u_short	m_FAN;
	u_char  m_ACC;
	u_char  m_NDV;
	u_short	m_NFE;
	short m_SDV[30]; //Limited to 25 by iec 103 norm. It is a Fixed Point number F16 with sign
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//End of transmission
struct iec_103_type31 {
	u_char	m_TOO;
	u_char	m_TOV;
	u_short	m_FAN;
	u_char  m_ACC;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//In control direction

//Clock synchronization
struct iec_103_type6 {
	cp56time2a time;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//General interrogation
struct iec_103_type7 {
	u_char  m_SCN;	
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

typedef struct tagRII_NGD {
    u_char  m_RII;
    NGD     m_NGD;
}RII_NGD;

typedef struct tagGIN_KOD_GDD {
    GIN     m_GIN; //2 bytes
    u_char  m_KOD; //kind of description 1 byte
    GDD     m_GDD; //3 bytes
}GIN_KOD_GDD;

//Data set cfr. 7.3.1.9
typedef struct tagDataSetAsdu10 {
//    GIN_KOD_GDD gin_kod_gdd;
    GIN     m_GIN; //2 bytes
    u_char  m_KOD; //kind of description 1 byte
    GDD     m_GDD; //3 bytes
	u_char*  m_GID;
}DataSetAsdu10;

#define MAX_DATA_SET_IN_TYPE10 50
//Generic data
struct iec_103_type10 {
	RII_NGD rii_ngd;
    DataSetAsdu10 m_data_set[MAX_DATA_SET_IN_TYPE10];
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//General command
struct iec_103_type20 {
    u_char  m_DCO;
	u_char  m_RII;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

typedef struct tagDataSetAsdu21 {
    GIN m_GIN;
	u_char m_KOD;
}DataSetAsdu21;

//Generic command
struct iec_103_type21 {
    u_char m_RII;
	u_char m_NOG;
    DataSetAsdu21 m_data_set[50];
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Order for disturbance data transmission
struct iec_103_type24 {
	u_char	m_TOO;
	u_char	m_TOV;
	u_short	m_FAN;
	u_char  m_ACC;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Acknowledgement for disturbance data transmission
struct iec_103_type25 {
	u_char	m_TOO;
	u_char	m_TOV;
	u_short	m_FAN;
	u_char  m_ACC;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//Generic data types:

//time tagged message type, generic data type <18>
struct time_tagged_message {
    DPI     m_Dpi;
	u_char  res:6;
    cp32time2a time;
    u_char  m_SIN;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//time tagged message with relative time, generic data type <19>
struct time_tagged_message_with_rel_time {
    DPI     m_Dpi;
	u_char  res:6;
	u_short m_RET;
    u_short m_FAN;
    cp32time2a time;
    u_char  m_SIN;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

//time tagged measurand with relative time, generic data type <20>
struct time_tagged_measurand_with_rel_time {
    float   val;
	u_short m_RET;
    u_short m_FAN;
    cp32time2a time;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

struct iec_103_unit_id {
	u_char		type;	/* type identification */
	u_char		num	:7; /* number of information objects */
	u_char		sq	:1; /* sequenced/not sequenced address i.e. paccata/non paccata*/
	u_char		cause; /* cause of transmission */
	u_char		ca;	/* one byte for common address of ASDU == Link address in iec 103*/
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack()              /* Byte Alignment   */
#endif

#ifdef __cplusplus
}
#endif

#endif	/* __IEC103_TYPES_H */
