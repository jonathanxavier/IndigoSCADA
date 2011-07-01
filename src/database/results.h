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

#ifndef include_results_hpp 
#define include_results_hpp 
#include "common.h"
#include "smplstat.h"
#include "realtimedb.h"
#include "historicdb.h"
#include "helper_functions.h"
//#include <sptypes.h>
#include "mythread.h"

extern "C"
{ 
	typedef struct scada_point SC_PT;
	//begin variabili e funzioni condivise con l'interprete EiC

	struct scada_point{
		char name[35];
		char tag[20];
		//double previous_value;
		double current_value;
		double next_value;
		unsigned char write_to_driver;
		unsigned int checksum;
	};

	#define MAX_SCADA_POINTS 5000

QSEXPORT extern struct scada_point scada_db[MAX_SCADA_POINTS];
	
	/** mutex for thread to stop the threads hitting data at the same time. */
QSEXPORT extern ins_mutex_t * mut;
};

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack(1)             /* Byte Alignment   */
#endif

/* 32-bit string state and change data unit */
struct iec_stcd {
	union {
		u_short		st[1];
		u_char		st1	:1;
		u_char		st2	:1;
		u_char		st3	:1;
		u_char		st4	:1;
		u_char		st5	:1;
		u_char		st6	:1;
		u_char		st7	:1;
		u_char		st8	:1;
		u_char		st9	:1;
		u_char		st10:1;
		u_char		st11:1;
		u_char		st12:1;
		u_char		st13:1;
		u_char		st14:1;
		u_char		st15:1;
		u_char		st16:1;
	};

	union {
		u_short		cd[1];
		u_char		cd1	:1;
		u_char		cd2	:1;
		u_char		cd3	:1;
		u_char		cd4	:1;
		u_char		cd5	:1;
		u_char		cd6	:1;
		u_char		cd7	:1;
		u_char		cd8	:1;
		u_char		cd9	:1;
		u_char		cd10:1;
		u_char		cd11:1;
		u_char		cd12:1;
		u_char		cd13:1;
		u_char		cd14:1;
		u_char		cd15:1;
		u_char		cd16:1;
	};
};

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
} cp56time2a;


/* CP16time2a timestamp*/
typedef struct cp16time2a {
	u_short		msec; /* milliseconds upto seconds */
}cp16time2a;


//////////////////////IEC types////////////////////////////////////////////////

/* M_SP_NA_1 - single point information with quality description */
typedef struct _iec_type1 {
	u_char		sp	:1; /*single point information 0 off, 1 on*/
	u_char		res	:3;
	u_char		bl	:1; /*1 blocked/ 0 not blocked*/
	u_char		sb	:1; /*1 substituted/ 0 not substituted*/
	u_char		nt	:1; /*1 not_topical/0 topical*/
	u_char		iv	:1; /*1 invalid/0 valid*/
}iec_type1;

/* M_DP_NA_1 - double point information with quality description */
typedef struct _iec_type3 {
	u_char		dp	:2;		/* 0 indeterminate or intermediate state
						    1 determinate state off
						    2 determinate state on
						    3 indeterminate state */
	u_char		res	:2;
	u_char		bl	:1; /* 1 blocked/0 not_blocked*/
	u_char		sb	:1; /* 1 substituted/0 not_substituted */
	u_char		nt	:1; /* 1 not_topical/0 topical */
	u_char		iv	:1; /* 1 invalid/0 valid */
}iec_type3;

/* M_BO_NA_1 - state and change information bit string */
typedef struct _iec_type7 {
	struct iec_stcd	stcd;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
}iec_type7;

/* M_ME_NA_1 - normalized measured value */
typedef struct _iec_type9 {
	u_short		mv;	/* normalized value: from 32767 to -32768 */
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
}iec_type9;

/* M_ME_NB_1 - scaled measured value */
typedef struct _iec_type11 {
	u_short		mv;	/* scaled value: from 32767 to -32768 */
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
}iec_type11;

/* M_ME_NC_1 - short floating point measured value */
typedef struct _iec_type13 {
	float		mv;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
}iec_type13;

/* M_IT_NA_1 - itegrated totals */
typedef struct _iec_type15 {
	int		count_read;	/* counter reading */ /*BCR = Binary counter reading, defined 7.2.6.9*/
	u_char		sq	:5; /* sequence number */
	u_char		cy	:1; /* carry: 0 = no counter overflow occurred / 1 = counter overflow occurred*/
	u_char		ca	:1; /* 0 = counter was not adjusted / 1 = counter was adjusted */
	u_char		iv	:1; /* 0 = valid/ 1 = invalid */
}iec_type15;


