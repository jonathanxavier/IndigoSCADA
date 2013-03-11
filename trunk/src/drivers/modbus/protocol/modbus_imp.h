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

#ifndef MODBUS_IMP_H
#define MODBUS_IMP_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#include <stdio.h>
#include <time.h>
#include "itrace.h"
#include "iec_item_type.h" //Middleware

////////////////////////////Middleware///////////////////////////////////////////////////////
extern void onRegFail(void *param);
extern void recvCallBack(const ORTERecvInfo *info,void *vinstance, void *recvCallBackParam); 
/////////////////////////////////////////////////////////////////////////////////////////////

struct modbusContext
{
	//////////////////MODBUS TCP context///////////////////////
	char modbus_server_address[40];
	char modbus_server_port[40];
	//////////////////MODBUS RTU context///////////////////////
	//The device argument specifies the name of the serial port handled by the OS,
	//eg. '/dev/ttyS0'. On Windows, it's necessary to prepend COM
	//name with '\\.\' for COM number greater than 9, eg. '\\\\.\\COM10'. See
	//http://msdn.microsoft.com/en-us/library/aa365247(v=vs.85).aspx for details

	char device[40];
	/* Bauds: 9600, 19200, 57600, 115200, etc */
    int baud;
    /* Data bit */
    uint8_t data_bit;
    /* Stop bit */
    uint8_t stop_bit;
    /* Parity: 'N', 'O', 'E' */
    char parity;
};

class modbus_imp
{
	public:
	char plc_server_prog_id[100];
	int g_dwNumItems;
	struct modbusItem* Config_db;
	/////////////////////Middleware/////////////////////////
	int received_command_callback;
	ORTEDomain              *domain;
	ORTEDomainProp          dp; 
	static ORTEPublication  *publisher;
	ORTESubscription        *subscriber;
	static iec_item_type    instanceSend;
	iec_item_type		    instanceRecv;
	ORTEDomainAppEvents     events;
	///////////////////////////////////Middleware///////////
	bool fExit;
	int  g_dwUpdateRate;
	double dead_band_percent;
	char		ServerIPAddress[80];
	char		ServerPort[80];
	int local_server;
	double		pollingTime;
	
	modbus_imp(struct modbusContext* ctx, char* line_number, int polling_time);
	~modbus_imp();
	int AddItems(void);
	void CreateSqlConfigurationFile(char* sql_file_name, char* opc_path);
	int Async2Update();
	int check_connection_to_server(void);
	int RfcStart(char* RfcServerProgID, char* RfcclassId, char* RfcUpdateRate, char* RfcPercentDeadband);
	void LogMessage(int* error = 0, const char* name = NULL);
	int RfcStop();
	int GetStatus(WORD *pwMav, WORD *pwMiv, WORD *pwB, LPWSTR *pszV);
	time_t epoch_from_cp56time2a(const struct cp56time2a* time);
	void epoch_to_cp56time2a(cp56time2a *time, signed __int64 epoch_in_millisec);
	void SendEvent2(void);
	signed __int64 epoch_from_FILETIME(const FILETIME *fileTime);
	short rescale_value(double V, double Vmin, double Vmax, int* error);
	double rescale_value_inv(double A, double Vmin, double Vmax, int* error);
	
	////////////////////Middleware//////////////////////
	void check_for_commands(struct iec_item *item);
	void alloc_command_resources(void);
	void free_command_resources(void);
	void get_utc_host_time(struct cp56time2a* time);
	void get_items(struct iec_item* p_item);
	////////////////////////////////////////////////
};

#endif //MODBUS_IMP_H