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
#define MAX_ITEMS 5000  //<--PARAMETRO

// critical section stuff
CComAutoCriticalSection sincronismo; 
CComAutoCriticalSection g_Readcs;
CComAutoCriticalSection g_Writecs;

HANDLE           cb_complete_c_sc_na_1;
HANDLE           cb_complete_c_ic_na_1;

class CLock
{
	public:
	CComAutoCriticalSection* m_pcs;
	CLock(CComAutoCriticalSection* pcs) {m_pcs = pcs; pcs->Lock();}
	~CLock() {m_pcs->Unlock();}
};

#define READ_LOCK CLock gl(&g_Readcs);
#define WRITE_LOCK CLock gl(&g_Writecs);
//#define SINCRONISMO_LOCK CLock gl(&sincronismo);

class ATL_NO_VTABLE CTestAdviseSink;
class ATL_NO_VTABLE COPCCallback; 
typedef CComObject<CTestAdviseSink> CComCTestAdviseSink;
typedef CComObject<COPCCallback> CComCOPCCallback;

UINT g_nOpcFormatData = ::RegisterClipboardFormat("OPCSTMFORMATDATA");
UINT g_nOpcFormatDatatime = ::RegisterClipboardFormat("OPCSTMFORMATDATATIME");
UINT g_nOpcFormatWrite = ::RegisterClipboardFormat("OPCSTMFORMATWRITECOMPLETE");


