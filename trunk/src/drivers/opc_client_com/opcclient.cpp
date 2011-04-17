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
// This OPC client does sync/async reads with up to MAX_ITEMS items
//
// Critical sections are used for async calls to keep track of the
// transaction ID.  This slows the response rate down, but ensures all
// calls are completed. An alternative, the client can place transaction ID's
// into a que from OnDataChange() and after a async call is completed. Then a
// watchdog thread after a set timeout period can check both ques to see if the
// transaction completed. Or the client can simply ignore transaction ID's and
// use the client handle returned as validation.
//
// OPC version 2.0 negates the need for critical sections, since the client
// generates the transaction ID BEFORE the read/write is called.
//
////////////////////////////////////////////////////////////////////////////////
//apa - Updated opcda.h to 3.0 DA support
//TODO: AddItems: add support of arrays


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

#include "opcda.h"	/* The OPC custom interface defintions */
#include "opccomn_i.c"
#include "opccomn.h"
#include "OpcEnum.h"
#include "opcerror.h"
#include "general_defines.h"
#include "IndentedTrace.h"

#include "opc_client_com_instance.h"
#include "opc_client_comdriverthread.h"

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


//void ShowError(HRESULT hr, LPCSTR pszError)
//{
//	printf("Error: %s failed,\t->%lX\t", pszError, hr);
//}

