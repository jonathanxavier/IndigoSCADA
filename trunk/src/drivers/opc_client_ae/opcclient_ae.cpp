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

#include "opcaeclasses.h"

int Opc_client_ae_DriverThread::Update()
{
	IT_IT("Opc_client_ae_DriverThread::Update");

	while(true)
	{
		if(fExit)
		{
			mandare_eventi = false;
			IT_COMMENT("Opc_client_ae_DriverThread exiting....");
			m_hevtEnd.signal();
			break; //terminate the thread
		}

		::Sleep(g_dwUpdateRate);
	}

	return 0;
}

int Opc_client_ae_DriverThread::OpcStart()
{
	IT_IT("Opc_client_ae_DriverThread::OpcStart");

	char show_msg[150];

	TCHAR  ServerIPAddress[80];

	strcpy(ServerIPAddress, ((Opc_client_ae_Instance*)Parent)->Cfg.OpcServerIPAddress);

	if((strlen(ServerIPAddress) == 0))
	{
		local_server = 1;
	}
	
	if(local_server)
	{
		//TODO: finish to implement the local connection apa+++ 14-04-2011
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

		IOPCEventServerPtr			g_pIOPCServer;

		hr = g_pIOPCServer.CreateInstance(*pData);

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
	}
	else
	{
		//DCOM connection

		printf("Trying to connect to remote A&E server on machine with IP: %s\n", ServerIPAddress);
		sprintf(show_msg, "Trying to connect to remote A&E server on machine with IP: %s", ServerIPAddress);

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
		
		////////////////////////end getListOfAEServers

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

		sprintf(show_msg, "Connected to A&E server on machine with IP: %s", ServerIPAddress);
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

		hr = g_pIOPCServer->QueryInterface(IID_IOPCCommon, (void**)&g_pIOPCCommon);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCCommon)");
		}
		else
		{
			g_pIOPCCommon->SetClientName(L"IndigoSCADA OPC AE Client");
		}

		BOOL bActive;
		DWORD dwBufferTime;
		DWORD dwMaxSize;
		DWORD hClientSubscription;
		DWORD dwRevisedBufferTime;
		DWORD dwRevisedMaxSize;

		CComCOPCEventSink   *m_pSink = NULL;
		CComCOPCShutdownRequest  *m_pShutdown = NULL;

		//ATLTRY(m_pSink = new CComCOPCEventSink);

		//if(m_pSink == NULL)
		//{
		//	ShowError(E_OUTOFMEMORY,"new CComCOPCEventSink");
		//	return 1;
		//}

		dwMaxSize = 1000; //The server can send upto 1000 events for each OnEvent call

		bActive = 1;

	    //server should check for maxsize of 0 however client should never pass it

		if(!dwMaxSize)
		{
			dwMaxSize=1;
		}

		hClientSubscription = 1243272;
		//dwBufferTime = 10000; //this is a parameter
		dwBufferTime = 0; //this is a parameter

		//sixth parametere is:
		/* Type of function called by rules to load a provider */
		//typedef LPUNKNOWN (CALLBACK * LPFNEXCHANGERULEEXTENTRY)(VOID);

		hr = g_pIOPCServer->CreateEventSubscription(bActive,
							dwBufferTime,
							dwMaxSize,
							hClientSubscription,
							//GUID_CAST(&__uuidof(m_ISubMgt)), //apa--- 17-04-2011
							IID_IOPCEventSubscriptionMgt, //apa+++ 17-04-2011
						   (IUnknown **)&m_ISubMgt,
						   &dwRevisedBufferTime,
						   &dwRevisedMaxSize);

		if(hr != S_OK)
		{
			printf("Failed to Create Subscription\n");
			return 1;
		}

		printf("A&E server dwRevisedBufferTime = %d, dwRevisedMaxSize = %d\n", dwRevisedBufferTime, dwRevisedMaxSize);

		if(m_ISubMgt == NULL)
		{
			printf("CreateEventSubscription returned m_ISubMgt NULL\n");
			return 1;
		}

		printf("CreateEventSubscription Done\n");
		
		// create advise
		CComObject<COPCEventSink>::CreateInstance(&m_pSink);
		m_dwCookie = 0xCDCDCDCD;

		IUnknown* pUnk;

		hr = m_pSink->_InternalQueryInterface( __uuidof(IUnknown), (void**)&pUnk );

		if(hr != S_OK)
		{
			printf("Failed m_pSink->_InternalQueryInterface\n");
			return 1;
		}

		hr = AtlAdvise(m_ISubMgt, pUnk, __uuidof(IOPCEventSink), &m_dwCookie );

		if(hr != S_OK)
		{
			printf("Failed AtlAdvise m_dwCookie\n");
			return 1;
		}

		//shutdown advise 
		///////////////////////////////////////////////Shutdown/////////////////////////////
