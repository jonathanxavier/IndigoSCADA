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
////////////////////////////////////////////////////////////////////////////////
// OPC Alarms & Events 1.10
////////////////////////////////////////////////////////////////////////////////

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
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

// check for Visual C++ 5 w/SP3
#if _ATL_VER < 0x0202
 #error minimum requirements: Visual C++ 5 w/SP3
#endif

#include "opc_ae.h"	/* The OPC custom interface defintions */
#include "opccomn_i.c"
#include "opccomn.h"
#include "OpcEnum.h"
#include "opcae_er.h"
#include "general_defines.h"
#include "IndentedTrace.h"

#include "opc_client_ae_instance.h"
#include "opc_client_aedriverthread.h"

#define MAX_KEYLEN 256

//
// Smart pointer typedef declarations
//

//_COM_SMARTPTR_TYPEDEF(OPCEventServerCATID, __uuidof(OPCEventServerCATID));
//_COM_SMARTPTR_TYPEDEF(IOPCEventServer, __uuidof(IOPCEventServer));
//_COM_SMARTPTR_TYPEDEF(IOPCEventSubscriptionMgt, __uuidof(IOPCEventSubscriptionMgt));
//_COM_SMARTPTR_TYPEDEF(IOPCEventAreaBrowser, __uuidof(IOPCEventAreaBrowser));
//_COM_SMARTPTR_TYPEDEF(IOPCEventSink, __uuidof(IOPCEventSink));
//_COM_SMARTPTR_TYPEDEF(IOPCEventServer2, __uuidof(IOPCEventServer2));
//_COM_SMARTPTR_TYPEDEF(IOPCEventSubscriptionMgt2, __uuidof(IOPCEventSubscriptionMgt2));



//typedef _COM_SMARTPTR<IOPCEventSubscriptionMgt, &IID> IOPCEventSubscriptionMgtPtr;
//typedef _COM_SMARTPTR<IOPCEventServer, &IID> IOPCEventServerPtr;

//typedef _COM_SMARTPTR<_COM_SMARTPTR_LEVEL2<IOPCEventSubscriptionMgt, &IID> > IOPCEventSubscriptionMgtPtr;
//typedef _COM_SMARTPTR<_COM_SMARTPTR_LEVEL2<IOPCEventServer, &IID> > IOPCEventServerPtr;

#if _MSC_VER > 1100  // VC 6.0 and higher
	#define GUID_CAST( a )		(const_cast<_GUID*>(a))
#else				// VC 5.0
	#define GUID_CAST( a )		((struct GUID *)a)
#endif

class ATL_NO_VTABLE COPCEventSink : 
	public CComObjectRoot,
	public IOPCEventSink
{
public:
	COPCEventSink(){}

BEGIN_COM_MAP(COPCEventSink)
	//COM_INTERFACE_ENTRY_IID( __uuidof(IOPCEventSink), IOPCEventSink)
	COM_INTERFACE_ENTRY(IOPCEventSink)
END_COM_MAP()

    HRESULT __stdcall OnEvent( 
            /* [in] */ OPCHANDLE hClientSubscription,
            /* [in] */ BOOL bRefresh,
            /* [in] */ BOOL bLastRefresh,
            /* [in] */ DWORD dwCount,
            /* [size_is][in] */ ONEVENTSTRUCT *pEvents) 
	{
		IT_IT("COPCEventSink::OnEvent");
		
		return S_OK;
	};


    virtual HRESULT __stdcall raw_OnEvent (
        unsigned long hClientSubscription,
        long bRefresh,
        long bLastRefresh,
        unsigned long dwCount,
        ONEVENTSTRUCT * pEvents );

};


HRESULT COPCEventSink::raw_OnEvent (
        unsigned long hClientSubscription,
        long bRefresh,
        long bLastRefresh,
        unsigned long dwCount,
        ONEVENTSTRUCT * pEvents )
{
/*
	MoveEvent MvEvent;

	for( DWORD i = 0; i < dwCount; i++ )
	{
		TRACE( "%ls  %ls   %ls   %ls\n", pEvents[i].szSource, 
								pEvents[i].szMessage,
								pEvents[i].szConditionName, 
								pEvents[i].szSubconditionName );
		curEvent = pEvents[i];
		MvEvent.PushEvent(curEvent);
	}
*/
	return S_OK;
}

typedef CComObject<COPCEventSink> CComCOPCEventSink;

