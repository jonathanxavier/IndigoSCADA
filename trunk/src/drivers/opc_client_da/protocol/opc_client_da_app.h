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
#ifndef OPC_CLIENT_DA
#define OPC_CLIENT_DA

#ifndef STRICT
#define STRICT
#endif
#define VC_EXTRALEAN

#ifndef _WIN32_DCOM
 #define _WIN32_DCOM
#endif
#define _ATL_FREE_THREADED

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include <atlbase.h>
#include "opcda.h"	/* The OPC custom interface defintions */
#include "opccomn.h"
#include "OpcEnum.h"
#include "opcerror.h"
#include "itrace.h"

struct structItem
{
	WCHAR wszName[256]; //Item ID of opc server, i.e. Simulated Card.Simulated Node.Random.R8
	CHAR spname[200]; //Item ID of opc server, i.e. Simulated Card.Simulated Node.Random.R8 as C string
	VARTYPE vt;//item OPC type
	DWORD dwAccessRights; //AccessRights
	DWORD hClient; //index of each item inside the opc client  <--------------INDEX (starts from 1)
	DWORD hServer; //index of each item inside the opc server
	unsigned int ioa_control_center;//unique inside CASDU
	unsigned int io_list_iec_type; //IEC 104 type
	int readable;
	int writeable;
	float min_measure;
	float max_measure;
	char opc_type[30];
};

enum opc_client_states {
	OPC_CLIENT_NOT_INITIALIZED = 0,
	OPC_CLIENT_INITIALIZED,
	OPC_CLIENT_WAITING_FOR_FIRST_EVENT,
	OPC_CLIENT_ON_LINE
};

#define ITEM_WRITTEN_AT_A_TIME 1

////////////////////////////Middleware///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

class Opc_client_da_imp
{
	public:

	Opc_client_da_imp(char* opc_server_address, char* line_number)
	{ 
		IT_IT("Opc_client_da_imp::Opc_client_da_imp");

		strcpy(ServerIPAddress, opc_server_address);

		g_bWriteComplete = true;
		g_dwReadTransID = 1;
		Config_db = NULL;
		opc_client_state_variable = OPC_CLIENT_NOT_INITIALIZED;
		timer_starts_at_epoch = 0;
		local_server = 0;
		g_iOpcProperties = NULL;
		g_iCatInfo = NULL;
		g_pIOPCBrowse = NULL;
		g_pIGroupUnknown = NULL;
		g_pIOPCCommon = NULL;
		g_pIOPCAsyncIO2 = NULL;
		g_pIOPCItemMgt = NULL;
		g_pIOPCSyncIO  = NULL;
		g_pIOPCAsyncIO = NULL ;
		g_pIOPCGroupStateMgt = NULL;
		g_pIDataObject = NULL;
		g_dwCancelID = 1;
		g_dwUpdateTransID = 1;
		g_hClientGroup = 0;
		g_bVer2 = false;
		g_dwNumItems = 0;
		g_dwClientHandle = 1;
		g_dwUpdateRate = 60000; //in milliseconds
		nThreads = 1;
		hServerRead = NULL;
		id_of_ItemToWrite = 0;

		opc_server_prog_id[0] = '\0';
				
		/////////////////////Middleware/////////////////////////////////////////////////////////////////
		char fifo_monitor_name[150];
		strcpy(fifo_monitor_name,"fifo_monitor_direction");
		strcat(fifo_monitor_name, line_number);
		strcat(fifo_monitor_name, "da");
		
		char fifo_control_name[150];
		strcpy(fifo_control_name,"fifo_control_direction");
		strcat(fifo_control_name, line_number);
		strcat(fifo_control_name, "da");
		///////////////////////////////////Middleware//////////////////////////////////////////////////

		IT_EXIT;
	};
		
	~Opc_client_da_imp()
	{
		IT_IT("Opc_client_da_imp::~Opc_client_da_imp");
		stop_opc_thread();
		Sleep(1000);
		IT_EXIT;
	}

	 DWORD g_dwUpdateRate;
	 DWORD g_dwClientHandle;
	 DWORD g_dwNumItems;
	 bool  g_bWriteComplete;
	 bool  g_bVer2; // version 2.0 flag
	 DWORD g_dwUpdateTransID;
	 DWORD g_dwCancelID;
	 DWORD g_dwReadTransID;
	 static DWORD g_dwWriteTransID;
	 OPCHANDLE g_hClientGroup;
	 VARIANT vCommandValue;
	 OPCHANDLE hServer[ITEM_WRITTEN_AT_A_TIME];
	 VARIANT Val[ITEM_WRITTEN_AT_A_TIME];
	 DWORD id_of_ItemToWrite;
	 static IOPCServer *g_pIOPCServer;

	 int nThreads;

	 static bool fExit;
	 int opc_client_state_variable;
	 time_t timer_starts_at_epoch;
	 static double dead_band_percent;
	 char ServerIPAddress[80];

	// group interfaces
	 IDataObject *g_pIDataObject;
	 IOPCGroupStateMgt *g_pIOPCGroupStateMgt;
	 IOPCAsyncIO *g_pIOPCAsyncIO;
	 IOPCSyncIO *g_pIOPCSyncIO;
	 IOPCItemMgt *g_pIOPCItemMgt;
	 IOPCAsyncIO2 *g_pIOPCAsyncIO2;
	 IOPCCommon *g_pIOPCCommon;
	 IUnknown *g_pIGroupUnknown;
	 IOPCBrowseServerAddressSpace *g_pIOPCBrowse;
	 IOPCServerList *g_iCatInfo;
	 IOPCItemProperties *g_iOpcProperties;
	 OPCHANDLE *hServerRead;

	 static struct structItem* Item; //OPC client items vector, indexed from 0
	 struct structItem* Config_db;
	 int local_server;
	 char opc_server_prog_id[100];
	 /////////////////////Middleware/////////////////////////////////////////////////////////////////
	 ///////////////////////////////////Middleware//////////////////////////////////////////////////

	 int OpcStart(char* OpcServerProgID, char* OpcclassId, char* OpcUpdateRate, char* OpcPercentDeadband);
	 int OpcStop();
	 int GetStatus(WORD *pwMav, WORD *pwMiv, WORD *pwB, LPWSTR *pswzV);
	 int AddItems();
	 static void LogMessage(HRESULT hr = S_OK, LPCSTR pszError = NULL, const char* name = NULL);
	 bool Version2();
	 int Async2Update();
	 static void SendEvent2(VARIANT *pValue, const FILETIME* ft, DWORD pwQualities, OPCHANDLE phClientItem, unsigned char cot);
	 static signed __int64 epoch_from_FILETIME(const FILETIME *fileTime);
	 static void get_utc_host_time(struct cp56time2a* time);
	 time_t epoch_from_cp56time2a(const struct cp56time2a* time);
	 static void epoch_to_cp56time2a(cp56time2a *time, signed __int64 epoch_in_millisec);
	 int check_connection_to_server(void);
	 static short rescale_value(double V, double Vmin, double Vmax, int* error);
	 double rescale_value_inv(double A, double Vmin, double Vmax, int* error);
	 void CreateSqlConfigurationFile(char* sql_file_name, char* opc_path);
	 ////////////////////Middleware////////////////////////////////////////////////////
	 void check_for_commands(struct iec_item *item);
	 void alloc_command_resources(void);
	 void free_command_resources(void);
	 ////////////////////Middleware////////////////////////////////////////////////////
	
	 void stop_opc_thread()
	 {
		fExit = true;
	 };
};

#endif //OPC_CLIENT_DA