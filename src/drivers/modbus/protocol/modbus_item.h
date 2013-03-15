/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2013 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef MODBUS_ITEM_H
#define MODBUS_ITEM_H

struct modbusItem
{
	//////////////////MODBUS RTU part/////////////////////////////////////////
	char name[100];			//Item ID is an unique name identifing it
	int modbus_function_read;	//modbus funtion to read
	int modbus_function_write;	//modbus funtion to write
	int modbus_start_address; //start address of the memory to fetch
	int block_size; //size in bit or word of the the memory to fetch
	int offset; //offset in bits or word into the memory to fetch (to sum with start address in order to find the address of interested data)
	//////////////control center part///////////////////////////////////
	unsigned int ioa_control_center; //unique inside CASDU
	unsigned int iec_type_read;   //IEC 104 type to read
	unsigned int iec_type_write;   //IEC 104 type to write
	int size_in_bits_of_iec_type; //The sise in bits of the IEC 104 type
	union {
		int a;
		float f;
    } last_value;
};

#endif //MODBUS_ITEM_H