int Opc_client_ae_DriverThread::OpcStart()
{
	IT_IT("Opc_client_ae_DriverThread::OpcStart");

	char show_msg[150];

	//128.1.1.2 ---> ENLABSERVOPC
	//128.1.1.25 ---> HMI2

	TCHAR  ServerIPAddress[80];

	strcpy(ServerIPAddress, ((Opc_client_ae_Instance*)Parent)->Cfg.OpcServerIPAddress);

	if((strlen(ServerIPAddress) == 0))
	{
		local_server = 1;
	}
	
	if(local_server)
	{
		//TODO: finisch to implement the local connection apa+++ 14-04-2011
		//COM connection
		
		// browse registry for OPC Servers
		HKEY hk = HKEY_CLASSES_ROOT;
		TCHAR szKey[MAX_KEYLEN];

		for(int nIndex = 0; ::RegEnumKey(hk, nIndex, szKey, MAX_KEYLEN) == ERROR_SUCCESS; nIndex++)
		{
			HKEY hProgID;
			TCHAR szDummy[MAX_KEYLEN];

			if(::RegOpenKey(hk, szKey, &hProgID) == ERROR_SUCCESS)
			{
				LONG lSize = MAX_KEYLEN;

				if(::RegQueryValue(hProgID, "OPC", szDummy, &lSize) == ERROR_SUCCESS)
				{
					printf("%s\n",szKey);
					IT_COMMENT1("%s",szKey);
				}

				::RegCloseKey(hProgID);
			}
		}

		WCHAR wszServerName[100];
		
		HRESULT hr;

		USES_CONVERSION;

		TCHAR serv[100];
		
		strcpy(serv, ((Opc_client_ae_Instance*)Parent)->Cfg.OpcServerProgID);

		wcscpy(wszServerName, T2W(serv));

		CLSID clsid;
			
		hr = ::CLSIDFromProgID(wszServerName, &clsid );

		if(FAILED(hr))
		{
			ShowError(hr,"CLSIDFromProgID()");
			return(1);
		}

		printf("Server ID found.\n");
		IT_COMMENT("Server ID found.\n");
		
		hr = ::CoInitializeEx(NULL,COINIT_MULTITHREADED); // setup COM lib

		if(FAILED(hr))
		{
			ShowError(hr,"CoInitializeEx()");
			return(1);
		}

		//////////////////////start here OPC example/////////////////////////////////
//		USES_CONVERSION;
		
		ICatInformation* pcr = NULL;
//		HRESULT hr=S_OK;

		hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_ALL, IID_ICatInformation, (void**)&pcr);

		IEnumCLSID* pEnumCLSID;

		CLSID catid =  __uuidof(OPCEventServerCATID);
		pcr->EnumClassesOfCategories(1, &catid, 1, &catid, &pEnumCLSID);

		//get 10 at a time for efficiency
		unsigned long c;
		CLSID clsids[10];

		char  strText[100];
		WCHAR* lpszProgID = NULL;
		//int item;

		while(SUCCEEDED(hr=pEnumCLSID->Next(10,clsids, &c)) && c)
		{
			for(unsigned long i =0;i<c;i++)
			{
				clsids[i];
				hr=ProgIDFromCLSID(clsids[i],&lpszProgID);

				strcpy(strText, W2T(lpszProgID));

				CoTaskMemFree( lpszProgID );

				//item = m_AlarmServerList.AddString(strText);

				//if(item==LB_ERR)
				//	return(1);

				printf("%s\n", strText);

				CLSID* pData;

				pData = new CLSID;

				*pData = clsids[i];

				//item = m_AlarmServerList.SetItemDataPtr(item,pData);

				//if(item==LB_ERR)
				//	return(1);
			}
		}

		pcr->Release();

		/*
		int item = m_AlarmServerList.GetCurSel();

		if(item == LB_ERR)
			return(1);

		CLSID* pData=NULL;

		pData = (CLSID *)m_AlarmServerList.GetItemDataPtr(item);

		IOPCEventServerPtr			m_IEventServer;

		hr = m_IEventServer.CreateInstance(*pData);

		if(hr==S_OK)
		{
			UINT count = m_AlarmServerList.GetCount();

			if(item == LB_ERR)
				return(1);


			for(UINT i=0;i<count;i++)
			{
				pData = (CLSID *)m_AlarmServerList.GetItemDataPtr(i);
				delete pData;
			}
		}

		*/

		/////////////////////////end here OPC example////////////////////////////////

		// Create a running object from that class ID
		// (CLSCTX_ALL will allow in-proc, local and remote)

		hr = ::CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_ALL, IID_ICatInformation, (void**)&g_pIOPCServer);

		//hr = ::CoCreateInstance(clsid, NULL, CLSCTX_ALL, IID_ICatInformation, (void**)&g_pIOPCServer);

		if(FAILED(hr) || (g_pIOPCServer == NULL))
		{
			if(FAILED(hr)){ ShowError(hr,"CoCreateInstance()");}
			printf("You may not have registered the OPC Proxy dll!\n");
			IT_COMMENT("You may not have registered the OPC Proxy dll!");

			return(1);
		}
			
		printf("Connected to local server.\n");
		IT_COMMENT("Connected to local server");

		sprintf(show_msg, "Connected to local server");
		Opc_client_ae_DriverThread::ShowMessage(S_OK, "", show_msg);

		WORD wMajor, wMinor, wBuild;

		LPWSTR pwsz = NULL;
		
		if(!GetStatus(&wMajor, &wMinor, &wBuild, &pwsz))
		{
			char ver[150];
			sprintf(ver,"Server version: %d.%d.%d, %s", wMajor, wMinor, wBuild, W2T(pwsz));
			printf("%s\n\n",ver);
			IT_COMMENT4("Version: %d.%d.%d, %s", wMajor, wMinor, wBuild,W2T(pwsz));
			
			Opc_client_ae_DriverThread::ShowMessage(S_OK, "",ver);
			::CoTaskMemFree(pwsz);
		}