// CTestAdviseSink class derived from IAdviseSink
// used with async updates
class ATL_NO_VTABLE CTestAdviseSink : 
	public CComObjectRoot,
	public IAdviseSink
{
public:

BEGIN_COM_MAP(CTestAdviseSink)
	COM_INTERFACE_ENTRY(IAdviseSink)
END_COM_MAP()

   STDMETHODIMP_(void) OnViewChange(DWORD, LONG) {IT_IT("CTestAdviseSink::OnViewChange"); };
   STDMETHODIMP_(void) OnRename(LPMONIKER) {IT_IT("CTestAdviseSink::OnRename"); };
   STDMETHODIMP_(void) OnSave(void) {IT_IT("CTestAdviseSink::OnSave"); };
   STDMETHODIMP_(void) OnClose(void) {IT_IT("CTestAdviseSink::OnClose"); };

   STDMETHODIMP_(void) OnDataChange(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
   {
	    IT_IT("CTestAdviseSink::OnDataChange");
   }
};

int Opc_client_ae_DriverThread::AsyncUpdate()
{
	IT_IT("Opc_client_ae_DriverThread::AsyncUpdate");

	return 0;
}

int Opc_client_ae_DriverThread::AsyncRead(bool bFlag)
{
	IT_IT("Opc_client_ae_DriverThread::AsyncRead");

	return 0;
}

// COPCCallback class derived from IOPCEventSink
// used with async updates
class ATL_NO_VTABLE COPCCallback : 
	public CComObjectRoot,
	public IOPCEventSink
{
public:

BEGIN_COM_MAP(COPCCallback)
	COM_INTERFACE_ENTRY(IOPCEventSink)
END_COM_MAP()

	STDMETHODIMP OnDataChange( 
    /* [in] */ DWORD dwTransid,
    /* [in] */ OPCHANDLE hGroup,
    /* [in] */ HRESULT hrMasterquality,
    /* [in] */ HRESULT hrMastererror,
    /* [in] */ DWORD dwCount,
    /* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
    /* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
    /* [size_is][in] */ WORD __RPC_FAR *pwQualities,
    /* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
	{

		IT_IT("COPCCallback::OnDataChange");

		//if(Opc_client_ae_DriverThread::mandare_eventi)
		{
			if(FAILED(hrMastererror))
			{
				Opc_client_ae_DriverThread::ShowError(hrMastererror,"General ConnectionPoint Update");
			}

			for(DWORD dw = 0; dw < dwCount; dw++)
			{
				IT_COMMENT2("phClientItems[%d] = %d", dw, phClientItems[dw]);
				IT_COMMENT2("pwQualities[%d] = %d", dw, pwQualities[dw]);

				//printf("phClientItems[%d] = %d\t", dw, phClientItems[dw]);
				//printf("pwQualities[%d] = %d\t", dw, pwQualities[dw]);

				if((pwQualities[dw] == OPC_QUALITY_GOOD) && SUCCEEDED(pErrors[dw]))
				{
					VARIANT *pValue = &pvValues[dw];

					//SINCRONISMO_LOCK

					const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);

					Opc_client_ae_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);
				}
				else if((pwQualities[dw] != OPC_QUALITY_GOOD) && SUCCEEDED(pErrors[dw]))
				{
					VARIANT *pValue = &pvValues[dw];

					//SINCRONISMO_LOCK

					const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);
																										
					Opc_client_ae_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);

					QString name;
					name = QString(Opc_client_ae_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				}
				else // else if
				{
					//SINCRONISMO_LOCK   //lock the other threads to enter in this critical section
					Opc_client_ae_DriverThread::SendEvent2(0, 0, pwQualities[dw], phClientItems[dw]);
					
					//const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);

					QString name;
					name = QString(Opc_client_ae_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				} // endif
				//printf("\n");
			} // end for

			//printf("\n");
		}

		return S_OK;
	}

	STDMETHODIMP OnReadComplete( 
    /* [in] */ DWORD dwTransid,
    /* [in] */ OPCHANDLE hGroup,
    /* [in] */ HRESULT hrMasterquality,
    /* [in] */ HRESULT hrMastererror,
    /* [in] */ DWORD dwCount,
    /* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
    /* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
    /* [size_is][in] */ WORD __RPC_FAR *pwQualities,
    /* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
	{
		IT_IT("COPCCallback::OnReadComplete");

		//if(Opc_client_ae_DriverThread::mandare_eventi)
		{
			if(FAILED(hrMastererror))
			{
				Opc_client_ae_DriverThread::ShowError(hrMastererror,"General Async2 Read");
			}

			if(dwTransid != Opc_client_ae_DriverThread::g_dwReadTransID)
			{
				Opc_client_ae_DriverThread::ShowError(S_OK,"Async2 Read callback, TransactionID's do not match");
				return S_FALSE;
			}

			for(DWORD dw=0; dw < dwCount; dw++)
			{
				IT_COMMENT2("phClientItems[%d] = %d", dw, phClientItems[dw]);
				IT_COMMENT2("pwQualities[%d] = %d", dw, pwQualities[dw]);

				//printf("phClientItems[%d] = %d\t", dw, phClientItems[dw]);
				//printf("pwQualities[%d] = %d\t", dw, pwQualities[dw]);

				if((pwQualities[dw] == OPC_QUALITY_GOOD) && SUCCEEDED(pErrors[dw]))
				{
					VARIANT *pValue = &pvValues[dw];

					//SINCRONISMO_LOCK

					const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);

					Opc_client_ae_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);
				}
				else if((pwQualities[dw] != OPC_QUALITY_GOOD) && SUCCEEDED(pErrors[dw]))
				{
					VARIANT *pValue = &pvValues[dw];

					//SINCRONISMO_LOCK

					const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);
																										
					Opc_client_ae_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);

					QString name;
					name = QString(Opc_client_ae_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				}
				else // else if
				{
					//SINCRONISMO_LOCK   //lock the other threads to enter in this critical section
					Opc_client_ae_DriverThread::SendEvent2(0, 0, pwQualities[dw], phClientItems[dw]);
					
					//const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);

					QString name;
					name = QString(Opc_client_ae_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_ae_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				} // endif

				//printf("\n");

			} // end for

			Opc_client_ae_DriverThread::g_bReadComplete = true;

			SetEvent(cb_complete_c_ic_na_1);

			//printf("\n");
		}

		return S_OK;
	}

	STDMETHODIMP OnCancelComplete( 
    /* [in] */ DWORD dwTransid,
    /* [in] */ OPCHANDLE hGroup)
	{
		IT_IT("COPCCallback::OnCancelComplete");
		return S_OK;
	}
};


int Opc_client_ae_DriverThread::OpcStart()
{
	IT_IT("Opc_client_ae_DriverThread::OpcStart");

	char show_msg[150];

	TCHAR  ServerIPAddress[80];
	//128.1.1.2 ---> ENLABSERVOPC
	//128.1.1.25 ---> HMI2
	strcpy(ServerIPAddress, ((Opc_client_ae_Instance*)Parent)->Cfg.OpcServerIPAddress);

	if((strlen(ServerIPAddress) == 0))
	{
		local_server = 1;
	}
	
	if(local_server)
	{
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

		// Create a running object from that class ID
		// (CLSCTX_ALL will allow in-proc, local and remote)

		hr = ::CoCreateInstance(clsid, NULL, CLSCTX_ALL, IID_ICatInformation, (void**)&g_pIOPCServer);

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
		
		/*
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
		*/

		printf("Group added, update rate = %ld.\n", dwRevisedUpdateRate);
		IT_COMMENT1("Group added, update rate = %ld.", dwRevisedUpdateRate);
		
		// Get pointer to OPC Server interfaces required for this program.
		/*
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

		printf("Try to connect to remote server: %s.\n", ServerIPAddress);
		sprintf(show_msg, "Try to connect to remote server: %s.\n", ServerIPAddress);

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

		printf("Available DA servers on remote server.\n");

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
		
		//Prosys.OPC.Simulation
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
			printf("OPC error:Failed obtain IID_IOPCEventServer interface from server, %x\n", hr);
			ShowError(hr,"Failed obtain IID_IOPCEventServer interface from server");
			return 1;
		}

		printf("Connected to server %s.\n", ServerIPAddress);

		sprintf(show_msg, "Connected to server %s.\n", ServerIPAddress);
		Opc_client_ae_DriverThread::ShowMessage(S_OK, "", show_msg);


		WORD wMajor, wMinor, wBuild;
		LPWSTR pwsz = NULL;

		if(!GetStatus(&wMajor, &wMinor, &wBuild, &pwsz))
		{
			char ver[150];
			//printf("Version: %d.%d.%d\n", wMajor, wMinor, wBuild);
			//printf("%ls\n\n",pwsz);
			sprintf(ver,"Server version: %d.%d.%d, %s", wMajor, wMinor, wBuild, W2T(pwsz));
			printf("%s\n\n",ver);

			IT_COMMENT4("Version: %d.%d.%d, %s", wMajor, wMinor, wBuild,W2T(pwsz));
			Opc_client_ae_DriverThread::ShowMessage(S_OK, "",ver);
			::CoTaskMemFree(pwsz);
		}

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

		/*
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
		*/
		
		/*
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
	if(FAILED(hr) || (pStatus == NULL) || (pStatus->dwServerState != OPCAE_STATUS_RUNNING))
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

// simple check for version OPC 2.0 type connection point containers
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


int Opc_client_ae_DriverThread::AddItems()
{
	IT_IT("Opc_client_ae_DriverThread::AddItems");

	/*
	// loop until all items are added
	char sz2[128];
	//TCHAR szBuffer[256];
	HRESULT hr = 0;
	int nTestItem = 0; // how many items there are
	IEnumString* pEnumString = NULL;
		
	USES_CONVERSION;

    hr = g_pIOPCBrowse->BrowseOPCItemIDs(OPC_FLAT, L"", VT_EMPTY, 0, &pEnumString);

	if(FAILED(hr))
	{
		ShowError(hr, _T("BrowseOPCItemIDs()"));
		return 1;
	}

    if(hr == S_OK)
    {
		LPOLESTR pszName = NULL;
        ULONG count = 0;
		char spname[30];
		char buf[256];
		ULONG nCount = 0;

		while((hr = pEnumString->Next(1, &pszName, &count)) == S_OK)
        {
			nCount++;
        }

		g_dwNumItems = nCount;

		printf(_T("OPC items: %d\n"), nCount);

		Item = (struct structItem*)malloc(g_dwNumItems*sizeof(struct structItem));

		pEnumString->Release();
	

		hr = g_pIOPCBrowse->BrowseOPCItemIDs(OPC_FLAT, L"", VT_EMPTY, 0, &pEnumString);

		if(FAILED(hr))
		{
			ShowError(hr, _T("BrowseOPCItemIDs()"));
			return 1;
		}

		QString unit = ((Opc_client_ae_Instance*)Parent)->unit_name;

		nTestItem = 0;
			
        while((hr = pEnumString->Next(1, &pszName, &count)) == S_OK)
        {
            printf(_T("%s\n"), OLE2T(pszName));

			IT_COMMENT1(_T("%s"), OLE2T(pszName));

			strcpy(buf, OLE2T(pszName));

			::CoTaskMemFree(pszName);

			wcscpy(Item[nTestItem].wszName, T2W(buf));

			///////Associo l'item, con il superior SCADA name////////////
			strcpy(buf, (const char*)unit);
			strcat(buf, "Point");
			sprintf(spname, "%s%02d", buf, nTestItem+1);
			strcpy(Item[nTestItem].spname, spname);
			////////////////////////////////////////////////////////////

			//set VT_EMPTY and the server will select the right type////
			strcpy(sz2,"VT_EMPTY");
			Item[nTestItem].vt = VT_EMPTY;
						
			OPCITEMRESULT *pItemResult = NULL;
			HRESULT *pErrors = NULL;
			OPCITEMDEF ItemDef;
			ItemDef.szAccessPath = L"";
			ItemDef.szItemID = Item[nTestItem].wszName;
			ItemDef.bActive = TRUE;
			ItemDef.hClient = g_dwClientHandle++; //parte da 1
			ItemDef.dwBlobSize = 0;
			ItemDef.pBlob = NULL;
			ItemDef.vtRequestedDataType = Item[nTestItem].vt;
			Item[nTestItem].hClient = ItemDef.hClient;

			hr = g_pIOPCItemMgt->AddItems(1, &ItemDef, &pItemResult, &pErrors);

			if(FAILED(hr))
			{
				ShowError(hr,"AddItems()");
				Sleep(20000);
				return 1;
			}

			hr = S_OK;

			if(FAILED(pErrors[0]))
			{
				ShowError(pErrors[0],"AddItem() item");
				Sleep(20000);
				return 1;
			}

			// record unique handle for this item
			Item[nTestItem].hServer = pItemResult->hServer;
			Item[nTestItem].vt = pItemResult->vtCanonicalDataType;
			Item[nTestItem].dwAccessRights = pItemResult->dwAccessRights;

			nTestItem++;
			
			::CoTaskMemFree(pItemResult);
			::CoTaskMemFree(pErrors);

			if(nTestItem >= MAX_ITEMS)
			{ 
				break;
			}
        }

		pEnumString->Release();
	}

	// Enumerate items and display
	OPCITEMATTRIBUTES *pItemAttr = NULL;

	ULONG dwFetched = 0;

	IEnumOPCItemAttributes *pEnumOPCItems = NULL;

	hr = g_pIOPCItemMgt->CreateEnumerator(IID_IEnumOPCItemAttributes,  reinterpret_cast<LPUNKNOWN*>(&pEnumOPCItems));

	if(SUCCEEDED(hr))
	{
		//printf("IOPCItemMgt::CreateEnumerator()\n");
		IT_COMMENT("IOPCItemMgt::CreateEnumerator()");

		pEnumOPCItems->Reset();

		// NOTE: 3rd param must not be a NULL or the proxy will puke
		hr = pEnumOPCItems->Next(static_cast<ULONG>(nTestItem), &pItemAttr, &dwFetched);

		if(SUCCEEDED(hr))
		{
			if((dwFetched != static_cast<ULONG>(nTestItem)) || (hr == S_FALSE))
			{
				//printf("Error: pEnumOPCItems->Next() - fetched != requested\n");
				IT_COMMENT("Error: pEnumOPCItems->Next() - fetched != requested");
			}

			for(ULONG i = 0; i < dwFetched; i++)
			{
				//printf("Item: %ls = VT_", pItemAttr[i].szItemID);
				IT_COMMENT1("Item: %ls is", pItemAttr[i].szItemID);

				switch(pItemAttr[i].vtCanonicalDataType)
				{
					case VT_I2:
					{
						//printf("I2 (short)");
						IT_COMMENT("VT_I2 (short)");
					}
					break;
					case VT_I4:
					default:
					{
						//printf("I4 (long)");
						IT_COMMENT("VT_I4 (long)");
					}
					break;
					case VT_R4:
					{
						//printf("R4 (float)");
						IT_COMMENT("VT_R4 (float)");
					}
					break;
					case VT_R8:
					{
						//printf("R8 (double)");
						IT_COMMENT("VT_R8 (double)");
					}
					break;
					case VT_BOOL:
					{
						//printf("BOOL (boolean)");
						IT_COMMENT("VT_BOOL (boolean)");
					}
					break;
					case VT_EMPTY:
					{
						//printf("EMPTY (Server Defined)");
						IT_COMMENT("VT_EMPTY (Server Defined)");
					}
					break;
				}

				DWORD ar = pItemAttr[i].dwAccessRights & OPC_READABLE;

				if(ar == OPC_READABLE)
				{
					//printf(" - Readable");
					IT_COMMENT(" - Readable");
					//Opc_client_ae_DriverThread::ShowMessage(S_OK, "Readable",(const char*)W2T(pItemAttr[i].szItemID));
				}

				ar = pItemAttr[i].dwAccessRights & OPC_WRITEABLE;

				if(ar == OPC_WRITEABLE)
				{
					//printf(" - Writable");
					IT_COMMENT(" - Writable");
					//Opc_client_ae_DriverThread::ShowMessage(S_OK, "Writable",(const char*)W2T(pItemAttr[i].szItemID));
				}
				
				//printf("\n");

				if(pItemAttr[i].szItemID)
				{
					::CoTaskMemFree(pItemAttr[i].szItemID);
				}

				if(pItemAttr[i].szAccessPath)
				{
					::CoTaskMemFree(pItemAttr[i].szAccessPath);
				}

				if(pItemAttr[i].dwBlobSize)
				{
					::CoTaskMemFree(pItemAttr[i].pBlob);
				}
			}

			// must release the memory after we are done with it
			::CoTaskMemFree(pItemAttr);
		}
		else
		{
			ShowError(hr,"pEnumOPCItems->Next()");
		}
	}
	else
	{
		ShowError(hr,"IOPCItemMgt::CreateEnumerator()");
	}

	pEnumOPCItems->Release();

	//printf("Do you wish to write values to each item (Y/N)?");

	//gets(szBuffer);

	//if((*szBuffer == _T('y')) || (*szBuffer == _T('Y')))
	//{
		g_bWriteEnable = true;
		//g_bWriteEnable = false;

	//}
	*/
	
	return(0);
}

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

