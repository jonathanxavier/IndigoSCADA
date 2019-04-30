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
#ifndef __IEC103_ITEM_H
#define __IEC103_ITEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iec103_types.h"

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack(1)             /* Byte Alignment   */
#endif

/* Information object */
struct iec_103_object {

    u_char fun_type;
    u_char inf_num;

	union {
		struct iec_103_type1	type1;
        struct iec_103_type2	type2;
		struct iec_103_type3	type3;
        struct iec_103_type4	type4;
        struct iec_103_type5	type5;
        struct iec_103_type6	type6;
		struct iec_103_type7	type7;
        struct iec_103_type8	type8;
		struct iec_103_type9	type9;
        struct iec_103_type10	type10;
		struct iec_103_type11	type11;
		struct iec_103_type20	type20;
		struct iec_103_type21	type21;
        struct iec_103_type23	type23;
        struct iec_103_type24	type24;
        struct iec_103_type25	type25;
        struct iec_103_type26	type26;
        struct iec_103_type27	type27;
        struct iec_103_type28	type28;
        struct iec_103_type29	type29;
        struct iec_103_type30	type30;
        struct iec_103_type31	type31;
	} o;	
};

struct iec_103_item {

    u_char iec_type; 
	struct iec_103_object iec_obj;
	unsigned char cause; //COT
	u_int   msg_id; //ID of the message
	unsigned char func; 
    unsigned char info; 
	u_short casdu; //IEC 103 CASDU where this record is allocated
    u_char	is_neg; //positive == 0 or negative == 1
	u_char	checksum; //Checksum of the message
};

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack()              /* Byte Alignment   */
#endif


#ifdef __cplusplus
}
#endif

#endif	/* __IEC101_DB_OPC_H */
