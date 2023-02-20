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

#ifndef OPC_UA_IMP_H
#define OPC_UA_IMP_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#include <stdio.h>
#include <time.h>
#include "itrace.h"
#include "stdint.h"
#include "client_api.h"

#include "iec_item_type.h" //Middleware
////////////////////////////Middleware///////////////////////////////////////////////////////
extern void onRegFail(void *param);
extern void recvCallBack(const ORTERecvInfo *info,void *vinstance, void *recvCallBackParam); 
/////////////////////////////////////////////////////////////////////////////////////////////

class opcua_imp
{
	public:
	///////////////////configuration database//////////////////////
	char database_name[MAX_PATH];
	int db_n_rows;
	int db_m_columns;
	/////////////////////////////////////////////////
	int g_dwNumItems;
	struct opcuaDbRecord* Config_db;
	/////////////////////Middleware/////////////////////////
	ORTEDomain              *domain;
	ORTEPublication			*publisher;
	ORTESubscription        *subscriber;
	iec_item_type			instanceSend;
	iec_item_type		    instanceRecv;
	//////////////////////////////end//Middleware///////////

	/////////////Middleware///////////////////////////////
	u_int n_msg_sent_monitor_dir;
	u_int n_msg_sent_control_dir;
	//////////////////////////////////////////////////////
	bool fExit;
	unsigned long pollingTime;
	bool general_interrogation;
	int loops;
    int comm_error_counter;
	bool is_connected;
	char serverURL[80];
	Session session;
	int nb_points;
	////////////////////////////////////////////////
	int lineNumber;
	
	opcua_imp(char* server_url, char* line_number, int polling_time);
	~opcua_imp();
	int AddItems(void);
	int PollServer(void);
	int Start(void);
	void LogMessage(int* error = 0, const char* name = NULL);
	int Stop(void);
	int64_t epoch_from_cp56time2a(const struct cp56time2a* time);
	int PollItems(void);
	short rescale_value(double V, double Vmin, double Vmax, int* error);
	double rescale_value_inv(double A, double Vmin, double Vmax, int* error);
	
	////////////////////Middleware//////////////////////
	void check_for_commands(struct iec_item *item);
	void get_utc_host_time(struct cp56time2a* time);
	void get_local_host_time(struct cp56time2a* time);
	////////////////////////////////////////////////
	void get_opc_ua_message_time(struct cp56time2a* time, OpcUa_DateTime* timestamp);
};

#endif //OPC_UA_IMP_H