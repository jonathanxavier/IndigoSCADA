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
#ifndef __OPCDA_2_0_CLASSES_H
#define __OPCDA_2_0_CLASSES_H

//////////OPC DA 2.0/////////////

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

		{
			if(FAILED(hrMastererror))
			{
				Opc_client_da_DriverThread::ShowError(hrMastererror,"General ConnectionPoint Update");
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

					Opc_client_da_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);
				}
				else if((pwQualities[dw] != OPC_QUALITY_GOOD) && SUCCEEDED(pErrors[dw]))
				{
					VARIANT *pValue = &pvValues[dw];

					//SINCRONISMO_LOCK

					const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);
																										
					Opc_client_da_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);

					QString name;
					name = QString(Opc_client_da_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				}
				else // else if
				{
					//SINCRONISMO_LOCK   //lock the other threads to enter in this critical section
					Opc_client_da_DriverThread::SendEvent2(0, 0, pwQualities[dw], phClientItems[dw]);
					
					//const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);

					QString name;
					name = QString(Opc_client_da_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
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

		{
			if(FAILED(hrMastererror))
			{
				Opc_client_da_DriverThread::ShowError(hrMastererror,"General Async2 Read");
			}

			if(dwTransid != Opc_client_da_DriverThread::g_dwReadTransID)
			{
				Opc_client_da_DriverThread::ShowError(S_OK,"Async2 Read callback, TransactionID's do not match");
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

					Opc_client_da_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);
				}
				else if((pwQualities[dw] != OPC_QUALITY_GOOD) && SUCCEEDED(pErrors[dw]))
				{
					VARIANT *pValue = &pvValues[dw];

					//SINCRONISMO_LOCK

					const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);
																										
					Opc_client_da_DriverThread::SendEvent2(pValue, ft, pwQualities[dw], phClientItems[dw]);

					QString name;
					name = QString(Opc_client_da_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				}
				else // else if
				{
					//SINCRONISMO_LOCK   //lock the other threads to enter in this critical section
					Opc_client_da_DriverThread::SendEvent2(0, 0, pwQualities[dw], phClientItems[dw]);
					
					//const FILETIME* ft = reinterpret_cast<const FILETIME *>(&pftTimeStamps[dw]);

					QString name;
					name = QString(Opc_client_da_DriverThread::Item[phClientItems[dw] - 1].spname);

					switch(pwQualities[dw])
					{
						case OPC_QUALITY_GOOD:
							//ShowError(S_OK, "Quality Good");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality Good",(const char*)name);
							break;
						case OPC_QUALITY_BAD:
						default:
							//ShowError(S_OK, "Quality Bad");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality Bad",(const char*)name);
							break;
						case OPC_QUALITY_UNCERTAIN:
							//ShowError(S_OK, "Quality UNCERTAIN");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "Quality UNCERTAIN",(const char*)name);
							break;
						case OPC_QUALITY_CONFIG_ERROR:
							//ShowError(S_OK, "CONFIG ERROR");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "CONFIG ERROR",(const char*)name);
							break;
						case OPC_QUALITY_NOT_CONNECTED:
							//ShowError(S_OK, "NOT CONNECTED");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "NOT CONNECTED",(const char*)name);
							break;
						case OPC_QUALITY_DEVICE_FAILURE:
							//ShowError(S_OK, "DEVICE FAILURE");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "DEVICE FAILURE",(const char*)name);
							break;
						case OPC_QUALITY_OUT_OF_SERVICE:
							//ShowError(S_OK, "OUT OF SERVICE");
							Opc_client_da_DriverThread::ShowMessage(S_OK, "OUT OF SERVICE",(const char*)name);
							break;
					}
				} // endif

				//printf("\n");

			} // end for

			Opc_client_da_DriverThread::g_bReadComplete = true;

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
			Opc_client_da_DriverThread::ShowError(hrMastererr,"General Async2 Write");

			//Send error detected in execution of command
		}

		//fprintf(stderr, "g_dwWriteTransID = %d, dwTransid = %d\n", Opc_client_da_DriverThread::g_dwWriteTransID, dwTransid);
		//fflush(stderr);

		if(Opc_client_da_DriverThread::g_dwWriteTransID != dwTransid)
		{
			Opc_client_da_DriverThread::ShowError(S_OK,"Async2 Write callback, TransactionID's do not match");
		}

		for(DWORD dw=0; dw < dwCount; dw++)
		{
			if(FAILED(pErrors[dw]))
			{
				Opc_client_da_DriverThread::ShowError(pErrors[dw], "Async2 Write request");
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

		Opc_client_da_DriverThread::g_bWriteComplete = true;

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

typedef CComObject<COPCCallback> CComCOPCCallback;

#endif //__OPCDA_2_0_CLASSES_H