/* M_SP_TB_1 - single point information with quality description and time tag */
typedef struct _iec_type30 {
	u_char		sp	:1; /* single point information */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}iec_type30;

/* M_DP_TB_1 - double point information with quality description and time tag */
typedef struct _iec_type31 {
	u_char		dp	:2; /* double point information */
	u_char		res	:2;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}iec_type31;

/* M_BO_TB_1 - state and change information bit string and time tag  */
typedef struct _iec_type33 {
	struct iec_stcd	stcd;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}iec_type33;

/* M_ME_TD_1 - normalized measured value with time tag */
typedef struct _iec_type34 {
	u_short		mv;	/* normalized value */
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}iec_type34;

/* M_ME_TE_1 - scaled measured value with time tag*/
typedef struct _iec_type35 {
	u_short		mv;	/* scaled value */
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}iec_type35;

/* M_ME_TF_1 - short floating point measurement value and time tag */
typedef struct _iec_type36 {
	float		mv;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}iec_type36;

/* M_IT_TB_1 - Integrated totals with time tag */
typedef struct _iec_type37 {
	int		    counter; /*binary counter reading*/
	u_char		seqn:5; /*sequence number*/
	u_char		cy	:1; /*carry*/
	u_char		ca	:1; /*counter was adjusted*/
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}iec_type37;

/* C_SC_NA_1 - Single command */
typedef struct _iec_type45 {
	u_char		scs:1;//See 7.2.6.15 Qualifier of command
	u_char		res:1;
	u_char		qu:5; //See 7.2.6.26 Qualifier of command
	u_char		se:1;//See 7.2.6.26 Qualifier of command
}iec_type45;

/* C_DC_NA_1 - Double command */
typedef struct _iec_type46 {
	u_char		dcs:2;//See 7.2.6.16 Qualifier of command
	u_char		qu:5;//See 7.2.6.26 Qualifier of command
	u_char		se:1;//See 7.2.6.26 Qualifier of command
}iec_type46;

/* C_BO_NA_1 - Bitstring of 32 bit*/
typedef struct _iec_type51 {
	struct iec_stcd	stcd;
}iec_type51;

/* C_SC_TA_1 - Single command with time tag*/
typedef struct _iec_type58 {
	u_char		scs:1;//See 7.2.6.15 Qualifier of command
	u_char		res:1;
	u_char		qu:5; //See 7.2.6.26 Qualifier of command
	u_char		se:1; //See 7.2.6.26 Qualifier of command
	cp56time2a	time; //contains the UTC time
}iec_type58;

/* C_BO_TA_1 - Bitstring of 32 bit command with time tag*/
typedef struct _iec_type64 {
	struct iec_stcd	stcd;
	u_char chs;
	cp56time2a	time; //contains the UTC time
}iec_type64;

/* M_EI_NA_1 - End of initialization*/
typedef struct _iec_type70 {
	u_char		coi_ui7:7; /*cause of initialisation*/
	u_char		coi_bs1:1; /*cause of initialisation*/
}iec_type70;

/* C_IC_NA_1 - Interrogation command*/
typedef struct _iec_type100 {
	u_char		qoi; /*qualifier of interrogation*/
}iec_type100;

/* C_CS_NA_1 - Clock synchronisation command*/
typedef struct _iec_type103 {
	cp56time2a	time;
}iec_type103;


/* C_TS_NA_1 - Test command*/
typedef struct _iec_type104 {
	u_short fbp; /*fixed test pattern*/
}iec_type104;

/* C_TS_TA_1 - Test command with time tag*/
typedef struct _iec_type107 {
	u_short fbp; /*fixed test pattern*/
	cp56time2a	time;
}iec_type107;

/* C_RP_NA_1 - Reset process command*/
typedef struct _iec_type105 {
	u_char		qrp; /*qualifier of reset process command*/
}iec_type105;

/* C_CD_NA_1 - Delay acquisition command*/
typedef struct _iec_type106 {
	cp16time2a	two_oct_time; /*two octect binary time cfr 7.2.6.20*/
}iec_type106;

/* F_FR_NA_1 - File ready*/
typedef struct _iec_type120 {
	u_short		nof; /*name of file cfr.7.2.6.33*/
	u_int		lof:24; /*length of file cfr.7.2.6.35*/
	u_int		frq_ui7:7; /*file ready qualifier cfr.7.2.6.28*/
	u_int		frq_bs1:1; /*file ready qualifier cfr.7.2.6.28*/
}iec_type120;