/*
		g_bVer2 = Version2();

		if(g_bVer2)
		{
			printf("Server supports OPC 2.0 interfaces\n\n");
			IT_COMMENT("Server supports OPC 2.0 interfaces");
		}

		hr = g_pIOPCServer->QueryInterface(IID_IOPCEventSubscriptionMgt, (void**)&g_pIOPCSubscriptionMgt);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCEventSubscriptionMgt");
		}

		float fTemp = 0.0f;
		long lTimeBias = 0;
		DWORD dwRevisedUpdateRate = 0;

		// create an in-active group
		// NOTE: 1st param must not be a NULL or the proxy will puke
		
		
		hr = g_pIOPCServer->AddGroup(L"",					// [in] Server name, if NULL OPC Server will generate a unique name
									 TRUE		,			// [in] State of group to add
									 g_dwUpdateRate,		// [in] Requested update rate for group (ms)
									 1234,					// [in] Client handle to OPC Group
									 &lTimeBias,			// [in] Time 
									 &fTemp,				// [in] Percent Deadband
									 0,						// [in] Localization ID
									 &g_hClientGroup,		// [out] Server Handle to group
									 &dwRevisedUpdateRate,	// [out] Revised update rate
									 IID_IUnknown,			// [in] Type of interface desired
									 &g_pIGroupUnknown);	// [out] where to store the interface pointer
			
		if(FAILED(hr))
		{
			ShowError(hr,"AddGroup()");
			g_pIOPCServer->Release();
			return(1);
		}

		printf("Group added, update rate = %ld.\n", dwRevisedUpdateRate);
		IT_COMMENT1("Group added, update rate = %ld.", dwRevisedUpdateRate);
		
		// Get pointer to OPC Server interfaces required for this program.

		hr = g_pIGroupUnknown->QueryInterface(IID_IDataObject, (void**)&g_pIDataObject);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IDataObject)");
		}

		hr = g_pIGroupUnknown->QueryInterface(IID_IOPCGroupStateMgt, (void**)&g_pIOPCGroupStateMgt);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCGroupStateMgt)");
		}

		hr = g_pIGroupUnknown->QueryInterface(IID_IOPCAsyncIO, (void**)&g_pIOPCAsyncIO);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCAsyncIO)");
		}

		hr = g_pIGroupUnknown->QueryInterface(IID_IOPCItemMgt, (void**)&g_pIOPCItemMgt);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCItemMgt)");
		}

		hr = g_pIGroupUnknown->QueryInterface(IID_IOPCSyncIO, (void**)&g_pIOPCSyncIO);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCSyncIO)");
		}

		if(g_bVer2)
		{
			hr = g_pIGroupUnknown->QueryInterface(IID_IOPCAsyncIO2, (void**)&g_pIOPCAsyncIO2);
			if(FAILED(hr))
			{
				ShowError(hr,"QueryInterface(IID_IOPCAsyncIO2)");
			}

			hr = g_pIOPCServer->QueryInterface(IID_IOPCCommon, (void**)&g_pIOPCCommon);
			if(FAILED(hr))
			{
				ShowError(hr,"QueryInterface(IID_IOPCCommon)");
			}
			else
			{
				g_pIOPCCommon->SetClientName(L"IndigoSCADA OPC Client");
			}
		}
		
		if(FAILED(hr))
		{
			g_pIOPCServer->Release();
			printf("OPC error: secondary QI failed\n");
			IT_COMMENT("OPC error: secondary QI failed");
			ShowError(hr,"secondary QI failed");
			return(1);
		}

		if(dwRevisedUpdateRate != g_dwUpdateRate)
		{
			g_dwUpdateRate = dwRevisedUpdateRate;
		}

		printf("Active Group interface added.\n");
		IT_COMMENT("Active Group interface added.");
	*/
	}
	else
	{
		//DCOM connection

		printf("Trying to connect to remote A&E server on machine with IP: %s\n", ServerIPAddress);
		sprintf(show_msg, "Trying to connect to remote A&E server on machine with IP: %s\n", ServerIPAddress);

		ShowMessage(S_OK, "", show_msg);
		
		HRESULT	hr = ::CoInitializeEx(NULL,COINIT_MULTITHREADED); // setup COM lib

		if(FAILED(hr))
		{
			printf("CoInitializeEx failed\n");
			ShowError(hr,"CoInitializeEx failed");
			return 1;
		}

		CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
		
		COAUTHINFO athn;
		ZeroMemory(&athn, sizeof(COAUTHINFO));
		// Set up the NULL security information
		athn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;
		//athn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
		athn.dwAuthnSvc = RPC_C_AUTHN_WINNT;
		athn.dwAuthzSvc = RPC_C_AUTHZ_NONE;
		athn.dwCapabilities = EOAC_NONE;
		athn.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
		athn.pAuthIdentityData = NULL;
		athn.pwszServerPrincName = NULL;

		USES_CONVERSION;

		COSERVERINFO remoteServerInfo;
		ZeroMemory(&remoteServerInfo, sizeof(COSERVERINFO));
		remoteServerInfo.pAuthInfo = &athn;
		remoteServerInfo.pwszName = T2OLE(ServerIPAddress);
		//printf("Remote host: %s\n", OLE2T(remoteServerInfo.pwszName));

		MULTI_QI reqInterface;
		reqInterface.pIID = &IID_IOPCServerList; //requested interface
		reqInterface.pItf = NULL;
		reqInterface.hr = S_OK;
								//requested class
		hr = CoCreateInstanceEx(CLSID_OpcServerList,NULL, CLSCTX_REMOTE_SERVER, &remoteServerInfo, 1, &reqInterface);
		
		if (FAILED(hr))
		{
			printf("OPC error:Failed to get remote interface, %x\n", hr);
			ShowError(hr,"Failed to get remote interface");
			return 1;
		}
		
		g_iCatInfo = (IOPCServerList *)reqInterface.pItf;

		///////////////////////////////////////

		CATID Implist[1];

		Implist[0] = __uuidof(OPCEventServerCATID);

		//CLSID catid =  __uuidof(OPCEventServerCATID);

		IEnumCLSID *iEnum = NULL;

		hr = g_iCatInfo->EnumClassesOfCategories(1, Implist,0, NULL,&iEnum);

		if (FAILED(hr))
		{
			printf("OPC error:Failed to get enum for categeories, %x\n", hr);
			ShowError(hr,"Failed to get enum for categeories");
			return 1;
		}

		GUID glist;

		ULONG actual;

		printf("Available A&E server(s) on remote machine:\n");

		while((hr = iEnum->Next(1, &glist, &actual)) == S_OK)
		{
			WCHAR *progID;
			WCHAR *userType;
			HRESULT res = g_iCatInfo->GetClassDetails(glist, &progID, &userType);/*ProgIDFromCLSID(glist, &progID)*/;

			if(FAILED(res))
			{
				printf("OPC error:Failed to get ProgId from ClassId, %x\n",res);
				ShowError(res,"Failed to get ProgId from ClassId");
				return 1;
			}
			else 
			{
				USES_CONVERSION;
				char * str = OLE2T(progID);
				char * str1 = OLE2T(userType);
				printf("%s\n", str);
				::CoTaskMemFree(progID);
				::CoTaskMemFree(userType);
			}
		}
		
		////////////////////////end getListOfDAServers

		TCHAR serverName[100];
				
		strcpy(serverName, ((Opc_client_ae_Instance*)Parent)->Cfg.OpcServerProgID);
				
		if((strlen(serverName) == 0))
		{
			printf("OPC error: Please supply ProgID\n");
			ShowError(S_FALSE,"Please supply ProgID");
			return 1;
		}
		
		//Get CLSID From RemoteRegistry
				
		char _progID[100];
		strcpy(_progID, serverName);

		char keyName[100];
		
		strcpy(keyName,"SOFTWARE\\Classes\\");
		strcat(keyName, _progID);
		strcat(keyName, "\\Clsid");

		HKEY remoteRegHandle;
		HKEY keyHandle;
		char classIdString[100];
		CLSID classId;

		hr = RegConnectRegistry(ServerIPAddress, HKEY_LOCAL_MACHINE, &remoteRegHandle);

		if(SUCCEEDED(hr))
		{
		   hr = RegOpenKeyEx(remoteRegHandle, keyName, 0, KEY_READ, &keyHandle);

		   if(SUCCEEDED(hr))
		   {
			   DWORD entryType;

			   unsigned bufferSize = 100;

			   hr = RegQueryValueEx(keyHandle, NULL, 0, &entryType, (LPBYTE)&classIdString, (LPDWORD)&bufferSize);

			   if(FAILED(hr))
			   {
					printf("RegQueryValueEx failed");
					ShowError(hr,"RegQueryValueEx failed");
					return 1;
			   }
			   else
			   {
					USES_CONVERSION;

					LPOLESTR sz = A2W(classIdString);

					hr = CLSIDFromString(sz,&classId);

					if(FAILED(hr))
					{
						printf("CLSIDFromString failed");
						ShowError(hr,"CLSIDFromString failed");
						return 1;
					}
			   }
		   }
		   else
		   {
				ShowError(hr,"RegOpenKeyEx failed");
		   }
		}	
	    else
		{
			ShowError(hr,"RegConnectRegistry failed");
		}

        RegCloseKey(remoteRegHandle);
	    RegCloseKey(keyHandle);

		////////////////////end Get CLSID From Remote Registry

		ZeroMemory(&athn, sizeof(COAUTHINFO));
		// Set up the NULL security information
		athn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_CONNECT;
		//athn.dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;
		athn.dwAuthnSvc = RPC_C_AUTHN_WINNT;
		athn.dwAuthzSvc = RPC_C_AUTHZ_NONE;
		athn.dwCapabilities = EOAC_NONE;
		athn.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
		athn.pAuthIdentityData = NULL;
		athn.pwszServerPrincName = NULL;
		
		ZeroMemory(&remoteServerInfo, sizeof(COSERVERINFO));
		remoteServerInfo.pAuthInfo = &athn;
		
		remoteServerInfo.pwszName = T2OLE(ServerIPAddress);

		//printf("%s\n", OLE2T(remoteServerInfo.pwszName));
		
		reqInterface.pIID = &IID_IUnknown; //requested interface
		reqInterface.pItf = NULL;
		reqInterface.hr = S_OK;
		                          //requsted class
		hr = CoCreateInstanceEx(classId,NULL, CLSCTX_REMOTE_SERVER, &remoteServerInfo, 1, &reqInterface);	
		
		if (FAILED(hr))
		{
			printf("OPC error:Failed to get remote interface, %x\n", hr);
			ShowError(hr,"Failed to get remote interface");
			return 1;
		}
		
		IUnknown * pIUnknown = NULL;

		pIUnknown = reqInterface.pItf;

		/////end make Remote Object

		hr = pIUnknown->QueryInterface(IID_IOPCEventServer, (void**)&g_pIOPCServer);

		if (FAILED(hr))
		{
			printf("OPC error:Failed to obtain IID_IOPCEventServer interface from server, %x\n", hr);
			ShowError(hr,"Failed to obtain IID_IOPCEventServer interface from server");
			return 1;
		}

		printf("Connected to server %s.\n", ServerIPAddress);

		sprintf(show_msg, "Connected to A&E server on machine with IP: %s\n", ServerIPAddress);
		Opc_client_ae_DriverThread::ShowMessage(S_OK, "", show_msg);

		WORD wMajor, wMinor, wBuild;
		LPWSTR pwsz = NULL;

		if(!GetStatus(&wMajor, &wMinor, &wBuild, &pwsz))
		{
			char ver[250];
			//printf("Version: %d.%d.%d\n", wMajor, wMinor, wBuild);
			//printf("%ls\n\n",pwsz);
			sprintf(ver,"Server version: %d.%d.%d, %s", wMajor, wMinor, wBuild, W2T(pwsz));
			printf("%s\n\n",ver);

			IT_COMMENT4("Version: %d.%d.%d, %s", wMajor, wMinor, wBuild,W2T(pwsz));
			Opc_client_ae_DriverThread::ShowMessage(S_OK, "",ver);
			::CoTaskMemFree(pwsz);
		}
	
		/*
		g_bVer2 = Version2();

		if(g_bVer2)
		{
			printf("Server supports OPC 2.0 interfaces\n\n");
		}
				
		hr = g_pIOPCServer->QueryInterface(IID_IOPCEventSubscriptionMgt, (void**)&g_pIOPCSubscriptionMgt);

		if (FAILED(hr))
		{
			printf("OPC error:Failed to obtain IID_IOPCEventSubscriptionMgt interface %x\n",hr);
			ShowError(hr,"Failed to obtain IID_IOPCEventSubscriptionMgt interface");
			return 1;
		}
		
		hr = g_pIOPCServer->QueryInterface(IID_IOPCEventAreaBrowser, (void**)&g_iOpcAreaBrowser);

		if (FAILED(hr))
		{
			printf("OPC error:Failed to obtain IID_IOPCEventAreaBrowser interface, %x\n",hr);
			ShowError(hr,"Failed to obtain IID_IOPCEventAreaBrowser interface");
			return 1;
		}

		float fTemp = 0.0f;

		long lTimeBias = 0;

		DWORD dwRevisedUpdateRate = 0;

		// create an in-active group
		// NOTE: 1st param must not be a NULL or the proxy will puke
		hr = g_pIOPCServer->AddGroup(L"",					// [in] Server name, if NULL OPC Server will generate a unique name
									 TRUE		,			// [in] State of group to add
									 g_dwUpdateRate,		// [in] Requested update rate for group (ms)
									 1234,					// [in] Client handle to OPC Group
									 &lTimeBias,			// [in] Time 
									 &fTemp,				// [in] Percent Deadband
									 0,						// [in] Localization ID
									 &g_hClientGroup,		// [out] Server Handle to group
									 &dwRevisedUpdateRate,	// [out] Revised update rate
									 IID_IUnknown,			// [in] Type of interface desired
									 &g_pIGroupUnknown);	// [out] where to store the interface pointer

		if(FAILED(hr))
		{
			ShowError(hr,"AddGroup()");
			g_pIOPCServer->Release();
			return 1;
		}

		printf("Group added, update rate = %ld.\n", dwRevisedUpdateRate);

		// Get pointer to OPC Server interfaces required for this program.
		hr = g_pIGroupUnknown->QueryInterface(IID_IDataObject, (void**)&g_pIDataObject);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IDataObject)");
		}

		hr = g_pIGroupUnknown->QueryInterface(IID_IOPCGroupStateMgt, (void**)&g_pIOPCGroupStateMgt);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCGroupStateMgt)");
		}

		hr = g_pIGroupUnknown->QueryInterface(IID_IOPCAsyncIO, (void**)&g_pIOPCAsyncIO);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCAsyncIO)");
		}

		hr = g_pIGroupUnknown->QueryInterface(IID_IOPCItemMgt, (void**)&g_pIOPCItemMgt);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCItemMgt)");
		}

		hr = g_pIGroupUnknown->QueryInterface(IID_IOPCSyncIO, (void**)&g_pIOPCSyncIO);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCSyncIO)");
		}

		if(g_bVer2)
		{
			hr = g_pIGroupUnknown->QueryInterface(IID_IOPCAsyncIO2, (void**)&g_pIOPCAsyncIO2);

			if(FAILED(hr))
			{
				ShowError(hr,"QueryInterface(IID_IOPCAsyncIO2)");
			}

			hr = g_pIOPCServer->QueryInterface(IID_IOPCCommon, (void**)&g_pIOPCCommon);

			if(FAILED(hr))
			{
				ShowError(hr,"QueryInterface(IID_IOPCCommon)");
			}
			else
			{
				g_pIOPCCommon->SetClientName(L"IndigoSCADA OPC Client");
			}
		}
		//
		if(FAILED(hr))
		{
			g_pIOPCServer->Release();
			printf("OPC error: secondary QI failed\n");
			ShowError(hr,"secondary QI failed");
			return 1;
		}

		if(dwRevisedUpdateRate != g_dwUpdateRate)
		{
			g_dwUpdateRate = dwRevisedUpdateRate;
		}

		printf("Active Group interface added.\n");
		*/

		int bActive;
		DWORD dwBufferTime;
		DWORD dwMaxSize;
		DWORD hClientSubscription;
		DWORD dwRevisedBufferTime;
		DWORD dwRevisedMaxSize;
		//IOPCEventSubscriptionMgtPtr m_ISubMgt;
		IOPCEventSubscriptionMgt* m_ISubMgt = NULL;
		//IOPCEventServerPtr		m_IEventServer;
		IOPCEventServer*		m_IEventServer = g_pIOPCServer;
		DWORD m_dwCookie;
		CComCOPCEventSink   *m_pSink = NULL;

		//ATLTRY(m_pSink = new CComCOPCEventSink);

		//if(m_pSink == NULL)
		//{
		//	ShowError(E_OUTOFMEMORY,"new CComCOPCEventSink");
		//	return 1;
		//}

		//bActive=m_ActiveCheck.GetCheck();
		//dwBufferTime = TexttoDWORD(m_BufferTime);
		//dwMaxSize = TexttoDWORD(m_MaxSize);  

		dwMaxSize = 1000; //this is parameter
		bActive = 1; //this is parameter

	    //server should check for maxsize of 0 however client should never pass it
		if(!dwMaxSize)
		{
			dwMaxSize=1;
			//DWORDtoText(m_MaxSize,dwMaxSize);
		}

		//hClientSubscription = TexttoDWORD(m_ClientSub);
		hClientSubscription = 1243272; //this is parameter ?
		dwBufferTime = 10000; //this is parameter

		bool  m_bNewSubscription = 1; //this is parameter

		if(m_bNewSubscription)
		{
			hr = m_IEventServer->CreateEventSubscription( bActive,
								dwBufferTime,
								dwMaxSize,
								hClientSubscription,
								//GUID_CAST(&__uuidof(m_ISubMgt)), //apa--- 17-04-2011
								IID_IOPCEventSubscriptionMgt, //apa+++ 17-04-2011
							   (IUnknown **)&m_ISubMgt,
							   &dwRevisedBufferTime,
							   &dwRevisedMaxSize );

			if(hr != S_OK)
			{
				//MessageBox("Fialed to Create Subscription");
				return 1;
			}
			
			// create advise
			CComObject<COPCEventSink>::CreateInstance(&m_pSink);
			m_dwCookie = 99;

			//IUnknownPtr pUnk;
			IUnknown* pUnk;

			m_pSink->_InternalQueryInterface( __uuidof(IUnknown), (void**)&pUnk );

			hr = AtlAdvise(m_ISubMgt, pUnk, __uuidof(IOPCEventSink), &m_dwCookie );
			
		}
		else
		{
			/*
			if(m_NullCheck.GetCheck())  //for testing purposes
			{
				long *pbActive;
				DWORD *pdwBufferTime;
				DWORD *pdwMaxSize;
				pbActive=NULL;	
				pdwBufferTime = NULL;
				pdwMaxSize = NULL;

				hr=m_ISubMgt->SetState(pbActive,pdwBufferTime,pdwMaxSize,
					hClientSubscription,&dwRevisedBufferTime,&dwRevisedMaxSize);
			}
			else
			*/
			{
				hr = m_ISubMgt->SetState(&bActive,&dwBufferTime,&dwMaxSize,
					hClientSubscription,&dwRevisedBufferTime,&dwRevisedMaxSize);
			}

			if(hr != S_OK)
			{
				//MessageBox("Fialed to Set State");
				return 1;
			}
		}

		//if(dwRevisedBufferTime != dwBufferTime)
		//{
			//DWORDtoText(m_BufferTime,dwRevisedBufferTime);
		//}

		//if(dwRevisedMaxSize != dwMaxSize)
		//{
			//DWORDtoText(m_MaxSize,dwRevisedMaxSize);
		//}

		//IOPCEventSubscriptionMgt2Ptr ISubMgt2 = m_ISubMgt;
		IOPCEventSubscriptionMgt2* ISubMgt2 = (struct IOPCEventSubscriptionMgt2*)m_ISubMgt;

		if(ISubMgt2 != NULL)
		{
			DWORD dwRevisedKeepAliveTime = 0;
			// set the keep-alive to 3X the dwRevisedBufferTime
			hr = ISubMgt2->SetKeepAlive( 3 * dwRevisedBufferTime, &dwRevisedKeepAliveTime );
		}
	}

    return(0);
}

