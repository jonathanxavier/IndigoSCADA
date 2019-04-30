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

#ifndef IEC104_IMP_H
#define IEC104_IMP_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
#include <stdio.h>
#include <time.h>
#include "itrace.h"

#ifdef USE_RIPC_MIDDLEWARE
////////Middleware/////////////
#include "RIPCThread.h"
#include "RIPCFactory.h"
#include "RIPCSession.h"
#include "RIPCServerFactory.h"
#include "RIPCClientFactory.h"
#include "ripc.h"
///////////////////////////////
#endif

////////////////////////////Middleware///////////////////////////////////////////////////////
#include "iec_item_type.h"
extern void onRegFail(void *param);
extern void recvCallBack(const ORTERecvInfo *info,void *vinstance, void *recvCallBackParam); 

#include "fifoc.h"

struct iec104Context
{
	//////////////////iec104 RTU context///////////////////////
	char iec104ServerAddress[80];
	char iec104ServerPort[80];
};

typedef class iec104_imp* par;

struct subs_args{
	par parent;
};

class iec104_imp
{
	public:
	///////////////////configuration database//////////////////////
	char database_name[MAX_PATH];
	int db_n_rows;
	int db_m_columns;
	int g_dwNumItems;
	/////////////Middleware///////////////////////////////
	unsigned int n_msg_sent_monitor_dir;
	unsigned int n_msg_sent_control_dir;
	int exit_threads;
	fifo_h fifo_monitor_direction;
	fifo_h fifo_control_direction;

	#ifdef USE_RIPC_MIDDLEWARE
    int          port;
    char const*  hostname;
    RIPCFactory* factory1;
	RIPCFactory* factory2;
	RIPCSession* session1;
	RIPCSession* session2;
	RIPCQueue*   queue_monitor_dir;
	RIPCQueue*   queue_control_dir;
	#endif
	struct subs_args arg;

	/////////////////////Middleware/////////////////////////
	ORTEDomain             *domain;
	ORTEPublication		*publisher;
	ORTESubscription       *subscriber;
	iec_item_type			instanceSend;
	iec_item_type		    instanceRecv;
	//////////////////////////////end//Middleware///////////

	//////////////////////////////////////////////////////
	bool fExit;
	struct iec104Context my_iec104_context;
	bool is_connected;
	////////////////IEC104 specific/////////////////
	struct iec104Context *ctx; //context
	////////////////////////////////////////////////
	int lineNumber;
	
	iec104_imp(struct iec104Context* my_ctx, char* line_number);
	~iec104_imp();
	int PollServer(void);
	int Start(void);
	void LogMessage(int* error = 0, const char* name = NULL);
	int Stop(void);
	time_t epoch_from_cp56time2a(const struct cp56time2a* time);
	short rescale_value(double V, double Vmin, double Vmax, int* error);
	double rescale_value_inv(double A, double Vmin, double Vmax, int* error);
	
	////////////////////Middleware//////////////////////
	void check_for_commands(struct iec_item *item);
	void alloc_command_resources(void);
	void free_command_resources(void);
	void get_utc_host_time(struct cp56time2a* time);
	void get_local_host_time(struct cp56time2a* time);
	////////////////////////////////////////////////
};

#endif //IEC104_IMP_H