/* F_SR_NA_1 - Section ready*/
typedef struct _iec_type121 {
	u_short		nof; /*name of file cfr.7.2.6.33*/
	u_char		nos; /*name of section cfr.7.2.6.34*/
	u_int		lof:24; /*length of file cfr.7.2.6.35*/
	u_int		srq_ui7:7; /*section ready qualifier cfr.7.2.6.29*/
	u_int		srq_bs1:1; /*section ready qualifier cfr.7.2.6.29*/
}iec_type121;

/* F_SC_NA_1 - Call directory, select file, call file, call section*/
typedef struct _iec_type122 {
	u_short		nof; /*name of file cfr.7.2.6.33*/
	u_char		nos; /*name of section cfr.7.2.6.34*/
	u_char		scq_ui4low:4; /*select and call qualifier cfr.7.2.6.30*/
	u_char		scq_ui4high:4; /*select and call qualifier cfr.7.2.6.30*/
}iec_type122;

/* F_LS_NA_1 - Last section, last segment*/
typedef struct _iec_type123 {
	u_short		nof; /*name of file cfr.7.2.6.33*/
	u_char		nos; /*name of section cfr.7.2.6.34*/
	u_char		lsq; /*last section or segment qualifier cfr.7.2.6.31*/
	u_char		chs; /*checksum cfr.7.2.6.37*/
}iec_type123;

/* F_AF_NA_1 - ACK file, ACK section*/
typedef struct _iec_type124 {
	u_short		nof; /*name of file cfr.7.2.6.33*/
	u_char		nos; /*name of section cfr.7.2.6.34*/
	u_char		afq_ui4low:4; /*ACK file or section qualifier cfr.7.2.6.32*/
	u_char		afq_ui4high:4; /*ACK file or section qualifier cfr.7.2.6.32*/
}iec_type124;


/* F_SG_NA_1 - Segment*/
typedef struct _iec_type125 {
	u_short		nof; /*name of file cfr.7.2.6.33*/
	u_char		nos; /*name of section cfr.7.2.6.34*/
	u_char		los; /*Length of segment cfr.7.2.6.36*/
}iec_type125;

/* F_DR_TA_1 - Directory. E' paccata ! It is packed !*/
typedef struct _iec_type126 {
	u_short		nof; /*name of file or subdirectory cfr.7.2.6.33*/
	u_int		lof:24; /*length of file cfr.7.2.6.35*/
	u_int		sof_status:5; /*Status of file cfr.7.2.6.38*/
	u_int		sof_lfd:1; /*last file of the directory cfr.7.2.6.38*/
	u_int		sof_for:1; /*Name defines file or directory cfr.7.2.6.38*/
	u_int		sof_fa:1; /*file active or waits cfr.7.2.6.38*/
	cp56time2a	time; /*Creation time of the file*/
}iec_type126;

/////////////////////////IndigoSCADA types/////////////////////////////////////////

/* M_ME_TN_1 - long floating point measurement value and time tag */
typedef struct _is_type150 {
	double		mv;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}is_type150;

/* M_ME_TO_1 - 32 bit unsigned int measurement value and time tag */
typedef struct _is_type151 {
	u_int mv;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}is_type151;

/* M_ME_TP_1 - 32 bit signed int measurement value and time tag */
typedef struct _is_type152 {
	int mv;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}is_type152;

/* M_ME_TQ_1 - 16 bit signed short measurement value and time tag */
typedef struct _is_type153 {
	short mv;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}is_type153;

/* M_ME_TR_1 - 32 bit unsigned int measurement value and time tag */
typedef struct _is_type154 {
	u_int mv;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}is_type154;

/* M_ME_TS_1 - 64 bit unsigned int measurement value and time tag */
typedef struct _is_type155 {
	unsigned __int64 mv;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}is_type155;

/* M_ME_TT_1 - 64 bit signed int measurement value and time tag */
typedef struct _is_type156 {
	__int64 mv;
	u_char		ov	:1; /* overflow/no overflow */
	u_char		res	:3;
	u_char		bl	:1; /* blocked/not blocked */
	u_char		sb	:1; /* substituted/not substituted */
	u_char		nt	:1; /* not topical/topical */
	u_char		iv	:1; /* valid/invalid */
	cp56time2a	time;
}is_type156;

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack()              /* Byte Alignment   */
#endif