void Opc_client_com_DriverThread::SyncRead(bool bFlag)
{
	IT_IT("Opc_client_com_DriverThread::SyncRead");
	
	OPCITEMSTATE *pItemState = NULL;
	HRESULT *pErrors = NULL;
	HRESULT hr = 0;
	// check for dupes
	int dupbool = 0;
	int dupi2 = 0;
	long dupi4 = 0;
	float dupr4 = 0.0f;
	double dupr8 = 0.0;

	if(g_bWriteEnable)
	{
		printf("Performing Sync reads/write...press a key to exit.\n");
	}
	else
	{
		printf("Performing Sync reads...press a key to exit.\n");
	}

	OPCHANDLE hServer[MAX_ITEMS];
	VARIANT Val[MAX_ITEMS];
	VARIANT vCount;

	for(DWORD dw = 0; dw < g_dwNumItems; dw++)
	{
		hServer[dw] = Item[dw].hServer;
		::VariantInit(&Val[dw]);
	}

	::VariantInit(&vCount);
	V_VT(&vCount) = VT_I2;
	V_I2(&vCount) = 0;
	HRESULT *pErrorsWrite = NULL;
	// loop around doing sync reads until user hits a key
	while(!_kbhit())
	{
		// read from the server
		hr = g_pIOPCSyncIO->Read(bFlag ? OPC_DS_CACHE : OPC_DS_DEVICE,
								 g_dwNumItems, 
								 &hServer[0], 
								 &pItemState, 
								 &pErrors);
	    if(hr == S_OK)
		{
			for(dw = 0; dw < g_dwNumItems; dw++)
			{
				switch(V_VT(&pItemState[dw].vDataValue))
				{
				case VT_BOOL:
					if(V_BOOL(&pItemState[dw].vDataValue) != dupbool)
						printf("%d\t", V_BOOL(&pItemState[dw].vDataValue));
					break;
				case VT_I2:
				default:
					if(V_I2(&pItemState[dw].vDataValue) != dupi2)
						printf("%d\t", V_I2(&pItemState[dw].vDataValue));
					break;
				case VT_I4:
					if(V_I4(&pItemState[dw].vDataValue) != dupi4)
						printf("%ld\t", V_I4(&pItemState[dw].vDataValue));
					break;
				case VT_R4:
					if(V_R4(&pItemState[dw].vDataValue) != dupr4)
						printf("%f\t", V_R4(&pItemState[dw].vDataValue));
					break;
				case VT_R8:
					if(V_R8(&pItemState[dw].vDataValue) != dupr8)
						printf("%lf\t", V_R8(&pItemState[dw].vDataValue));
					break;
				case VT_BSTR:
					printf("%ls\t", V_BSTR(&pItemState[dw].vDataValue));
					break;
				}
			}
			printf("\n");
	 
			::CoTaskMemFree(pItemState);
			::CoTaskMemFree(pErrors);
		}
		else if(hr == S_FALSE)
		{
			for(dw = 0; dw < g_dwNumItems; dw++)
			{
				if(FAILED(pErrors[dw]))
				{
					char sz[100];
					sprintf(sz,"SyncIO->Read(%ls) returned", Item[dw].wszName);
					ShowError(pErrors[dw], sz);
				}
			}
		}
		else
		{
			ShowError(hr,"Sync Read");
		}
		if(g_bWriteEnable) // quick write enable hack
		{
			// pump out data sync to items
			for(dw = 0; dw < g_dwNumItems; dw++)
			{
				V_VT(&Val[dw]) = VT_I2;
				::VariantCopy(&Val[dw], &vCount);
				::VariantChangeType(&Val[dw], &Val[dw], 0, V_VT(&Item[dw]));
			}
			V_I2(&vCount)++;
			if((V_VT(&Item[0]) == VT_BOOL) && (V_I2(&vCount) > 1))
			{
				V_I2(&vCount) = 0; // allow bool to toggle on/off
			}
			hr = g_pIOPCSyncIO->Write(g_dwNumItems, hServer, Val, &pErrorsWrite);
			if(FAILED(hr))
			{
				ShowError(hr,"SyncIO->Write()");
			}
			else if(hr == S_FALSE)
			{
				for(dw = 0; dw < g_dwNumItems; dw++)
				{
					if(FAILED(pErrorsWrite[dw]))
					{
						ShowError(pErrorsWrite[dw],"SyncIO->Write() item returned");
					}
				}
				::CoTaskMemFree(pErrorsWrite);
			}
			else // S_OK
			{
				::CoTaskMemFree(pErrorsWrite);
			}
		}
		::Sleep(g_dwUpdateRate); // sleep between updates
	}
	for(dw = 0; dw < g_dwNumItems; dw++)
	{
		::VariantClear(&Val[dw]);
	}
}

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

		if(Opc_client_com_DriverThread::mandare_eventi)
		{
			// Verify the format follows the OPC spec
			if(TYMED_HGLOBAL != pFE->tymed)
			{ 
				return;
			}

			if(pSTM->hGlobal == 0)
			{ 
				return;
			}

			if(pFE->cfFormat == g_nOpcFormatWrite)
			{
				WRITE_LOCK; //lock the other threads to enter in this critical section

				const LPBYTE pBuffer = reinterpret_cast<const LPBYTE>(::GlobalLock(pSTM->hGlobal));

				if(pBuffer == NULL)
				{ 
					return;
				}

				const OPCGROUPHEADERWRITE *pHeader = reinterpret_cast<const OPCGROUPHEADERWRITE*>(pBuffer);

				if(FAILED(pHeader->hrStatus))
				{
					Opc_client_com_DriverThread::ShowError(pHeader->hrStatus,"General Async Write");
				}

				if(Opc_client_com_DriverThread::g_dwWriteTransID != pHeader->dwTransactionID)
				{
					Opc_client_com_DriverThread::ShowError(S_OK,"Async Write callback, TransactionID's do not match");
				}

				DWORD dwSize = sizeof(OPCGROUPHEADERWRITE);

				for(DWORD dw=0; dw < pHeader->dwItemCount; dw++, dwSize += sizeof(OPCITEMHEADERWRITE))
				{
					const OPCITEMHEADERWRITE* pItemHeader = reinterpret_cast<const OPCITEMHEADERWRITE*>(&pBuffer[dwSize]);

					if(FAILED(pItemHeader->dwError))
					{
						Opc_client_com_DriverThread::ShowError(pItemHeader->dwError,"Async Write request");
					}
				}
				
				//Opc_client_com_DriverThread::ShowMessage(S_OK,"g_bWriteComplete", "Messaggio");
				//printf("Write completa");
				Opc_client_com_DriverThread::g_bWriteComplete = true;

				::GlobalUnlock(pSTM->hGlobal);

				//unlock, so let the other threads to enter in this critical section
				
				return;
			}
			else if(pFE->cfFormat != g_nOpcFormatDatatime)
			{ 
				return;
			}

			const LPBYTE pBuffer = reinterpret_cast<const LPBYTE>(::GlobalLock(pSTM->hGlobal));

			if(pBuffer == NULL)
			{ 
				return;
			}

			const OPCGROUPHEADER *pHeader = reinterpret_cast<const OPCGROUPHEADER*>(pBuffer);

			if(FAILED(pHeader->hrStatus))
			{
				Opc_client_com_DriverThread::ShowError(pHeader->hrStatus,"General Async Read");
			}

			if(Opc_client_com_DriverThread::g_bPoll)
			{
				// if we are polling, ignore async updates
				if(pHeader->dwTransactionID == 0)
				{
					return;
				}

				READ_LOCK; //lock the other threads to enter in this critical section

				if(pHeader->dwTransactionID != Opc_client_com_DriverThread::g_dwReadTransID)
				{
					Opc_client_com_DriverThread::ShowError(S_OK,"Async Read callback, TransactionID's do not match");
					
					return;
				}

				if(!(Opc_client_com_DriverThread::g_bReadComplete))
				{ 
					Opc_client_com_DriverThread::g_bReadComplete = true;
				}

			}	//unlock, so let the other threads to enter in this critical section

			DWORD dwSize = sizeof(OPCGROUPHEADER);

			IT_COMMENT1("pHeader->dwItemCount = %d", pHeader->dwItemCount);

			for(DWORD dw=0; dw < pHeader->dwItemCount; dw++, dwSize += sizeof(OPCITEMHEADER1))
			{
				const OPCITEMHEADER1* pItemHeader = reinterpret_cast<const OPCITEMHEADER1*>(&pBuffer[dwSize]);

				IT_COMMENT1("pItemHeader->hClient = %d", pItemHeader->hClient);
				IT_COMMENT1("pItemHeader->dwValueOffset = %d", pItemHeader->dwValueOffset);
				IT_COMMENT1("pItemHeader->wQuality = %d", pItemHeader->wQuality);
				IT_COMMENT1("pItemHeader->wReserved = %d", pItemHeader->wReserved);
				
				if(pItemHeader->wQuality == OPC_QUALITY_GOOD)
				{
					VARIANT *pValue = reinterpret_cast<VARIANT*>(&pBuffer[pItemHeader->dwValueOffset]);

					//SINCRONISMO_LOCK

					const FILETIME* ft = reinterpret_cast<const FILETIME *>(&(pItemHeader->ftTimeStampItem));

					Opc_client_com_DriverThread::SendEvent(pItemHeader, pValue, ft);
					
					switch(V_VT(pValue))
					{
						case VT_BOOL:
						{
							printf("%x\t", V_BOOL(pValue));
						}
						break;
						case VT_I2:
						{
							printf("%d\t", V_I2(pValue));
						}
						break;
						case VT_I4:
						{
							printf("%ld\t", V_I4(pValue));
						}
						break;
						case VT_R4:
						{
							printf("%f\t", V_R4(pValue));
						}
						break;
						case VT_R8:
						{
							printf("%lf\t", V_R8(pValue));
						}
						break;
						case VT_BSTR:
						{
							printf("%ls\t", V_BSTR(pValue));
						}
						break;
						default:
						{
							if(SUCCEEDED(::VariantChangeType(pValue,pValue,0,VT_I4)))
							{
								printf("%ld\t", V_I4(pValue));
							}
							else
							{
								printf("***\t");
							}
						}
						break;
					}
				}
				else
				{
					//SINCRONISMO_LOCK   //lock the other threads to enter in this critical section
					Opc_client_com_DriverThread::SendEvent(pItemHeader, 0, 0);
					
					//const FILETIME* ft = reinterpret_cast<const FILETIME *>(&(pItemHeader->ftTimeStampItem));

					QString name;
					name = QString(Opc_client_com_DriverThread::Item[pItemHeader->hClient].spname);
					//name = QString(Opc_client_com_DriverThread::Item[pItemHeader->hClient - 1].spname); mettere il -1 ?
					
					switch(pItemHeader->wQuality)
					{
						case OPC_QUALITY_BAD:
						default:
						{
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
						}
						break;
						case OPC_QUALITY_UNCERTAIN:
						{
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
						}
						break;
						case OPC_QUALITY_CONFIG_ERROR:
						{
							Opc_client_com_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
						}
						break;
						case OPC_QUALITY_NOT_CONNECTED:
						{
							Opc_client_com_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
						}
						break;
						case OPC_QUALITY_DEVICE_FAILURE:
						{
							Opc_client_com_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
						}
						break;
						case OPC_QUALITY_OUT_OF_SERVICE:
						{
							Opc_client_com_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
						}
						break;
					}
				}
			}
		}

		printf("\n");

		::GlobalUnlock(pSTM->hGlobal);
   }
};

