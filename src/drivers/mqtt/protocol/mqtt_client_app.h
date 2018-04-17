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
#ifndef MQTT_CLIENT
#define MQTT_CLIENT

#ifndef STRICT
#define STRICT
#endif
#define VC_EXTRALEAN

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "itrace.h"

//MQTT specific/////////////
#include "mqtt_client.h"
#include "mqttexample.h"
#include "mqttnet.h"
////////////////////////////

////////Middleware/////////////
#include "RIPCThread.h"
#include "RIPCFactory.h"
#include "RIPCSession.h"
#include "RIPCServerFactory.h"
#include "RIPCClientFactory.h"
#include "ripc.h"
///////////////////////////////
#include "fifoc.h"

struct structItem
{
	CHAR spname[200]; //Topic name, i.e. Simulated Card/SimulatedNode/Random/R8 as C string
	unsigned int ioa_control_center;//unique inside CASDU
	unsigned int io_list_iec_type; //IEC 104 type
	int readable;
	int writeable;
	float min_measure;
	float max_measure;
	char opc_type[30];
	unsigned int hash_key;
};

////////Middleware/////////////
typedef class MQTT_client_imp* par;

struct subs_args{
	par parent;
};
////////Middleware/////////////

class MQTT_client_imp
{
	public:
	 MQTT_client_imp(char* broker_host_name, char* line_number);
	~MQTT_client_imp();

	 DWORD g_dwUpdateRate;
	 DWORD g_dwNumItems;

	 MQTTCtx mqttCtx;
	 FILE *dump;
	 int nThreads;

	 static bool fExit;
	 int opc_client_state_variable;
	 static double dead_band_percent;
	 char BrokerHostName[80];

	 static struct structItem* Item; //MQTT client topics vector, indexed from 0
	 struct structItem* Config_db;
	 /////////////Middleware///////////////////////////////
	 u_int n_msg_sent_monitor_dir;
	 u_int n_msg_sent_control_dir;
	 int exit_threads;
	 fifo_h fifo_control_direction;
	 int          port;
	 char const*  hostname;
	 RIPCFactory* factory1;
	 RIPCFactory* factory2;
	 RIPCSession* session1;
	 RIPCSession* session2;
	 static RIPCQueue*   queue_monitor_dir;
	 RIPCQueue*   queue_control_dir;
	 struct subs_args arg;
	 //////////////////////////////////////////////////////

	 int MQTTStart(char* SubscribeTopicName, char*UserName, char* Password, int Port, char* ClientID);
	 int MQTTStop();
	 int AddItems();
	 int Update();
	 static void get_utc_host_time(struct cp56time2a* time);
	 static void get_local_host_time(struct cp56time2a* time);
	 time_t epoch_from_cp56time2a(const struct cp56time2a* time);
	 static void epoch_to_cp56time2a(cp56time2a *time, signed __int64 epoch_in_millisec);
	 static short rescale_value(double V, double Vmin, double Vmax, int* error);
	 double rescale_value_inv(double A, double Vmin, double Vmax, int* error);
	 void CreateSqlConfigurationFile(char* sql_file_name);
	 void check_for_commands(struct iec_item *item);
};

#endif //MQTT_CLIENT