////////////////////////////////////////////////////////////////////////////////////
//measurement_type
enum m_type
{
	M_SP_NA_1 =		1,
	M_DP_NA_1 =		3,
	M_BO_NA_1 =		7,
	M_ME_NA_1 =		9,
	M_ME_NB_1 =		11,
	M_ME_NC_1 =		13,
	M_SP_TB_1 =		30,
	M_DP_TB_1 =		31,
	M_BO_TB_1 =		33,
	M_ME_TD_1 =		34,
	M_ME_TE_1 =		35,
	M_ME_TF_1 =		36,
	M_ME_TN_1 =		150, //custom type
	M_ME_TO_1 =		151,
	M_ME_TP_1 =		152,
	M_ME_TQ_1 =		153,
	M_ME_TR_1 =		154,
	M_ME_TS_1 =		155,
	M_ME_TT_1 =		156,
	s8bit	  =		200, //custom type
	u8bit	  =		201, //custom type
	s16bit	  =		202, //custom type
	u16bit	  =		203, //custom type
	s32bit	  =		204, //custom type
	u32bit	  =		205, //custom type
	s64bit	  =		206, //custom type
	u64bit	  =		207, //custom type
	f32bit	  =		208, //custom type
	f64bit	  =		209 //custom type
};

struct SpValue // a measured result
{
	QString tag; // the tag name eg value, 0.5
	unsigned char type; //the type of measurement
	//from single bit with quality to value at 64 bits
	union {
		iec_type1 v1_q;
		iec_type3 v3_q;
		iec_type7 v7_q;
		iec_type9 v9_q;
		iec_type11 v11_q;
		iec_type13 v13_q;
		iec_type30 v30_q;
		iec_type33 v33_q;
		iec_type34 v34_q;
		iec_type35 v35_q;
		iec_type36 v36_q;
		is_type150 v150_q;
		is_type151 v151_q;
		is_type152 v152_q;
		is_type153 v153_q;
		is_type154 v154_q;
		is_type155 v155_q;
		is_type156 v156_q;
		/*
		unsigned char uval8;						//0 to           255 
		char val8;									//-128 to           127
		short int val16; //measurand with sign         -32,768 to        32,767 
		unsigned short int uval16; //measurand unsigned 0 to        65,535
		unsigned int uval32;// measurand unsigned 0 to 4,294,967,295
		int    val32; //32 bit signed -2,147,483,648 to 2,147,483,647
		float  fval32; //32 bit floating point
		__int64 val64;
		unsigned __int64 uval64;
		double fval64; //64 bit floating point 
		double value; // the value 
		*/
	};

	SpValue(const QString &s= "", void* v = NULL, unsigned char tp = 0) : tag(s), type(tp)
	{
		/*
		memset(&v1_q, 0x00, sizeof(v1_q));
		memset(&v3_q, 0x00, sizeof(v3_q));
		memset(&v7_q, 0x00, sizeof(v7_q));
		memset(&v9_q, 0x00, sizeof(v9_q));
		memset(&v11_q, 0x00, sizeof(v11_q));
		memset(&v13_q, 0x00, sizeof(v13_q));
		memset(&v30_q, 0x00, sizeof(v30_q));
		memset(&v33_q, 0x00, sizeof(v33_q));
		memset(&v34_q, 0x00, sizeof(v34_q));
		memset(&v35_q, 0x00, sizeof(v35_q));
		memset(&v36_q, 0x00, sizeof(v36_q));
		memset(&v150_q, 0x00, sizeof(v150_q));
		memset(&v151_q, 0x00, sizeof(v151_q));
		memset(&v152_q, 0x00, sizeof(v152_q));
		memset(&v153_q, 0x00, sizeof(v153_q));
		memset(&v154_q, 0x00, sizeof(v154_q));
		memset(&v155_q, 0x00, sizeof(v155_q));
		*/
		memset(&v150_q, 0x00, sizeof(v150_q));

		if(v == NULL)
		{
			v150_q.mv = 0.0;
			v150_q.iv = 0;
			v = (void *)&v150_q;
		}

		switch(tp)
		{
			case M_SP_NA_1:
			v1_q = *(iec_type1*)v;
			break;
			case M_DP_NA_1:
			v3_q = *(iec_type3*)v;
			break;
			case M_BO_NA_1:
			v7_q = *(iec_type7*)v;
			break;
			case M_ME_NA_1:
			v9_q = *(iec_type9*)v;
			break;
			case M_ME_NB_1:
			v11_q = *(iec_type11*)v;
			break;
			case M_ME_NC_1:
			v13_q = *(iec_type13*)v;
			break;
			case M_SP_TB_1:
			v30_q = *(iec_type30*)v;
			break;
			case M_BO_TB_1:
			v33_q = *(iec_type33*)v;
			break;
			case M_ME_TD_1:
			v34_q =  *(iec_type34*)v;
			break;
			case M_ME_TE_1:
			v35_q =  *(iec_type35*)v;
			break;
			case M_ME_TF_1:
			v36_q =  *(iec_type36*)v;
			break;
			case M_ME_TN_1:
			v150_q =  *(is_type150*)v;
			break;
			default:
			v150_q.mv = *((double*)v);
			type = 150;
			break;
		}
	};

