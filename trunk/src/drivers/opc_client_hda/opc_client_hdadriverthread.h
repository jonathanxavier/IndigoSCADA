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


#ifndef OPC_CLIENT_HDA_DRIVERTHREAD
#define OPC_CLIENT_HDA_DRIVERTHREAD

#include "opc_client_hda.h"
#include "IndentedTrace.h"
#define STRICT
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
#include "opchda.h"	/* The OPC custom interface defintions */
#include "opccomn.h"
////////////////////////////////////
#include "general_defines.h"
#include "IndentedTrace.h"
#include "fifo.h"
#include "fifoc.h"
#include "clear_crc_eight.h"

struct structItem
{
	WCHAR wszName[256]; //Item ID of opc server, i.e. Simulated Card.Simulated Node.Random.R8
	CHAR spname[30]; //IndigoSCADA sample point name
	VARTYPE vt;//item type
	DWORD dwAccessRights; //AccessRights
	DWORD hClient; //index of each item inside the opc client
	DWORD hServer; //index of each item inside the opc server
};

class OPC_CLIENT_HDADRV Opc_client_hda_DriverThread : public DriverThread 
{
	public:

	Opc_client_hda_DriverThread(DriverInstance *parent) : 
		DriverThread(parent)
	{ 
		IT_IT("Opc_client_hda_DriverThread::Opc_client_hda_DriverThread");

		g_dwUpdateRate = ((Opc_client_hda_Instance*)Parent)->Cfg.SampleTime; //milliseconds <--PARAMETRO
		g_dwClientHandle = 1;
		g_dwNumItems = 0;
		g_bWriteEnable = false;
		g_hClientGroup = 0;
		g_dwUpdateTransID = 1;
		g_dwCancelID = 1;

		g_pIOPCCommon = NULL;
		g_pIGroupUnknown = NULL;
//		g_pIOPCSubscriptionMgt = NULL;
		g_iCatInfo = NULL;
//		g_iOpcAreaBrowser = NULL;

		StaticParent = Parent;
		StaticThis = this;

		fifo_control_direction = ((Opc_client_hda_Instance*)Parent)->fifo_control_direction;

//		m_ISubMgt = NULL;
	};

	~Opc_client_hda_DriverThread()
	{
		IT_IT("Opc_client_hda_DriverThread::~Opc_client_hda_DriverThread");
	}

	DWORD g_dwUpdateRate;
	DWORD g_dwClientHandle;
	DWORD g_dwNumItems;
	bool  g_bWriteEnable;
	static bool  g_bWriteComplete;
	static bool  g_bReadComplete;
	DWORD g_dwUpdateTransID;
	DWORD g_dwCancelID;
	static DWORD g_dwReadTransID;
	static DWORD g_dwWriteTransID;
	//static FILE *g_stream; // file log handle
	OPCHANDLE g_hClientGroup;
	static IOPCHDA_Server *g_pIOPCServer;

	/////////////comandi/////////////////////////////////
	static fifo_h fifo_control_direction; //fifo in control direction: SCADA-------------> RTU
	/////////////////////////////////////////////////////

	static bool mandare_eventi;

	IOPCCommon *g_pIOPCCommon;
	IUnknown *g_pIGroupUnknown;
//	IOPCEventSubscriptionMgt *g_pIOPCSubscriptionMgt;
	IOPCServerList *g_iCatInfo;
//	IOPCEventAreaBrowser *g_iOpcAreaBrowser;

	static struct structItem* Item;

	////////begin HDA specific variables///////////////////////////////////////////////////
	DWORD m_dwCookie, m_dwShutdownCookie;
//	IOPCEventSubscriptionMgt* m_ISubMgt;
	////////end HDA specific variables///////////////////////////////////////////////////

	int OpcStart();
	int OpcStop();
	int GetStatus(WORD *pwMav, WORD *pwMiv, WORD *pwB, LPWSTR *pswzV);
    int chek_connection_with_server(void);
	int Update();
	static void ShowError(HRESULT hr, LPCSTR pszError);
	static void ShowMessage(HRESULT hr, LPCSTR pszError, const char* name);
//	void StartErrorLog();
//	void EndErrorLog();
	static LPCSTR GetDateTime();
//	bool Version2();
	int Async2Read(bool bFlag);
	static void SendEvent2(VARIANT *pValue, const FILETIME* ft, DWORD pwQualities, OPCHANDLE phClientItem);
	static DriverInstance *StaticParent;
	static DriverThread *StaticThis;
	static signed __int64 Epoch_from_FILETIME(const FILETIME *fileTime);
	//static QString Epoc_from_FILETIME_fast(const FILETIME *fileTime);

	static void post_val(SpValue &v, QString &name);

	protected:
	void run(); // thread main routine
};

#endif //OPC_CLIENT_HDA_DRIVERTHREAD