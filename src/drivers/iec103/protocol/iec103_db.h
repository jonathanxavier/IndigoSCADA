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

#ifndef IEC103_DB_H
#define IEC103_DB_H

struct iec103DbRecord
{
	//////////////////iec103 RTU part/////////////////////////////////////////
	unsigned char iec103_func;	//iec103 funtion
	unsigned char iec103_info;	//iec103 info
	unsigned char iec103_type_command; //iec103 type used in (executing) writing a command
	//////////////control center part/////////////////////////////////////////
	unsigned int ioa_control_center; //unique inside CASDU
	float deadband; //Deadband for analog values
	union {
		int a;
		float f;
    } last_value;
};

#endif //IEC103_ITEM_H