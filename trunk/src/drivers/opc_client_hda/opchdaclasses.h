#ifndef __OPCHDACLASSES_H
#define __OPCHDACLASSES_H


/*
typedef struct tagOPCHDA_ITEM
    {
    OPCHANDLE hClient;
    DWORD haAggregate;
    DWORD dwCount;
    // [size_is] 
	FILETIME *pftTimeStamps;
    // [size_is] 
	DWORD *pdwQualities;
    // [size_is] 
	VARIANT *pvDataValues;
    } 	OPCHDA_ITEM;
*/

//////////////////////////class COPCHistoricDASink//////////////////////////////////////

class ATL_NO_VTABLE COPCHistoricDASink : 
	public CComObjectRoot,
	public IOPCHDA_DataCallback
{
public:
	COPCHistoricDASink(){}

BEGIN_COM_MAP(COPCHistoricDASink)
	//COM_INTERFACE_ENTRY_IID( __uuidof(IOPCHDA_DataCallback), IOPCHDA_DataCallback)
	COM_INTERFACE_ENTRY(IOPCHDA_DataCallback)
END_COM_MAP()

	HRESULT STDMETHODCALLTYPE OnDataChangeOnDataChange( 
            /* [in] */ DWORD dwTransactionID,
            /* [in] */ HRESULT hrStatus,
            /* [in] */ DWORD dwNumItems,
            /* [size_is][in] */ OPCHDA_ITEM *pItemValues,
            /* [size_is][in] */ HRESULT *phrErrors) 
	{
		IT_IT("COPCHistoricDASink::OnDataChangeOnDataChange");

		//IMPORTANT NOTE: no blocking function may be called here

		char show_msg[150];

		for(DWORD i = 0; i < dwNumItems; i++)
		{
			/*
			printf("%ls  %ls   %ls   %ls\n", pItemValues[i].szSource, 
									pItemValues[i].szMessage,
									pItemValues[i].szConditionName, 
									pItemValues[i].szSubconditionName);
			*/

			/*
			printf("hClientSubscription = %d, bRefresh = %d bLastRefresh = %d dwCount = %d\n", hClientSubscription, bRefresh, bLastRefresh, dwCount);

			printf("%d, %d, %ls, %ls, %d, %d, %d, %ls, %ls, %d, %d, %d, %d, %d, %ls\n", 

				pItemValues[i].wChangeMask,
				pItemValues[i].wNewState,
				pItemValues[i].szSource,
				//FILETIME pItemValues[i].ftTime,
				pItemValues[i].szMessage,
				pItemValues[i].dwEventType,
				pItemValues[i].dwEventCategory,
				pItemValues[i].dwSeverity,
				pItemValues[i].szConditionName,
				pItemValues[i].szSubconditionName,
				pItemValues[i].wQuality,
				pItemValues[i].wReserved,
				pItemValues[i].bAckRequired,
				//FILETIME pItemValues[i].ftActiveTime,
				pItemValues[i].dwCookie,
				pItemValues[i].dwNumEventAttrs,
				//[size_is] VARIANT *pEventAttributes,
				pItemValues[i].szActorID);


				sprintf(show_msg, "%d, %d, %ls, %ls, %d, %d, %d, %ls, %ls, %d, %d, %d, %d, %d, %ls", 
				pItemValues[i].wChangeMask,
				pItemValues[i].wNewState,
				pItemValues[i].szSource,
				//FILETIME pItemValues[i].ftTime,
				pItemValues[i].szMessage,
				pItemValues[i].dwEventType,
				pItemValues[i].dwEventCategory,
				pItemValues[i].dwSeverity,
				pItemValues[i].szConditionName,
				pItemValues[i].szSubconditionName,
				pItemValues[i].wQuality,
				pItemValues[i].wReserved,
				pItemValues[i].bAckRequired,
				//FILETIME pItemValues[i].ftActiveTime,
				pItemValues[i].dwCookie,
				pItemValues[i].dwNumEventAttrs,
				//[size_is] VARIANT *pEventAttributes,
				pItemValues[i].szActorID);

				Opc_client_hda_DriverThread::ShowMessage(S_OK, "", show_msg);
				*/
		}
		
		return S_OK;
	};
};

typedef CComObject<COPCHistoricDASink> CComCOPCHistoricDASink;

//////////////////////////class COPCShutdownRequest//////////////////////////////////////

class ATL_NO_VTABLE COPCShutdownRequest : 
	//public CComObjectRootEx<CComSingleThreadModel>,
	public CComObjectRoot,
	public IOPCShutdown
{
public:
	COPCShutdownRequest(){}

BEGIN_COM_MAP(COPCShutdownRequest)
	//COM_INTERFACE_ENTRY_IID( __uuidof(IOPCShutdown), IOPCShutdown)
	COM_INTERFACE_ENTRY(IOPCShutdown)
END_COM_MAP()

	HRESULT STDMETHODCALLTYPE ShutdownRequest( 
            /* [string][in] */ LPCWSTR szReason)
	{
		char show_msg[150];
		sprintf(show_msg, "Remote HDA server sends shutdown request");

		Opc_client_ae_DriverThread::ShowMessage(S_OK, "", show_msg);
		return S_OK;
	};
};

typedef CComObject<COPCShutdownRequest> CComCOPCShutdownRequest;


#endif