int Opc_client_com_DriverThread::AsyncUpdate()
{
	IT_IT("Opc_client_com_DriverThread::AsyncUpdate");

	HRESULT hr = 0;
	FORMATETC formatetc;
	DWORD dwUpdateConnection = 0;
	DWORD dwWriteConnection = 0;
	formatetc.cfFormat = g_nOpcFormatDatatime;
	// need to fill the rest of the struct or the proxy make puke
	formatetc.ptd = NULL;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex = -1;
	formatetc.tymed = TYMED_HGLOBAL;
	CComCTestAdviseSink *pSink = NULL;

	ATLTRY(pSink = new CComCTestAdviseSink);
	if(pSink == NULL)
	{
		ShowError(E_OUTOFMEMORY,"new CTestAdviseSink");
		return 1;
	}

	hr = g_pIDataObject->DAdvise(&formatetc, 0, pSink, &dwUpdateConnection);
	if(FAILED(hr))
	{
		ShowError(hr,"DAdvise(Datatime)");
		return 1;
	}
	if(g_bWriteEnable)
	{
		formatetc.cfFormat = g_nOpcFormatWrite;
		hr = g_pIDataObject->DAdvise(&formatetc, 0, pSink, &dwWriteConnection);
		if(FAILED(hr))
		{
			ShowError(hr,"DAdvise(Write)");
			return 1;
		}
		printf("Performing Async updates/write...press a key to exit.\n");
	}
	else
		printf("Performing Async updates...press a key to exit.\n");

	OPCHANDLE hServer[MAX_ITEMS];
	VARIANT Val[MAX_ITEMS];
	VARIANT vCount;
	DWORD dw = 0;
	if(g_bWriteEnable)
	{
		for(dw = 0; dw < g_dwNumItems; dw++)
		{
			hServer[dw] = Item[dw].hServer;
			::VariantInit(&Val[dw]);
		}
	}
	::VariantInit(&vCount);
	V_VT(&vCount) = VT_I2;
	V_I2(&vCount) = 0;
	HRESULT *pErrorsWrite = NULL;
	// nap while server does its callback
	while(!_kbhit())
	{
		::Sleep(10);
		if(g_bWriteEnable && Opc_client_com_DriverThread::g_bWriteComplete)
		{
			// pump out data async to items
			for(dw = 0; dw < g_dwNumItems; dw++)
			{
				V_VT(&Val[dw]) = VT_I2;
				::VariantCopy(&Val[dw], &vCount);
				::VariantChangeType(&Val[dw], &Val[dw], 0, V_VT(&Item[dw]));
			}
			V_I2(&vCount)++;
			if((V_VT(&Item[0]) == VT_BOOL) && (V_I2(&vCount) > 1))
			{
				V_I2(&vCount) = 0; // allow bool to toggle on/off
			}
			Opc_client_com_DriverThread::g_bWriteComplete = false;
			g_Writecs.Lock(); // lock callbacks until we get transid
			hr = g_pIOPCAsyncIO->Write(dwWriteConnection, g_dwNumItems, hServer, Val, &g_dwWriteTransID, &pErrorsWrite);
			g_Writecs.Unlock();
			if(FAILED(hr))
			{
				ShowError(hr,"AsyncIO->Write()");
			}
			else if(hr == S_FALSE)
			{
				for(dw = 0; dw < g_dwNumItems; dw++)
				{
					if(FAILED(pErrorsWrite[dw]))
					{
						ShowError(pErrorsWrite[dw],"AsyncIO->Write() item returned");
					}
				}
				::CoTaskMemFree(pErrorsWrite);
			}
			else // S_OK
			{
				::CoTaskMemFree(pErrorsWrite);
			}
		}
	}
	if(g_bWriteEnable)
	{
		for(dw = 0; dw < g_dwNumItems; dw++)
		{
			::VariantClear(&Val[dw]);
		}
	}
	::VariantClear(&vCount);

	hr = g_pIDataObject->DUnadvise(dwUpdateConnection);
	if(FAILED(hr))
	{
		ShowError(hr,"DUnadvise(Datatime)");
	}
	if(g_bWriteEnable)
	{
		hr = g_pIDataObject->DUnadvise(dwWriteConnection);
		if(FAILED(hr))
		{
			ShowError(hr,"DUnadvise(Write)");
		}
	}

	return 0;
}