	SpValue(const SpValue &s) : tag(s.tag), type(s.type) 
	{
		switch(s.type)
		{
			case M_SP_NA_1:
			v1_q = s.v1_q;
			break;
			case M_DP_NA_1:
			v3_q = s.v3_q;
			break;
			case M_BO_NA_1:
			v7_q = s.v7_q;
			break;
			case M_ME_NA_1:
			v9_q = s.v9_q;
			break;
			case M_ME_NB_1:
			v11_q = s.v11_q;
			break;
			case M_ME_NC_1:
			v13_q = s.v13_q;
			break;
			case M_SP_TB_1:
			v30_q = s.v30_q;
			break;
			case M_BO_TB_1:
			v33_q = s.v33_q;
			break;
			case M_ME_TD_1:
			v34_q = s.v34_q;
			break;
			case M_ME_TE_1:
			v35_q = s.v35_q;
			break;
			case M_ME_TF_1:
			v36_q = s.v36_q;
			break;
			case M_ME_TN_1:
			v150_q = s.v150_q;
			break;
			default:
			v150_q = s.v150_q;
			break;
		}
	}; 
};


extern "C"
{
	QSEXPORT double LookupCurrentValue(char *name, char *tag);
	QSEXPORT double get_value_sp_value(SpValue &v);
	QSEXPORT struct cp56time2a 	get_time_of_sp_value(SpValue &v);
	QSEXPORT void set_value_sp_value(SpValue &v, double val);
	QSEXPORT __int64 Epoch_in_millisec_from_cp56time2a(const struct cp56time2a* time);
};

QSEXPORT QString GetIsoDateString_from_epoch_in_millisec(__int64 epoch_in_millisec);

typedef std::vector<SpValue> SpValueList; 

struct AlarmLimit // a tag's alarm limits
{
	bool Enabled;

	double Limit;

	AlarmLimit() : Enabled(0),Limit(0) {};

	bool Check(SpValue v, bool dir = true) // true if upper, false if lower
	{
		if(Enabled)
		{
			if(dir)
			{
				if(get_value_sp_value(v) > Limit) return true; 
			}
			else
			{
				if(get_value_sp_value(v) < Limit) return true;
			};
		};
		return false;
	};
};

struct TagItem // current tag value
{
	AlarmLimit UpperAlarm;
	AlarmLimit UpperWarning;
	AlarmLimit LowerWarning;
	AlarmLimit LowerAlarm;
	bool changed;  // set on a new value
	//double value; // current value
	SpValue value; //prova del 07-12-09
	int state; // current state
	QDateTime updated; // when it was updated
	SampleStatistic stats; // the stats
	bool enabled;	//APA added on 29-12-01

	TagItem() : changed(0), value(0),state(0),enabled(false) 
	{
	};
};