/*
//NOT WORKING...
		CComObject<COPCShutdownRequest>::CreateInstance(&m_pShutdown);
		m_dwShutdownCookie = 0xCDCDCDCD;

		//IUnknown* pUnk;

		hr = m_pShutdown->_InternalQueryInterface( __uuidof(IUnknown), (void**)&pUnk );

		if(hr != S_OK)
		{
			printf("Failed m_pShutdown->_InternalQueryInterface\n");
			return 1;
		}

		//hr = AtlAdvise(m_ISubMgt, m_pShutdown->GetUnknown(),__uuidof(IOPCShutdown), &m_dwShutdownCookie);

		hr = AtlAdvise(m_ISubMgt, pUnk, __uuidof(IOPCShutdown), &m_dwShutdownCookie);
		
		if(hr != S_OK)
		{
			printf("Failed shutdown advise\n");
			return 1;
		}
*/
		////////////////////////GetStae and SetState/////////////////////////////////////////////////////////////

		hr = m_ISubMgt->GetState(&bActive,&dwBufferTime,&dwMaxSize,&hClientSubscription);

		if(hr != S_OK)
		{
			printf("Failed m_ISubMgt->GetState\n");
			return 1;
		}

		printf("Server state: bActive = %d, dwBufferTime = %d, dwMaxSize = %d, hClientSubscription = %d\n", bActive, dwBufferTime, dwMaxSize, hClientSubscription);
/*
Here on for test, is working.

		hr = m_ISubMgt->SetState(&bActive, &dwBufferTime, &dwMaxSize, hClientSubscription, &dwRevisedBufferTime, &dwRevisedMaxSize);

		if(hr != S_OK)
		{
			printf("Failed m_ISubMgt->SetState\n");
			return 1;
		}
*/
		///////////////////////Refresh//////////////////////////////////////////////////////////////

		hr = m_ISubMgt->Refresh(m_dwCookie);

		if(hr != S_OK)
		{
			printf("Failed Refresh\n");
			return 1;
		}

		///////////////////////SetKeepAlive//////////////////////////////////////////////////////////////
		
		//IOPCEventSubscriptionMgt2Ptr ISubMgt2 = m_ISubMgt;

/*
NOT WORKING...

		IOPCEventSubscriptionMgt2* ISubMgt2 = (struct IOPCEventSubscriptionMgt2*)m_ISubMgt;
		
		if(ISubMgt2 != NULL)
		{
			DWORD dwRevisedKeepAliveTime = 0;
			// set the keep-alive to 3X the dwRevisedBufferTime
			hr = ISubMgt2->SetKeepAlive(3 * dwRevisedBufferTime, &dwRevisedKeepAliveTime);

			printf("dwRevisedKeepAliveTime = %d\n", dwRevisedKeepAliveTime);
		}
*/

/*
		IOPCEventSubscriptionMgt2* ISubMgt2;

		hr = g_pIOPCServer->QueryInterface(IID_IOPCEventSubscriptionMgt2, (void**)&ISubMgt2);

		if(FAILED(hr))
		{
			printf("OPC error:Failed to obtain IID_IOPCEventSubscriptionMgt2 interface %x\n",hr);
			ShowError(hr,"Failed to obtain IID_IOPCEventSubscriptionMgt2 interface");
			return 1;
		}
*/
	}

    return(0);
}

int Opc_client_ae_DriverThread::OpcStop()
{
	IT_IT("Opc_client_ae_DriverThread::OpcStop");

	HRESULT hr;

	if(m_ISubMgt != NULL)
	{
		if(m_dwCookie != 0xCDCDCDCD)
		{
			hr = AtlUnadvise(m_ISubMgt, __uuidof(IOPCEventSink), m_dwCookie);
			m_dwCookie = 0xCDCDCDCD;
		}

		if(m_dwShutdownCookie != 0xCDCDCDCD)
		{
			hr = AtlUnadvise(m_ISubMgt, __uuidof(IOPCShutdown), m_dwShutdownCookie);
			m_dwShutdownCookie = 0xCDCDCDCD;
		}
	}

	// terminate server and it will clean up itself
	if(g_pIOPCServer) while(g_pIOPCServer->Release()) ;
	::CoUninitialize();

	printf("Server and all group interfaces terminated.\n");

	ShowMessage(S_OK,"","Server and all group interfaces terminated");
	return 1;
}

/*
void ::OnOpcDisconnect() 
{
	CWaitCursor	cWait;		//show wait cursor. 
	HRESULT hr;

	m_bConnected = FALSE;
	m_bSubscription = FALSE;
	
	if(m_ISub != NULL)
	{
		OLE_TRY(hr = AtlUnadvise( m_ISub,__uuidof(IOPCEventSink), m_dwCookie ));
		OLE_TRY(hr = AtlUnadvise( m_ISub,__uuidof(IOPCShutdown), m_dwShutdownCookie ));
		m_ISub.Attach(NULL);
	}

	m_IEventServer2.Attach(NULL);
	m_IEventServer.Attach(NULL);  //detach server
	m_ICommon.Attach(NULL);
	
	OnViewClearAll();
	
}
*/

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

	if(FAILED(hr) || (pStatus == NULL) )
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

void Opc_client_ae_DriverThread::ShowError(HRESULT hr, LPCSTR pszError)
{
	/*
	LPWSTR pwszError = NULL;

	if((g_pIOPCServer != NULL) && SUCCEEDED(g_pIOPCServer->GetErrorString(hr, 0, &pwszError)))
	{
		QString err;
		USES_CONVERSION;
		err.sprintf("Error: %s, %s", pszError, W2T(pwszError));

		postEvent(StaticParent,new DriverEvent(StaticThis, DriverEvent::OpSendAlarmString,0,err));
		
		::CoTaskMemFree(pwszError);
	}
	else
	*/
	{
		QString err;
		err.sprintf("Error: %s, %lX", pszError, hr);

		postEvent(StaticParent,new DriverEvent(StaticThis, DriverEvent::OpSendAlarmString,0,err));
	}
}

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

