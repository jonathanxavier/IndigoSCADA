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
#ifndef __IEC101_ITEM_H
#define __IEC101_ITEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iec101_types.h"

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack(1)             /* Byte Alignment   */
#endif

/* Information object */
struct iec_object {
	u_int		ioa;	/* information object address */
	union {
		struct iec_type1	type1;
		struct iec_type3	type3;
		struct iec_type7	type7;
		struct iec_type9	type9;
		struct iec_type11	type11;
		struct iec_type13	type13;
		struct iec_type15	type15;
		struct iec_type30	type30;
		struct iec_type31	type31;
		struct iec_type33	type33;
		struct iec_type34	type34;
		struct iec_type35	type35;
		struct iec_type36	type36;
		struct iec_type37	type37;
		struct iec_type45	type45;
		struct iec_type46	type46;
		struct iec_type48	type48;
		struct iec_type49	type49;
		struct iec_type50	type50;
		struct iec_type51	type51;
		struct iec_type58	type58;
		struct iec_type59	type59;
		struct iec_type61	type61;
		struct iec_type62	type62;
		struct iec_type63	type63;
		struct iec_type64	type64;
		struct iec_type70	type70;
		struct iec_type100	type100;
		struct iec_type103	type103;
		struct iec_type104	type104;
		struct iec_type105	type105;
		struct iec_type106	type106;
		struct iec_type107	type107;
		struct iec_type120	type120;
		struct iec_type121	type121;
		struct iec_type122	type122;
		struct iec_type123	type123;
		struct iec_type124	type124;
		struct iec_type125	type125;
		struct iec_type126	type126;
		struct iec_type150	type150;
		struct iec_type151	type151;
		struct iec_type152	type152;
		struct iec_type153	type153;
		struct iec_type154	type154;
		struct iec_type155	type155;
		struct iec_type156	type156;
	} o;	
};

//Record format of configuration database inside iec 104 master and slave
//the collection of all records form a table, which is the configuration database

struct iec_item {
    u_char iec_type;
	struct iec_object iec_obj;
	unsigned char cause; //spontaneous or general interrogation cause
	u_int   msg_id; //ID of the message
	unsigned int ioa_control_center; //unstructured
	u_short casdu; //IEC 104 CASDU where this record is allocated, it is unstructured
    u_char	is_neg; //positive == 0 or negative == 1
	u_char	checksum; //Checksum of the message, must be the last if filled with CRC 8
};


#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack()              /* Byte Alignment   */
#endif


#ifdef __cplusplus
}
#endif

#endif	/* __IEC101_DB_OPC_H */