int Opc_client_ae_DriverThread::OpcStop()
{
	IT_IT("Opc_client_ae_DriverThread::OpcStop");

	if(Item)
	{
		free(Item);
		Item = NULL;
	}

	// terminate server and it will clean up itself
	if(g_pIOPCServer) while(g_pIOPCServer->Release()) ;
	::CoUninitialize();

	printf("Server and all group interfaces terminated.\n");

	ShowMessage(S_OK,"","Server and all group interfaces terminated");
	return 1;
}

int Opc_client_ae_DriverThread::GetStatus(WORD *pwMav, WORD *pwMiv, WORD *pwB, LPWSTR *pszV)
{
	IT_IT("Opc_client_ae_DriverThread::GetStatus");

	*pwMav = 0;
	*pwMiv = 0;
	*pwB = 0;
	*pszV = NULL;
	OPCEVENTSERVERSTATUS *pStatus = NULL;
	if(g_pIOPCServer == NULL) return E_POINTER;
	HRESULT hr = g_pIOPCServer->GetStatus(&pStatus);
	if(FAILED(hr) || 
		(pStatus == NULL) 
		//|| (pStatus->dwServerState != OPCAE_STATUS_RUNNING) ||
		//(pStatus->dwServerState != OPCAE_STATUS_TEST)
		)
	{
		if(FAILED(hr))	ShowError(hr,"GetStatus()");
		if(pStatus != NULL) ::CoTaskMemFree(pStatus);
		return E_FAIL;
	}

	*pwMav = pStatus->wMajorVersion;
	*pwMiv = pStatus->wMinorVersion;
	*pwB = pStatus->wBuildNumber;
	*pszV = pStatus->szVendorInfo;
	::CoTaskMemFree(pStatus);

	return 0;
}