class QSEXPORT SamplePoint            // the sample point configuration - we need to cache these
{
	public:	
	QString  Name;            // Sample Point name
	bool     fChanged;        // has the sample point changed
	bool	 fOvState;		  // APA added 06-03-04, has the state been setted
	bool     fAckTriggered;   // has an ack been triggered
	int      AlarmState;      // the alarm state
	bool     Fileable;        // do we file this point's data
	int      OldState;        // the previous larm state
	//
	unsigned AlarmThreshold;  // do we need more than one alarm event to trigger an alarm
	unsigned AlarmEvents;     // alarm events
	QString  Unit;            // the unit associated with this sample point
	QString  Type;            // the type of the sample point
	QString InputIndex;       // the input index
	bool Retriggerable;       // are the alarms retriggerable - if in alarm do we trigger ack and alarm events on each alarm
	// or do we wait for it to exit alarm
	QString Comment;          // the last comment string (for current value)
	QDateTime updated;        // last update time
	//
	// statistics
	int nalarms;         // number of alarms
	int nwarnings;       // number of warnings
	int nmeasures;       // number of measures
	QDateTime alarmtime; // last alarm
	QDateTime failtime;  // last failure
	//
	typedef std::map<QString, TagItem, std::less<QString> > TagDict;
	TagDict Tags; // the tag values
	//
	//
	SamplePoint(const QString &name) : Name(name),
	fChanged(0),fOvState(0),fAckTriggered(0),AlarmState(0),
	Fileable(0),OldState(0),AlarmThreshold(0),AlarmEvents(0),
	Retriggerable(0),nalarms(0),nwarnings(0),nmeasures(0),db_idx(0)
	{
		alarmtime.setTime_t(0);
		failtime.setTime_t(0);
		mut = ins_mutex_new();
	};

	~SamplePoint()
	{
	}

	//Il vettore scada_db e' ordinato come il risultato della query:
	//GetCurrentDb()->DoExec(this,"select * from TAGS_DB;",tTagsCurrent);
	int db_idx; //indice nel vettore scada_db

	//
	// copy constructor
	// 
	SamplePoint(const SamplePoint &s) : 
	Name(s.Name),
	fChanged(s.fChanged),
	fOvState(s.fOvState),
	fAckTriggered(s.fAckTriggered),
	AlarmState(s.AlarmState),
	Fileable(s.Fileable),
	OldState(s.OldState),
	AlarmThreshold(s.AlarmThreshold),
	AlarmEvents(s.AlarmEvents),
	Retriggerable(s.Retriggerable),
	nalarms(s.nalarms),
	nwarnings(s.nwarnings),
	nmeasures(s.nmeasures)
	{
		alarmtime.setTime_t(0);
		failtime.setTime_t(0);
	};
	//
	//
	//
	void Update(int state, bool ack = false, const QString &c=""); // mark a sample point as updated and ack triggered
	
	int UpdateTag(const QString &name, SpValue value, QString Type); 
};


typedef std::map<QString,SamplePoint, std::less<QString> > SamplePointDictWrap; // dictionary of them


class QSEXPORT Results : public QObject
{
	//
	// Alarm Groups cause a lot of database traffic - so we have to cache them 
	// and then write them out from time to time
	//
	Q_OBJECT
	//
	enum { tCurrentSample = 1 , tCurrentTags, tTags, tTable, tAlarmGroup, tActions,tAlarmActions}; // various transactions
	static QMutex ResultLock;
	//
	//
	void UpdateStart(const QString &name); //  start an update 
	void Update(const QString &name, const QString &tag, SpValue &value); // update a sample point by tag
	void UpdateEnd(const QString &name); // end an update on a sample point
	//
	public:
	//
	class StateDict : public std::map<QString, bool, std::less<QString> >  // state dictionary - alarm group item
	{
		public:
		int AckState; // the acknowledge state of the group
		int State;    // the state of the group
		bool Changed; // has it changed
		StateDict() : AckState(0),State(0),Changed(0){};
	};
	//
	typedef std::map<QString, StateDict, std::less<QString> > GroupDict; // dictionary of dictionaries
//	private:
public:

	static GroupDict Groups; // the alarm groups
	static SamplePointDictWrap EnabledPoints;// this is the list of enabled sample points and current values
	
	public:
	static GroupDict &GetGroups(); // the alarm groups - NB Also Locks 
	static SamplePointDictWrap &GetEnabledPoints();// this is the list of enabled sample points and current values - NB Also Locks
	
	static double LookupValue(const char *name, const char *tag);
	//
	// responsibility to action the update
	//
	Results(QObject *parent); 
	//     
	~Results() {};
	// queue a new result packet for filing
	void QueueResult(const QString &name, SpValueList &pL); //queue a result
	static void Lock()
	{
		ResultLock.lock(); // lock it		
	}; // wait for mutex to lock
	static void Unlock() // unlock the mutex (assumes we locked it)
	{
		ResultLock.unlock();
	};
	//
	void UpdateSamplePointAlarmGroup(const QString &spname, int state,bool fAck); // update a single sample point in an alarm group
	//
	public slots:
	//
	void ConfigQueryResponse (QObject *,const QString &, int, QObject*);  // handles configuration responses
	//    
};

#endif

