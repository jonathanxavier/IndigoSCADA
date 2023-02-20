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

#ifndef OPC_UA_DB_H
#define OPC_UA_DB_H

struct opcuaDbRecord
{
	char nodeid[50];		//OPC UA node ID string
	int namespace_index;	// namespace index
	int dataType;			//data type of the node
	//////////////control center part/////////////////////////////////////////
	unsigned int ioa_control_center; //unique inside CASDU
};

#endif //OPC_UA_DB_H