/*
bool Opc_client_ae_DriverThread::Version2()
{
	IT_IT("Opc_client_ae_DriverThread::Version2");

	if(g_pIOPCServer == NULL) return false;
	IConnectionPointContainer *pCPC = NULL;
	if(FAILED(g_pIOPCServer->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC)))
	{
		return false;
	}
	pCPC->Release();
	return true;
}
*/

void Opc_client_ae_DriverThread::ShowMessage(HRESULT hr, LPCSTR pszError, const char* name)
{
	/*
	LPWSTR pwszError = NULL;

	if((g_pIOPCServer != NULL) && SUCCEEDED(g_pIOPCServer->GetErrorString(hr, 0, &pwszError)))
	{
		QString err;
		USES_CONVERSION;
		err.sprintf("%s: %s, %s", name, pszError, W2T(pwszError));

		postEvent(StaticParent,new DriverEvent(StaticThis, DriverEvent::OpSendEventString,0,err));
		
		::CoTaskMemFree(pwszError);
	}
	else
	*/
	{
		QString err;
		err.sprintf("%s: %s, %lX", name, pszError, hr);
		postEvent(StaticParent,new DriverEvent(StaticThis, DriverEvent::OpSendEventString,0,err));
	}
}

/*

void Opc_client_ae_DriverThread::StartErrorLog()
{
	char opc_log_file[_MAX_PATH];
		
	opc_log_file[0] = '\0';

	if(GetModuleFileName(NULL, opc_log_file, _MAX_PATH))
	{
		*(strrchr(opc_log_file, '\\')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(opc_log_file, '\\')) = '\0';        // Strip \\bin off path
    }

	strcat(opc_log_file, "\\logs\\OPC_client.log");
	
	g_stream = fopen(opc_log_file, _T("w"));

	if(g_stream)
	{
		fprintf(g_stream, "%sIndigoSCADA OPC Client Start.\n", GetDateTime());
	}
}

void Opc_client_ae_DriverThread::EndErrorLog()
{
	if(g_stream)
	{
		fprintf(g_stream, "%sIndigoSCADA OPC Client End.\n", GetDateTime());
		fclose(g_stream);
	}
}

LPCSTR Opc_client_ae_DriverThread::GetDateTime()
{
	static char sz[128];
	char sz2[128];
	_strdate(sz);
	strcat(sz, " ");
	_strtime(sz2);
	strcat(sz, sz2);
	strcat(sz, "|");
	return sz;
}
*/

