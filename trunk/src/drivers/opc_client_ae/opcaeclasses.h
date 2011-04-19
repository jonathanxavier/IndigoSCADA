#ifndef __OPCAECLASSES_H
#define __OPCAECLASSES_H

/*
typedef  struct __MIDL___MIDL_itf_opc_ae_0257_0004
    {
    WORD wChangeMask;
    WORD wNewState;
    LPWSTR szSource;
    FILETIME ftTime;
    LPWSTR szMessage;
    DWORD dwEventType;
    DWORD dwEventCategory;
    DWORD dwSeverity;
    LPWSTR szConditionName;
    LPWSTR szSubconditionName;
    WORD wQuality;
    WORD wReserved;
    BOOL bAckRequired;
    FILETIME ftActiveTime;
    DWORD dwCookie;
    DWORD dwNumEventAttrs;
    [size_is] VARIANT *pEventAttributes;
    LPWSTR szActorID;
    } 	ONEVENTSTRUCT;

*/

//////////////////////////class COPCEventSink//////////////////////////////////////

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

	HRESULT STDMETHODCALLTYPE OnEvent( 
            /* [in] */ OPCHANDLE hClientSubscription,
            /* [in] */ BOOL bRefresh,
            /* [in] */ BOOL bLastRefresh,
            /* [in] */ DWORD dwCount,
            /* [size_is][in] */ ONEVENTSTRUCT *pEvents) 
	{
		IT_IT("COPCEventSink::OnEvent");

		//IMPORTANT NOTE: no blocking function may be called here

		char show_msg[150];

		for(DWORD i = 0; i < dwCount; i++)
		{
			/*
			printf("%ls  %ls   %ls   %ls\n", pEvents[i].szSource, 
									pEvents[i].szMessage,
									pEvents[i].szConditionName, 
									pEvents[i].szSubconditionName);
			*/

			printf("hClientSubscription = %d, bRefresh = %d bLastRefresh = %d dwCount = %d\n", hClientSubscription, bRefresh, bLastRefresh, dwCount);

			printf("%d, %d, %ls, %ls, %d, %d, %d, %ls, %ls, %d, %d, %d, %d, %d, %ls\n", 

				pEvents[i].wChangeMask,
				pEvents[i].wNewState,
				pEvents[i].szSource,
				//FILETIME pEvents[i].ftTime,
				pEvents[i].szMessage,
				pEvents[i].dwEventType,
				pEvents[i].dwEventCategory,
				pEvents[i].dwSeverity,
				pEvents[i].szConditionName,
				pEvents[i].szSubconditionName,
				pEvents[i].wQuality,
				pEvents[i].wReserved,
				pEvents[i].bAckRequired,
				//FILETIME pEvents[i].ftActiveTime,
				pEvents[i].dwCookie,
				pEvents[i].dwNumEventAttrs,
				//[size_is] VARIANT *pEventAttributes,
				pEvents[i].szActorID);


				sprintf(show_msg, "%d, %d, %ls, %ls, %d, %d, %d, %ls, %ls, %d, %d, %d, %d, %d, %ls", 
				pEvents[i].wChangeMask,
				pEvents[i].wNewState,
				pEvents[i].szSource,
				//FILETIME pEvents[i].ftTime,
				pEvents[i].szMessage,
				pEvents[i].dwEventType,
				pEvents[i].dwEventCategory,
				pEvents[i].dwSeverity,
				pEvents[i].szConditionName,
				pEvents[i].szSubconditionName,
				pEvents[i].wQuality,
				pEvents[i].wReserved,
				pEvents[i].bAckRequired,
				//FILETIME pEvents[i].ftActiveTime,
				pEvents[i].dwCookie,
				pEvents[i].dwNumEventAttrs,
				//[size_is] VARIANT *pEventAttributes,
				pEvents[i].szActorID);

				Opc_client_ae_DriverThread::ShowMessage(S_OK, "", show_msg);
		}
		
		return S_OK;
	};
};

typedef CComObject<COPCEventSink> CComCOPCEventSink;

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
		sprintf(show_msg, "Remote A&E server sends shutdown request");

		Opc_client_ae_DriverThread::ShowMessage(S_OK, "", show_msg);
		return S_OK;
	};
};

typedef CComObject<COPCShutdownRequest> CComCOPCShutdownRequest;


#endif