int Opc_client_com_DriverThread::AsyncRead(bool bFlag)
{
	IT_IT("Opc_client_com_DriverThread::AsyncRead");

	HRESULT hr = 0;
	FORMATETC formatetc;
	DWORD dwReadConnection = 0;
	DWORD dwWriteConnection = 0;
	//g_bPoll = true; // we are polling for values
	g_bPoll = false;
	formatetc.cfFormat = g_nOpcFormatDatatime;
	// need to fill the rest of the struct or the proxy make puke
	formatetc.ptd = NULL;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex = -1;
	formatetc.tymed = TYMED_HGLOBAL;
	CTestAdviseSink *pSink = NULL;
	//bool end = false;

	ATLTRY(pSink = new CComCTestAdviseSink);

	if(pSink == NULL)
	{
		ShowError(E_OUTOFMEMORY,"new CTestAdviseSink");
		return 1;
	}

	hr = g_pIDataObject->DAdvise(&formatetc, 0, pSink, &dwReadConnection);

	if(FAILED(hr))
	{
		ShowError(hr,"DAdvise(Datatime)");
		return 1;
	}

	if(g_bWriteEnable)
	{
		formatetc.cfFormat = g_nOpcFormatWrite;
		
		hr = g_pIDataObject->DAdvise(&formatetc, 0, pSink, &dwWriteConnection);
		if(FAILED(hr))
		{
			ShowError(hr,"DAdvise(Write)");
			return 1;
		}
		
		//printf("Performing Async reads/write...press a key to exit.\n");
	}
	else
	{
		//printf("Performing Async reads...press a key to exit.\n");
	}

	//OPCHANDLE hServer[MAX_ITEMS];
	VARIANT Val[MAX_ITEMS];
	#ifdef PROVA_SCRITTURA
	VARIANT vCount;
	#endif
	VARIANT vCommandValue;
	DWORD dw = 0;
	
	if(g_bWriteEnable)
	{
		for(dw = 0; dw < g_dwNumItems; dw++)
		{
			//DWORD v = Item[dw].dwAccessRights && OPC_WRITEABLE;
			//if (v == OPC_WRITEABLE)
			//{
			//	hServer[dw] = Item[dw].hServer;
			//	::VariantInit(&Val[dw]);
			//}
			//else
			//{
			//	hServer[dw] = Item[dw].hServer;
			//	::VariantInit(&Val[dw]);
			//}

			//hServer[dw] = Item[dw].hServer;
			::VariantInit(&Val[dw]);

		}
	}
	
	#ifdef PROVA_SCRITTURA
	::VariantInit(&vCount);
	V_VT(&vCount) = VT_I2;
	V_I2(&vCount) = 0;
	#endif

	::VariantInit(&vCommandValue);
	V_VT(&vCommandValue) = VT_BOOL;
	//V_BOOL(&vCommandValue) = commandValue;

	HRESULT *pErrorsWrite = NULL;

	// nap while server does its callback

	while(true)
	{
		//IT_COMMENT("Opc_client_com_DriverThread Waiting....");
		
		//{
		//	dbCriticalSection cs(mutex); //enter critical section
		//	m_hevtSinc.wait(mutex);
		//	m_hevtSinc.reset();
		//}//exit critical section

		if(fExit)
		{
			mandare_eventi = false;
			IT_COMMENT("Opc_client_com_DriverThread exiting....");
			m_hevtEnd.signal();
			break; //terminate the thread
		}
		
		//IT_COMMENT("Opc_client_com_DriverThread Past Wait Flag");

		//if(!SendRece(*pinternal_packet))
		//{
		//	IT_COMMENT("SendRece failed");
		//	fFail = true;
		//}


		if(g_bWriteEnable && Opc_client_com_DriverThread::g_bWriteComplete)
		{
			//pump out data async to items
			for(dw = 0; dw < g_dwNumItems; dw++)
			{
				V_VT(&Val[dw]) = VT_I2;
				//::VariantCopy(&Val[dw], &vCommandValue);
				//::VariantChangeType(&Val[dw], &Val[dw], 0, V_VT(&Item[dw]));
			}

			//V_I2(&vCount)++;

			//if((V_VT(&Item[0]) == VT_BOOL) && (V_I2(&vCount) > 1))
			//{
			//	V_I2(&vCount) = 0; // allow bool to toggle on/off
			//}
	

			// write to one item at a time
			for(dw = 0; dw < g_dwNumItems; dw++)
			{
				const char *a = Item[dw].spname;
				
				//USES_CONVERSION;

				//if(strcmp(W2T(Item[dw].wszName), "Simulated Card.Simulated Node.STATE") == 0)
				//	break;
			
				{

					::VariantCopy(&Val[dw], &vCommandValue);
					::VariantChangeType(&Val[dw], &Val[dw], 0, V_VT(&Item[dw]));

					#ifdef PROVA_SCRITTURA
					///////////////////////////////////INIZIO PROVA//////////////////////////////////////////

					V_I2(&vCount)++;

					if((V_VT(&Item[0]) == VT_BOOL) && (V_I2(&vCount) > 1))
					{
						V_I2(&vCount) = 0; // allow bool to toggle on/off
					}

					::VariantCopy(&Val[dw], &vCount);
					::VariantChangeType(&Val[dw], &Val[dw], 0, V_VT(&Item[dw]));

					//////////////////////////////fine prova//////////////////////////////////////////////////
					#endif

					//IT_COMMENT2("Command for sample point %s, value: %lf", Item[dw].spname, commandValue);
					//DWOERD index = Item[dw].hClient;
					DWORD dwAccessRights = Item[dw].dwAccessRights;

					dwAccessRights = dwAccessRights & OPC_WRITEABLE;

					if(dwAccessRights == OPC_WRITEABLE)
					{
						Opc_client_com_DriverThread::g_bWriteComplete = false;
						
						g_Writecs.Lock(); // lock callbacks until we get transid

						OPCHANDLE serversItemHandle = Item[dw].hServer;
						
						hr = g_pIOPCAsyncIO->Write(dwWriteConnection, 1,  &serversItemHandle, &Val[dw], &g_dwWriteTransID, &pErrorsWrite);

						g_Writecs.Unlock();

						IT_COMMENT1("Write executed for sample point %s", Item[dw].spname);

						if(FAILED(hr))
						{
							ShowError(hr,"AsyncIO->Write()");
						}
						else if(hr == S_FALSE)
						{
							for(dw = 0; dw < g_dwNumItems; dw++)
							{
								if(FAILED(pErrorsWrite[dw]))
								{
									ShowError(pErrorsWrite[dw],"AsyncIO->Write() item returned");
								}
							}

							::CoTaskMemFree(pErrorsWrite);
						}
						else // S_OK
						{
							::CoTaskMemFree(pErrorsWrite);
						}
					}
					else
					{
						IT_COMMENT1("No access write for sample point %s", Item[dw].spname);
					}

				}
			}
		}

		if(g_bReadComplete)
		{
			g_bReadComplete	= false;
			g_Readcs.Lock(); // lock callbacks until we get transid
			// read all items in group
			hr = g_pIOPCAsyncIO->Refresh(dwReadConnection,
										 bFlag ? OPC_DS_CACHE : OPC_DS_DEVICE, 
										 &g_dwReadTransID);
			g_Readcs.Unlock();
			if(FAILED(hr))
			{
				ShowError(hr,"AsyncIO->Refresh()");
			}
		}

		::Sleep(g_dwUpdateRate);
	}

	if(g_bWriteEnable)
	{
		for(dw = 0; dw < g_dwNumItems; dw++)
		{
			::VariantClear(&Val[dw]);
		}
	}

	//::VariantClear(&vCount);
	::VariantClear(&vCommandValue);

	hr = g_pIDataObject->DUnadvise(dwReadConnection);

	if(FAILED(hr))
	{
		ShowError(hr,"DUnadvise(Datatime)");
	}

	if(g_bWriteEnable)
	{
		hr = g_pIDataObject->DUnadvise(dwWriteConnection);

		if(FAILED(hr))
		{
			ShowError(hr,"DUnadvise(Write)");
		}
	}

	return 0;
}

