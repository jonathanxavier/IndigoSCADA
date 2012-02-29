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
#ifndef __OPCDA_1_0_CLASSES_H
#define __OPCDA_1_0_CLASSES_H

//////////OPC DA 1.0/////////////

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
					Opc_client_da_DriverThread::ShowError(pHeader->hrStatus,"General Async Write");
				}

				if(Opc_client_da_DriverThread::g_dwWriteTransID != pHeader->dwTransactionID)
				{
					Opc_client_da_DriverThread::ShowError(S_OK,"Async Write callback, TransactionID's do not match");
				}

				DWORD dwSize = sizeof(OPCGROUPHEADERWRITE);

				for(DWORD dw=0; dw < pHeader->dwItemCount; dw++, dwSize += sizeof(OPCITEMHEADERWRITE))
				{
					const OPCITEMHEADERWRITE* pItemHeader = reinterpret_cast<const OPCITEMHEADERWRITE*>(&pBuffer[dwSize]);

					if(FAILED(pItemHeader->dwError))
					{
						Opc_client_da_DriverThread::ShowError(pItemHeader->dwError,"Async Write request");
					}
				}
				
				//Opc_client_da_DriverThread::ShowMessage(S_OK,"g_bWriteComplete", "Messaggio");
				//printf("Write completa");
				Opc_client_da_DriverThread::g_bWriteComplete = true;

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
				Opc_client_da_DriverThread::ShowError(pHeader->hrStatus,"General Async Read");
			}

			if(Opc_client_da_DriverThread::g_bPoll)
			{
				// if we are polling, ignore async updates
				if(pHeader->dwTransactionID == 0)
				{
					return;
				}

				READ_LOCK; //lock the other threads to enter in this critical section

				if(pHeader->dwTransactionID != Opc_client_da_DriverThread::g_dwReadTransID)
				{
					Opc_client_da_DriverThread::ShowError(S_OK,"Async Read callback, TransactionID's do not match");
					
					return;
				}

				if(!(Opc_client_da_DriverThread::g_bReadComplete))
				{ 
					Opc_client_da_DriverThread::g_bReadComplete = true;
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

					Opc_client_da_DriverThread::SendEvent(pItemHeader, pValue, ft);
					
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
					Opc_client_da_DriverThread::SendEvent(pItemHeader, 0, 0);
					
					//const FILETIME* ft = reinterpret_cast<const FILETIME *>(&(pItemHeader->ftTimeStampItem));

					QString name;
					name = QString(Opc_client_da_DriverThread::Item[pItemHeader->hClient].spname);
					//name = QString(Opc_client_da_DriverThread::Item[pItemHeader->hClient - 1].spname); mettere il -1 ?
					
					switch(pItemHeader->wQuality)
					{
						case OPC_QUALITY_BAD:
						default:
						{
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
						}
						break;
						case OPC_QUALITY_UNCERTAIN:
						{
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
						}
						break;
						case OPC_QUALITY_CONFIG_ERROR:
						{
							Opc_client_da_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
						}
						break;
						case OPC_QUALITY_NOT_CONNECTED:
						{
							Opc_client_da_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
						}
						break;
						case OPC_QUALITY_DEVICE_FAILURE:
						{
							Opc_client_da_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
						}
						break;
						case OPC_QUALITY_OUT_OF_SERVICE:
						{
							Opc_client_da_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
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

typedef CComObject<CTestAdviseSink> CComCTestAdviseSink;

#endif //__OPCDA_1_0_CLASSES_H