// COPCCallback class derived from IOPCDataCallback
// used with async updates
class ATL_NO_VTABLE COPCCallback : 
	public CComObjectRoot,
	public IOPCDataCallback
{
public:

BEGIN_COM_MAP(COPCCallback)
	COM_INTERFACE_ENTRY(IOPCDataCallback)
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

		//if(Opc_client_com_DriverThread::mandare_eventi)
		{
			if(FAILED(hrMastererror))
			{
				Opc_client_com_DriverThread::ShowError(hrMastererror,"General ConnectionPoint Update");
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

					Opc_client_com_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);
				}
				else if((pwQualities[dw] != OPC_QUALITY_GOOD) && SUCCEEDED(pErrors[dw]))
				{
					VARIANT *pValue = &pvValues[dw];

					//SINCRONISMO_LOCK

					const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);
																										
					Opc_client_com_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);

					QString name;
					name = QString(Opc_client_com_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				}
				else // else if
				{
					//SINCRONISMO_LOCK   //lock the other threads to enter in this critical section
					Opc_client_com_DriverThread::SendEvent2(0, 0, pwQualities[dw], phClientItems[dw]);
					
					//const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);

					QString name;
					name = QString(Opc_client_com_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
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

		//if(Opc_client_com_DriverThread::mandare_eventi)
		{
			if(FAILED(hrMastererror))
			{
				Opc_client_com_DriverThread::ShowError(hrMastererror,"General Async2 Read");
			}

			if(dwTransid != Opc_client_com_DriverThread::g_dwReadTransID)
			{
				Opc_client_com_DriverThread::ShowError(S_OK,"Async2 Read callback, TransactionID's do not match");
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

					Opc_client_com_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);
				}
				else if((pwQualities[dw] != OPC_QUALITY_GOOD) && SUCCEEDED(pErrors[dw]))
				{
					VARIANT *pValue = &pvValues[dw];

					//SINCRONISMO_LOCK

					const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);
																										
					Opc_client_com_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);

					QString name;
					name = QString(Opc_client_com_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				}
				else // else if
				{
					//SINCRONISMO_LOCK   //lock the other threads to enter in this critical section
					Opc_client_com_DriverThread::SendEvent2(0, 0, pwQualities[dw], phClientItems[dw]);
					
					//const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);

					QString name;
					name = QString(Opc_client_com_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_com_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				} // endif

				//printf("\n");

			} // end for

			Opc_client_com_DriverThread::g_bReadComplete = true;

			SetEvent(cb_complete_c_ic_na_1);

			//printf("\n");
		}

		return S_OK;
	}

	STDMETHODIMP OnWriteComplete( 
    /* [in] */ DWORD dwTransid,
    /* [in] */ OPCHANDLE hGroup,
    /* [in] */ HRESULT hrMastererr,
    /* [in] */ DWORD dwCount,
    /* [size_is][in] */ OPCHANDLE __RPC_FAR *pClienthandles,
    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
	{
		IT_IT("COPCCallback::OnWriteComplete");
		
		if(FAILED(hrMastererr))
		{
			Opc_client_com_DriverThread::ShowError(hrMastererr,"General Async2 Write");

			//Send error detected in execution of command
		}

		//fprintf(stderr, "g_dwWriteTransID = %d, dwTransid = %d\n", Opc_client_com_DriverThread::g_dwWriteTransID, dwTransid);
		//fflush(stderr);

		if(Opc_client_com_DriverThread::g_dwWriteTransID != dwTransid)
		{
			Opc_client_com_DriverThread::ShowError(S_OK,"Async2 Write callback, TransactionID's do not match");
		}

		for(DWORD dw=0; dw < dwCount; dw++)
		{
			if(FAILED(pErrors[dw]))
			{
				Opc_client_com_DriverThread::ShowError(pErrors[dw], "Async2 Write request");
			}

			switch(pErrors[dw])
			{
				case S_OK:
					//The data item was written!
					//Send message to parent that command WAS executed!
				break;
				case OPC_E_BADRIGHTS:
					//NOT executed, no permission of write
				break;
				case OPC_E_INVALIDHANDLE:
					//NOT executed, the passed itme handle was invalid
				break;
				case OPC_E_UNKNOWNITEMID:
					//NOT executed, the passed itme handle was invalid
				break;
				default:
					//S_XXX
					//The data item WAS written but there is a vendor 
					//specific warning (for example the value was clamped)
					//Send message to parent that command WAS executed!

					//E_XXX
					//The data item was NOT written and there is a vendor 
					//specific error which provide more information 
					//(for example the device is offline). 
				break;
			}
		}

		Opc_client_com_DriverThread::g_bWriteComplete = true;

		SetEvent(cb_complete_c_sc_na_1);

		//Client must always return S_OK in this function
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

int Opc_client_com_DriverThread::Async2Update()
{
	IT_IT("Opc_client_com_DriverThread::Async2Update");

	if(g_pIOPCAsyncIO2 == NULL) return 1; // not supported

	IConnectionPointContainer *pCPC = NULL;
	IConnectionPoint *pCP = NULL;
	DWORD dwCookie = 0;
	HRESULT hr = S_OK;

	cb_complete_c_sc_na_1 =
	CreateEvent(NULL,    /* security */
              TRUE,   /* bManualReset */
              TRUE,   /* bInitialState */
              NULL);    /* name */

	cb_complete_c_ic_na_1 =
	CreateEvent(NULL,    /* security */
              TRUE,   /* bManualReset */
              TRUE,   /* bInitialState */
              NULL);    /* name */

	// create the sink
	CComCOPCCallback *pSink = NULL;

	ATLTRY(pSink = new CComCOPCCallback);

	if(pSink == NULL)
	{
		ShowError(E_OUTOFMEMORY,"new COPCCallback");
		return 1;
	}

	// obtain connection points
	hr = g_pIGroupUnknown->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);

	if(FAILED(hr))
	{
		ShowError(hr, "QueryInterface(IID_IConnectionPointContainer)");
		return 1;
	}

	hr = pCPC->FindConnectionPoint(IID_IOPCDataCallback, &pCP);

	if(FAILED(hr))
	{
		ShowError(hr, "FindConnectionPoint(IID_IOPCDataCallback)");
		return 1;
	}

	hr = pCP->Advise(pSink, &dwCookie);

	if(FAILED(hr))
	{
		ShowError(hr, "Advise()");
		return 1;
	}

	OPCHANDLE hServer[MAX_ITEMS];
	VARIANT Val[MAX_ITEMS];
	
	DWORD dw = 0;
	DWORD nWriteItems = 1;
	VARIANT vCommandValue;
	DWORD id_of_ItemToWrite = 0;
	HRESULT *pErrorsWrite = NULL;

	::VariantInit(&vCommandValue);

	if(g_bWriteEnable)
	{
		for(dw = 0; dw < nWriteItems; dw++)
		{
			id_of_ItemToWrite = dw;
			::VariantInit(&Val[dw]);
		}
	}

	/////////////////////begin remove ASAP//////////////////////////
	//Lasciare qui solo per sviluppo GENERAL INTERROGATION
	//g_pIOPCAsyncIO2->SetEnable(FALSE); // turn off update callbacks 
	////////////////////////end remove////////////////////////////////////

	while(true)
	{
		if(fExit)
		{
			mandare_eventi = false;
			IT_COMMENT("Opc_client_com_DriverThread exiting....");
			m_hevtEnd.signal();
			break; //terminate the thread
		}

		//fprintf(stderr, "Opc_client_com_DriverThread::g_bWriteComplete = %d\n", Opc_client_com_DriverThread::g_bWriteComplete);
		//fflush(stderr);
		
		
		{
			// receive command
			struct iec_item queued_item;
			int n, len;
			const unsigned wait_limit_ms = 1;
			char buf[sizeof(struct iec_item)];
			struct iec_item* p_item;
			u_int message_checksum, msg_checksum;
			
			for(n = 0; (len = fifo_get(Opc_client_com_DriverThread::fifo_control_direction, buf, sizeof(struct iec_item), wait_limit_ms)) >= 0; n += 1)
			{ 
				p_item = (struct iec_item*)buf;

				//printf("Receiving %d th message \n", p_item->msg_id);
				IT_COMMENT1("Receiving %d th message \n", p_item->msg_id);
				
				//////calculate checksum with checksum byte set to value zero//////////////////////////////////////
				msg_checksum = p_item->checksum;

				p_item->checksum = 0; //azzero

				message_checksum = 0;
				for (int k = 0; k < len; k++) 
				{ 
					message_checksum = message_checksum + buf[k];
				}
				message_checksum = message_checksum%256;

				if(message_checksum != msg_checksum)
				{
					printf("Cheksum error\n");
					continue;
				}
				//////////////////end checksum////////////////////////////////////////

				memcpy(&queued_item, buf, sizeof(struct iec_item));

				/////////////////////write command///////////////////////////////////////////////////////////
				#ifdef COMMAND_WITH_TIME
				if(queued_item.iec_type == C_SC_TB_1)
				#else //COMMAND_WITH_TIME
				if(queued_item.iec_type == C_SC_NA_1)
				#endif
				{
					DWORD timeout = 5000; //5s
					int rc = WaitForSingleObject(cb_complete_c_sc_na_1, timeout);

					if(rc != WAIT_OBJECT_0) 
					{
						if(rc == WAIT_TIMEOUT)
						{
							//11-04-10 TODO: gestire questo errore
							//probably the have lost the connection with server
							//so exit main loop
							//fExit = 1;

							char err[150];
							sprintf(err, "Command timeout");
							ShowError(rc, err);
						}
						else
						{
							char err[150];
							sprintf(err, "fExit = 1; at line %d ", __LINE__);
							ShowError(S_OK, err);
							//TODO: gestire meglio questo
							fExit = 1;
							//GetLastError();
						}
					}

					ResetEvent(cb_complete_c_sc_na_1);

					//Receive a write command
					if(g_bWriteEnable && Opc_client_com_DriverThread::g_bWriteComplete)
					{
						//printf("Receiving command for hClient %d\n", queued_item.hClient);

						#ifdef COMMAND_WITH_TIME
						//Check the life time of the command
						//If life time > 10 seconds => DO NOT execute the command
						//C_SC_TB_1 time contains the UTC time
						u_short command_life_time_in_seconds = queued_item.iec_obj.o.type52.time.msec/1000;

						struct cp56time2a actual_time;
						get_utc_host_time(&actual_time);

						u_short delta = actual_time.msec/1000 - command_life_time_in_seconds;

						if(delta < 10)
						#endif //COMMAND_WITH_TIME
						{
							hServer[id_of_ItemToWrite] = Item[queued_item.hClient - 1].hServer; //<--l'handle del server identifica l'item da scrivere

							switch(V_VT(&Item[queued_item.hClient - 1]))
							{
								case VT_BSTR:
								{
									USES_CONVERSION;

									V_VT(&vCommandValue) = VT_BSTR;

									V_BSTR(&vCommandValue) = SysAllocString(T2COLE(queued_item.command_string));
									
									if (FAILED(::VariantCopy(&Val[id_of_ItemToWrite], &vCommandValue)))
									{
										printf("Error %d, %d",__LINE__, __FILE__);
										//send_negative_termination_to_iec_104slave(&queued_item);
										goto write_error;
									}

									printf("Command for sample point %s, value: %s\n", Item[queued_item.hClient - 1].spname, OLE2T(V_BSTR(&vCommandValue)));

									IT_COMMENT2("Command for sample point %s, value: %s\n", Item[queued_item.hClient - 1].spname, OLE2T(V_BSTR(&vCommandValue)));

									SysFreeString(V_BSTR(&vCommandValue));
								}
								break;
								default:
								{
									V_VT(&vCommandValue) = VT_R4;

									//double cmd_val = atof(queued_item.command_string);

									double cmd_val = queued_item.commandValue;
									
									V_R4(&vCommandValue) = (float)cmd_val;
									
									if (FAILED(::VariantCopy(&Val[id_of_ItemToWrite], &vCommandValue)))
									{
										printf("Error %d, %d\n",__LINE__, __FILE__);
										//send_negative_termination_to_iec_104slave(&queued_item);
										goto write_error;
									}

									printf("Command for sample point %s, value: %lf\n", Item[queued_item.hClient - 1].spname, cmd_val);

									//IT_COMMENT2("Command for sample point %s, value: %lf", Item[queued_item.hClient - 1].spname, cmd_val);
								}
								break;
							}

							if (FAILED(::VariantChangeType(&Val[id_of_ItemToWrite], &Val[id_of_ItemToWrite], 0, V_VT(&Item[queued_item.hClient - 1]))))
							{
								printf("Error %d, %d\n",__LINE__, __FILE__);
								//send_negative_termination_to_iec_104slave(&queued_item);
								goto write_error;
							}
											
							DWORD dwAccessRights = Item[queued_item.hClient - 1].dwAccessRights;

							dwAccessRights = dwAccessRights & OPC_WRITEABLE;

							if(dwAccessRights == OPC_WRITEABLE)
							{
								Opc_client_com_DriverThread::g_bWriteComplete = false;

								//fprintf(stderr, "Opc_client_com_DriverThread::g_bWriteComplete = %d\n", Opc_client_com_DriverThread::g_bWriteComplete);
								//fflush(stderr);

								//fprintf(stderr, "g_dwWriteTransID = %d\n", Opc_client_com_DriverThread::g_dwWriteTransID);
								//fflush(stderr);

								hr = g_pIOPCAsyncIO2->Write(nWriteItems, hServer, Val, ++g_dwWriteTransID, &g_dwCancelID, &pErrorsWrite);

								if(FAILED(hr))
								{
									ShowError(hr,"AsyncIO2->Write()");
									//send_negative_termination_to_iec_104slave(&queued_item);
									goto write_error;
								}
								else if(hr == S_FALSE)
								{
									for(dw = 0; dw < nWriteItems; dw++)
									{
										if(FAILED(pErrorsWrite[dw]))
										{
											ShowError(pErrorsWrite[dw],"AsyncIO2->Write() item returned");
											//send_negative_termination_to_iec_104slave(&queued_item);
											goto write_error;
										}
									}
									::CoTaskMemFree(pErrorsWrite);
								}
								else // S_OK
								{
									::CoTaskMemFree(pErrorsWrite);
								}

								if(V_VT(&Val[id_of_ItemToWrite]) == VT_BSTR)
								{
									SysFreeString(V_BSTR(&Val[id_of_ItemToWrite]));
								}
							}
							else
							{
								IT_COMMENT1("No access write for sample point %s", Item[queued_item.hClient - 1].spname);
								printf("No access write for sample point %s\n", Item[queued_item.hClient - 1].spname);
								Opc_client_com_DriverThread::ShowError(0,"No access write for sample point");
								//send_negative_termination_to_iec_104slave(&queued_item);
								goto write_error;
							}
						}
						#ifdef COMMAND_WITH_TIME
						else
						{
							IT_COMMENT1("Scartato comando for sample point %s, aged > 10 s\n", Item[queued_item.hClient - 1].spname);
							printf("Scartato comando for sample point %s, aged > 10 s\n", Item[queued_item.hClient - 1].spname);
							//send_negative_termination_to_iec_104slave(&queued_item);
							goto write_error;
						}
						#endif
					}
					else
					{
						fprintf(stderr, "Pacchetto perso!\n");
						fflush(stderr);
						//assert(0);
					}
				}
				else if(queued_item.iec_type == C_EX_IT_1)
				{
					//Receiving EXIT process command from SLAVE
					//exit the thread, and stop the process
					fExit = true;
				}
				else if(queued_item.iec_type == C_IC_NA_1)
				{
					//Receiving general interrogation command
					IT_COMMENT("Receiving general interrogation command");
					printf("Receiving general interrogation command\n");
					
					OPCHANDLE hServerRead[MAX_ITEMS];
					HRESULT *pErrorsRead = NULL;
					
					for(DWORD dw = 0; dw < g_dwNumItems; dw++)
					{
						hServerRead[dw] = Item[dw].hServer;
					}

					DWORD timeout = 20000; //waits 20 s to finisch "general interrogation" of OPC server
					int rc = WaitForSingleObject(cb_complete_c_ic_na_1, timeout);

					if(rc != WAIT_OBJECT_0) 
					{
						if(rc == WAIT_TIMEOUT)
						{
							//11-04-10 TODO: gestire questo errore
							//probably the have lost the connection with server
							//so exit main loop
							//fExit = 1;

							char err[150];
							sprintf(err, "Command timeout");
							ShowError(rc, err);
						}
						else
						{
							char err[150];
							sprintf(err, "fExit = 1; at line %d ", __LINE__);
							ShowError(S_OK, err);

							//TODO: gestire meglio questo
							fExit = 1;
							//GetLastError();
						}
					}

					ResetEvent(cb_complete_c_ic_na_1);
					
					if(g_bReadComplete)
					{
						g_bReadComplete	= false;

						// read all items in group

						hr = g_pIOPCAsyncIO2->Read(g_dwNumItems, hServerRead, ++g_dwReadTransID, &g_dwCancelID, &pErrorsRead);

						if(FAILED(hr))
						{
							ShowError(hr,"AsyncIO2->Read()");
						}
						else if(hr == S_FALSE)
						{
							for(dw = 0; dw < g_dwNumItems; dw++)
							{
								if(FAILED(pErrorsRead[dw]))
								{
									ShowError(pErrorsRead[dw],"AsyncIO2->Read() item returned");
								}
							}
							::CoTaskMemFree(pErrorsRead);
						}
						else // S_OK
						{
							::CoTaskMemFree(pErrorsRead);
						}
					}
					else
					{
						//general interrogation persa
						//assert(0);
						printf("General interrogation persa!");
					}

					/////////end General interrogation command
				}
			}
		}

		write_error:

		::Sleep(g_dwUpdateRate);
	}

	if(g_bWriteEnable)
	{
		for(dw = 0; dw < nWriteItems; dw++)
		{
			::VariantClear(&Val[dw]);
		}
	}
	
	::VariantClear(&vCommandValue);
	
	// release interfaces
	hr = pCP->Unadvise(dwCookie);

	if(FAILED(hr))
	{
		ShowError(hr, "Unadvise()");
	}

	pCP->Release();
	pCPC->Release();
	return 0;
}

int Opc_client_com_DriverThread::Async2Read(bool bFlag)
{
	IT_IT("Opc_client_com_DriverThread::Async2Read");

	if(g_pIOPCAsyncIO2 == NULL) return 1; // not supported

	IConnectionPointContainer *pCPC = NULL;
	IConnectionPoint *pCP = NULL;
	DWORD dwCookie = 0; // advise cookie
	HRESULT hr = S_OK;

	// create the sink
	CComCOPCCallback *pSink = NULL;

	ATLTRY(pSink = new CComCOPCCallback);

	if(pSink == NULL)
	{
		ShowError(E_OUTOFMEMORY,"new COPCCallback");
		return 1;
	}

	// obtain connection points
	hr = g_pIGroupUnknown->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);

	if(FAILED(hr))
	{
		ShowError(hr, "QueryInterface(IID_IConnectionPointContainer)");
		return 1;
	}

	hr = pCPC->FindConnectionPoint(IID_IOPCDataCallback, &pCP);

	if(FAILED(hr))
	{
		ShowError(hr, "FindConnectionPoint(IID_IOPCDataCallback)");
		return 1;
	}

	hr = pCP->Advise(pSink, &dwCookie);

	if(FAILED(hr))
	{
		ShowError(hr, "Advise()");
		return 1;
	}

	g_pIOPCAsyncIO2->SetEnable(FALSE); // turn off update callbacks

	if(g_bWriteEnable)
	{
		//printf("Performing Async2 reads/writes...press a key to exit.\n");
	}
	else
	{
		//printf("Performing Async2 reads...press a key to exit.\n");
	}

	OPCHANDLE hServer[MAX_ITEMS];
	VARIANT Val[MAX_ITEMS];
	VARIANT vCount;

	for(DWORD dw = 0; dw < g_dwNumItems; dw++)
	{
		hServer[dw] = Item[dw].hServer;
		::VariantInit(&Val[dw]);
	}

	::VariantInit(&vCount);

	V_VT(&vCount) = VT_I2;

	V_I2(&vCount) = 0;

	HRESULT *pErrorsWrite = NULL;
	HRESULT *pErrorsRead = NULL;
	// nap while server does its callback

	while(true)
	{
		if(fExit)
		{
			mandare_eventi = false;
			IT_COMMENT("Opc_client_com_DriverThread exiting....");
			m_hevtEnd.signal();
			break; //terminate the thread
		}
		
		if(g_bWriteEnable && Opc_client_com_DriverThread::g_bWriteComplete)
		{
			// pump out data async to items

			for(dw = 0; dw < g_dwNumItems; dw++)
			{
				V_VT(&Val[dw]) = VT_I2;
				::VariantCopy(&Val[dw], &vCount);
				::VariantChangeType(&Val[dw], &Val[dw], 0, V_VT(&Item[dw]));
			}

			V_I2(&vCount)++;

			if((V_VT(&Item[0]) == VT_BOOL) && (V_I2(&vCount) > 1))
			{
				V_I2(&vCount) = 0; // allow bool to toggle on/off
			}

			Opc_client_com_DriverThread::g_bWriteComplete = false;

			// write items

			hr = g_pIOPCAsyncIO2->Write(g_dwNumItems, hServer, Val, ++g_dwWriteTransID, &g_dwCancelID, &pErrorsWrite);

			if(FAILED(hr))
			{
				ShowError(hr,"AsyncIO2->Write()");
			}
			else if(hr == S_FALSE)
			{
				for(dw = 0; dw < g_dwNumItems; dw++)
				{
					if(FAILED(pErrorsWrite[dw]))
					{
						ShowError(pErrorsWrite[dw],"AsyncIO2->Write() item returned");
					}
				}
				::CoTaskMemFree(pErrorsWrite);
			}
			else // S_OK
			{
				::CoTaskMemFree(pErrorsWrite);
			}
		}

		if(g_bReadComplete)
		{
			g_bReadComplete	= false;

			// read all items in group

			hr = g_pIOPCAsyncIO2->Read(g_dwNumItems, hServer, ++g_dwReadTransID, &g_dwCancelID, &pErrorsRead);

			if(FAILED(hr))
			{
				ShowError(hr,"AsyncIO2->Read()");
			}
			else if(hr == S_FALSE)
			{
				for(dw = 0; dw < g_dwNumItems; dw++)
				{
					if(FAILED(pErrorsRead[dw]))
					{
						ShowError(pErrorsRead[dw],"AsyncIO2->Read() item returned");
					}
				}
				::CoTaskMemFree(pErrorsRead);
			}
			else // S_OK
			{
				::CoTaskMemFree(pErrorsRead);
			}
		}

		::Sleep(g_dwUpdateRate);
	}

	for(dw = 0; dw < g_dwNumItems; dw++)
	{
		::VariantClear(&Val[dw]);
	}

	::VariantClear(&vCount);

	// release interfaces

	hr = pCP->Unadvise(dwCookie);

	if(FAILED(hr))
	{
		ShowError(hr, "Unadvise()");
	}

	pCP->Release();
	pCPC->Release();
	return 0;
}

int Opc_client_com_DriverThread::OpcStart()
{
	IT_IT("Opc_client_com_DriverThread::OpcStart");

	char show_msg[150];

	TCHAR  ServerIPAddress[80];

	strcpy(ServerIPAddress, ((Opc_client_com_Instance*)Parent)->Cfg.OpcServerIPAddress);

	if((strlen(ServerIPAddress) == 0))
	{
		local_server = 1;
	}
	
	if(local_server)
	{
		//COM connection

		// browse registry for OPC 1.0A Servers
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
		
		strcpy(serv, ((Opc_client_com_Instance*)Parent)->Cfg.OpcServerProgID);

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

		hr = ::CoCreateInstance(clsid, NULL, CLSCTX_ALL, IID_IOPCServer, (void**)&g_pIOPCServer);

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
		Opc_client_com_DriverThread::ShowMessage(S_OK, "", show_msg);

		WORD wMajor, wMinor, wBuild;

		LPWSTR pwsz = NULL;

		if(!GetStatus(&wMajor, &wMinor, &wBuild, &pwsz))
		{
			char ver[150];
			sprintf(ver,"Server version: %d.%d.%d, %s", wMajor, wMinor, wBuild, W2T(pwsz));
			printf("%s\n\n",ver);
			IT_COMMENT4("Version: %d.%d.%d, %s", wMajor, wMinor, wBuild,W2T(pwsz));
			
			Opc_client_com_DriverThread::ShowMessage(S_OK, "",ver);
			::CoTaskMemFree(pwsz);
		}

		g_bVer2 = Version2();

		if(g_bVer2)
		{
			printf("Server supports OPC 2.0 interfaces\n\n");
			IT_COMMENT("Server supports OPC 2.0 interfaces");
		}

		hr = g_pIOPCServer->QueryInterface(IID_IOPCBrowseServerAddressSpace, (void**)&g_pIOPCBrowse);

		if(FAILED(hr))
		{
			ShowError(hr,"QueryInterface(IID_IOPCBrowseServerAddressSpace");
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

		Implist[0] = IID_CATID_OPCDAServer20;

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
		strcpy(serverName, ((Opc_client_com_Instance*)Parent)->Cfg.OpcServerProgID);
				
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

		hr = pIUnknown->QueryInterface(IID_IOPCServer, (void**)&g_pIOPCServer);

		if (FAILED(hr))
		{
			printf("OPC error:Failed obtain IID_IOPCServer interface from server, %x\n", hr);
			ShowError(hr,"Failed obtain IID_IOPCServer interface from server");
			return 1;
		}

		printf("Connected to server %s.\n", ServerIPAddress);

		sprintf(show_msg, "Connected to server %s.\n", ServerIPAddress);
		Opc_client_com_DriverThread::ShowMessage(S_OK, "", show_msg);


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
			Opc_client_com_DriverThread::ShowMessage(S_OK, "",ver);
			::CoTaskMemFree(pwsz);
		}

		g_bVer2 = Version2();

		if(g_bVer2)
		{
			printf("Server supports OPC 2.0 interfaces\n\n");
		}
		
		hr = g_pIOPCServer->QueryInterface(IID_IOPCBrowseServerAddressSpace, (void**)&g_pIOPCBrowse);

		if (FAILED(hr))
		{
			printf("OPC error:Failed to obtain IID_IOPCBrowseServerAddressSpace interface %x\n",hr);
			ShowError(hr,"Failed to obtain IID_IOPCBrowseServerAddressSpace interface");
			return 1;
		}
		
		hr = g_pIOPCServer->QueryInterface(IID_IOPCItemProperties, (void**)&g_iOpcProperties);

		if (FAILED(hr))
		{
			printf("OPC error:Failed to obtain IID_IOPCItemProperties interface, %x\n",hr);
			ShowError(hr,"Failed to obtain IID_IOPCItemProperties interface");
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
	}

    return(0);
}

int Opc_client_com_DriverThread::OpcStop()
{
	IT_IT("Opc_client_com_DriverThread::OpcStop");

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

int Opc_client_com_DriverThread::GetStatus(WORD *pwMav, WORD *pwMiv, WORD *pwB, LPWSTR *pszV)
{
	IT_IT("Opc_client_com_DriverThread::GetStatus");

	*pwMav = 0;
	*pwMiv = 0;
	*pwB = 0;
	*pszV = NULL;
	OPCSERVERSTATUS *pStatus = NULL;
	if(g_pIOPCServer == NULL) return E_POINTER;
	HRESULT hr = g_pIOPCServer->GetStatus(&pStatus);
	if(FAILED(hr) || (pStatus == NULL) || (pStatus->dwServerState != OPC_STATUS_RUNNING))
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
bool Opc_client_com_DriverThread::Version2()
{
	IT_IT("Opc_client_com_DriverThread::Version2");

	if(g_pIOPCServer == NULL) return false;
	IConnectionPointContainer *pCPC = NULL;
	if(FAILED(g_pIOPCServer->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC)))
	{
		return false;
	}
	pCPC->Release();
	return true;
}


int Opc_client_com_DriverThread::AddItems()
{
	IT_IT("Opc_client_com_DriverThread::AddItems");

	// loop until all items are added
	char sz2[128];
	//TCHAR szBuffer[256];
	HRESULT hr = 0;
	int nTestItem = 0; // how many items there are
	IEnumString* pEnumString = NULL;
		
	USES_CONVERSION;

    hr = g_pIOPCBrowse->BrowseOPCItemIDs(OPC_FLAT, L""/*NULL*/, VT_EMPTY, 0, &pEnumString);

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
	

		hr = g_pIOPCBrowse->BrowseOPCItemIDs(OPC_FLAT, L""/*NULL*/, VT_EMPTY, 0, &pEnumString);

		if(FAILED(hr))
		{
			ShowError(hr, _T("BrowseOPCItemIDs()"));
			return 1;
		}

		QString unit = ((Opc_client_com_Instance*)Parent)->unit_name;

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
					//Opc_client_com_DriverThread::ShowMessage(S_OK, "Readable",(const char*)W2T(pItemAttr[i].szItemID));
				}

				ar = pItemAttr[i].dwAccessRights & OPC_WRITEABLE;

				if(ar == OPC_WRITEABLE)
				{
					//printf(" - Writable");
					IT_COMMENT(" - Writable");
					//Opc_client_com_DriverThread::ShowMessage(S_OK, "Writable",(const char*)W2T(pItemAttr[i].szItemID));
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
	
	
	return(0);
}

void Opc_client_com_DriverThread::ShowError(HRESULT hr, LPCSTR pszError)
{
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
	{
		QString err;
		err.sprintf("Error: %s, %lX", pszError, hr);

		postEvent(StaticParent,new DriverEvent(StaticThis, DriverEvent::OpSendAlarmString,0,err));
	}
}

void Opc_client_com_DriverThread::ShowMessage(HRESULT hr, LPCSTR pszError, const char* name)
{
	//if(Opc_client_com_DriverThread::mandare_eventi)
	{	
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
		{
			QString err;
			err.sprintf("%s: %s, %lX", name, pszError, hr);
			postEvent(StaticParent,new DriverEvent(StaticThis, DriverEvent::OpSendEventString,0,err));
		}
	}		
}

/*

void Opc_client_com_DriverThread::StartErrorLog()
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

void Opc_client_com_DriverThread::EndErrorLog()
{
	if(g_stream)
	{
		fprintf(g_stream, "%sIndigoSCADA OPC Client End.\n", GetDateTime());
		fclose(g_stream);
	}
}

LPCSTR Opc_client_com_DriverThread::GetDateTime()
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

