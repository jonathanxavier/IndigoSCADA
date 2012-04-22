/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2012 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#define INITGUID
#define _WIN32_DCOM
#include <windows.h>
#include <locale.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "device.h"
#include "unilog.h"

#define LOGID logg,0
#define LOG_FNAME "opc_server_da.log"
#define CFG_FILE "opc_server_da.ini"

#include <opcda.h>
#include <opcerror.h>
#include "lightopc.h"

#define ECL_SID  "Enscada OPC server"// identificator of OPC server
#define PROG_ID "Enscada.OPC.Srv" //OPC Server ProgID

unilog *logg = NULL;

/**************************************************************************
			   OPC vendor info
**************************************************************************/
static const loVendorInfo vendor = {
  2 /*Major */ , 1 /*Minor */ , 10 /*Build */ , 0 /*Reserv */ ,
  ECL_SID
};

loService *our_service;
static int OPCstatus = OPC_STATUS_RUNNING;

#define TAGNAME_LEN 64

static DEV *devp;

static int driver_init(int lflags);
static void driver_destroy(void);
static void server_finished(void*, loService*, loClient*);

//-OPC Server CLSID

// {78BB6572-B8BA-47f9-83B3-4EC99DF9B1A3}
static const GUID GID_EnscadaOPCserverExe = 
{ 0x78bb6572, 0xb8ba, 0x47f9, { 0x83, 0xb3, 0x4e, 0xc9, 0x9d, 0xf9, 0xb1, 0xa3 } };

//*************************************************************************
class ourClassFactory: public IClassFactory
{
public:
  LONG RefCount;
  LONG server_count;
  CRITICAL_SECTION lk;  /* protect RefCount */
  
  ourClassFactory(): RefCount(0), server_count(0)
  {
    InitializeCriticalSection(&lk);
  }

  ~ourClassFactory()
  {
    DeleteCriticalSection(&lk);
  }

  //IUnknown
  STDMETHODIMP QueryInterface(REFIID, LPVOID*);
  STDMETHODIMP_(ULONG) AddRef( void);
  STDMETHODIMP_(ULONG) Release( void);
  // IClassFactory
  STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
  STDMETHODIMP LockServer(BOOL);
  //
  inline LONG getRefCount(void)
  {
    LONG rc;
    EnterCriticalSection(&lk);
    rc = RefCount;
    LeaveCriticalSection(&lk);                                    
    return rc;
  }

  inline int in_use(void)
  {
    int rv;
    EnterCriticalSection(&lk);
    rv = RefCount | server_count;
    LeaveCriticalSection(&lk);
    return rv;
  }

  inline void serverAdd(void)
  {
    InterlockedIncrement(&server_count);
  }

  inline void serverRemove(void)
  {
    InterlockedDecrement(&server_count);
  }

};

/**************************************************************************
 IUnknown
**************************************************************************/
STDMETHODIMP ourClassFactory::QueryInterface(REFIID iid, LPVOID* ppInterface)
{
  if (ppInterface == NULL) return E_INVALIDARG;

  if (iid == IID_IUnknown || iid == IID_IClassFactory)
    {
      UL_DEBUG((LOGID, "ourClassFactory::QueryInterface() Ok"));
      *ppInterface = this;
      AddRef();
      return S_OK;
    }
  UL_DEBUG((LOGID, "ourClassFactory::QueryInterface() Failed"));

  *ppInterface = NULL;
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ourClassFactory::AddRef(void)
{
  ULONG rv;
  EnterCriticalSection(&lk);
  rv = (ULONG)++RefCount;
  LeaveCriticalSection(&lk);                                    
  UL_DEBUG((LOGID, "ourClassFactory::AddRef(%ld)", rv));
  return rv;
}

STDMETHODIMP_(ULONG) ourClassFactory::Release(void)
{
  ULONG rv;
  EnterCriticalSection(&lk);
  rv = (ULONG)--RefCount;
  LeaveCriticalSection(&lk);
  UL_DEBUG((LOGID, "ourClassFactory::Release(%d)", rv));
  return rv;
}
/**************************************************************************
 IClassFactory
**************************************************************************/

STDMETHODIMP ourClassFactory::LockServer(BOOL fLock)
{
  if (fLock)
    AddRef();
  else
    Release();

  UL_DEBUG((LOGID, "ourClassFactory::LockServer(%d)", fLock)); 
  return S_OK;
}

STDMETHODIMP ourClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppvObject)
{
  if (pUnkOuter != NULL)
    return CLASS_E_NOAGGREGATION; // Aggregation is not supported by this code

  IUnknown *server = 0;

  AddRef(); /* for server_finished() */
  if (loClientCreate(our_service, (loClient**)&server, 0, &vendor, server_finished, this))
    {
      UL_DEBUG((LOGID, "ourClassFactory::loCreateClient() failed"));
      Release();
      return E_OUTOFMEMORY;
    }
  serverAdd();

  HRESULT hr = server->QueryInterface(riid, ppvObject);
  if (FAILED(hr))
    {
      UL_DEBUG((LOGID, "ourClassFactory::loClient QueryInterface() failed"));
    }
  else
    {
      loSetState(our_service, (loClient*)server, loOP_OPERATE, OPCstatus, 0);
      UL_DEBUG((LOGID, "ourClassFactory::server_count = %ld", server_count));
    }
  server->Release();
  return hr;
}

/***************************************************************************
 EXE-specific stuff
***************************************************************************/

static ourClassFactory our_CF;

char argv0[FILENAME_MAX + 32];

void timetToFileTime( time_t t, LPFILETIME pft )
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD) ll;
    pft->dwHighDateTime = (unsigned long)(ll >>32);
}

char *logPath(char *fileName)
{
  static char path[MAX_PATH]="\0";
//  char *cp;
  
  strcpy(path, argv0);
  //if(NULL==(cp=strrchr(path,'\\'))) cp=path; else cp++;
  //cp=strcpy(cp,fileName);

  *(strrchr(path, '\\')) = '\0';        // Strip \\filename.exe off path
  *(strrchr(path, '\\')) = '\0';        // Strip \\bin off path

  strcat(path, "\\logs");
  strcat(path, "\\");
  strcat(path, fileName);

  return path;
}

static void server_finished(void *a, loService *b, loClient *c)
{
  our_CF.serverRemove();
  if (a) ((ourClassFactory*)a)->Release();
  UL_DEBUG((LOGID, "server_finished(%lu)", our_CF.server_count));
}

inline void init_common(void)
{
  logg = unilog_Create(ECL_SID, logPath(LOG_FNAME), NULL,
		      -1, /* Max filesize: -1 unlimited, -2 -don't change */
		      ll_DEBUG); /* level [ll_FATAL...ll_DEBUG] */

  UL_INFO((LOGID, "Start"));
}

inline void cleanup_common(void)
{
  UL_INFO((LOGID, "Finish"));
  unilog_Delete(logg); logg = NULL;
}

inline void cleanup_all(DWORD objid)
{
  if (FAILED(CoRevokeClassObject(objid)))
    UL_WARNING((LOGID, "CoRevokeClassObject() failed..."));
  driver_destroy();
  CoUninitialize();
  cleanup_common();
}

static void poll_device(void);
static int opc_main(HINSTANCE hInstance, int argc, char *argv[]);
static int show_msg(LPCTSTR msg);
static int show_error(LPCTSTR msg);

extern "C" int main(int argc, char *argv[])
{
  return opc_main(GetModuleHandle(NULL), argc, argv);
}

#define SUPPLIER "@ enscada.com"
#define APPLICATION "opc_server_da.exe"

int opc_main(HINSTANCE hInstance, int argc, char *argv[]) {
  const char eClsidName [] = ECL_SID;
  const char eProgID [] = PROG_ID;
  DWORD objid;
  char *cp;

  //version control///////////////////////////////////////////////////////////////
  char version[100];
  sprintf(version, ""APPLICATION" - Built on %s %s %s",__DATE__,__TIME__,SUPPLIER);
  fprintf(stderr, "%s\n", version);
  fflush(stderr);
  SYSTEMTIME oT;
  ::GetLocalTime(&oT);
  fprintf(stderr,"%02d/%02d/%04d, %02d:%02d:%02d Starting ... %s\n",oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,APPLICATION); 
  fflush(stderr);
  ////////////////////////////////////////////////////////////////////////////////

  objid=::GetModuleFileName(NULL, argv0, sizeof(argv0));
  if(objid==0 || objid+50 > sizeof(argv0)) return 0;

  init_common();

  if((cp = setlocale(LC_ALL, ".1251")) == NULL) 
  {
    UL_ERROR((LOGID, "setlocale() - Can't set 1251 code page"));
    cleanup_common();
    return 0;
  }

  cp = argv[1];
  if(cp) 
  {
    int finish = 1;

    if (strstr(cp, "/r")) 
	{
      if(loServerRegister(&GID_EnscadaOPCserverExe, eProgID, eClsidName, argv0, 0)) 
	  {
		show_error("Registration Failed");
		UL_ERROR((LOGID, "Registration <%s> <%s> Failed", eProgID, argv0));    
      } 
	  else 
	  {
		show_msg("Registration Ok");
		UL_INFO((LOGID, "Registration <%s> <%s> Ok", eProgID, argv0));    
      }
    } 
	else if (strstr(cp, "/u")) 
	{
      if (loServerUnregister(&GID_EnscadaOPCserverExe, eProgID)) 
	  {
		show_error("UnRegistration Failed");
		UL_ERROR((LOGID, "UnReg <%s> <%s> Failed", eClsidName, argv0));    
      } 
	  else 
	  {
		show_msg("Server Unregistered");
		UL_INFO((LOGID, "UnReg <%s> <%s> Ok", eClsidName, argv0));
      }
    } 
	else 
	{
      UL_WARNING((LOGID, "Ignore unknown option <%s>", cp));
      finish = 0;
    }

    if (finish) 
	{
      cleanup_common();
      return 0;
    }
  }

  if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) 
  {
    UL_ERROR((LOGID, "CoInitializeEx() failed. Exiting..."));
    cleanup_common();
    return 0;
  }

  /////////////////Allocate device//////////////////////////////
  devp = new DEV();

  devp->idnum = 1; //Only for DEMO
  devp->ids[0] = 101;
  //////////////////////////////////////////////////////////////

  if(driver_init(0)) 
  { 
    CoUninitialize();
    cleanup_common();
    return 0;
  }

  if (FAILED(CoRegisterClassObject(GID_EnscadaOPCserverExe, &our_CF, 
				   CLSCTX_LOCAL_SERVER|
				   CLSCTX_REMOTE_SERVER|
				   CLSCTX_INPROC_SERVER, 
				   REGCLS_MULTIPLEUSE, &objid)))
  {
      UL_ERROR((LOGID, "CoRegisterClassObject() failed. Exiting..."));
      cleanup_all(objid);
      return 0;
  }

  Sleep(3000);
  our_CF.Release(); /* avoid locking by CoRegisterClassObject() */

  if (OPCstatus != OPC_STATUS_RUNNING) 
  {
    while(our_CF.in_use())
      Sleep(1000);
    cleanup_all(objid);
    return 0;
  }

  while(our_CF.in_use())
  {
    poll_device();
  }
  
 cleanup_all(objid);
 return 0;
}

/***************************************************************************
	       The Process Data to be exported via OPC
***************************************************************************/

static CRITICAL_SECTION lk_values; /* protects ti[] from simultaneous access */

int tTotal;
static loTagId *ti;
static char **tn;
static loTagValue *tv;

int WriteTags(const loCaller *ca,
              unsigned count, loTagPair taglist[],
              VARIANT values[], HRESULT error[], HRESULT *master, LCID lcid)
{
	unsigned i, ii, ei, devi;
	VARIANT v;
	char cmdData[DEV_DATALEN_MAX+1];
	char ldm;
	struct lconv *lcp;

	lcp = localeconv();
	ldm = *(lcp->decimal_point);
	VariantInit(&v);

	UL_TRACE((LOGID, "WriteTags(%d) invoked", count));

	EnterCriticalSection(&lk_values);

	for(ii = 0; ii < count; ii++) 
	{
		HRESULT hr = S_OK;
		loTagId clean = 0;
		cmdData[0] = '\0';
		cmdData[DEV_DATALEN_MAX] = '\0';

		UL_TRACE((LOGID,  "WriteTags(Rt=%u Ti=%u)", taglist[ii].tpRt, taglist[ii].tpTi));
		i = (unsigned)taglist[ii].tpRt - 1;
		ei = i % devp->cv_size;
		devi = i / devp->cv_size;

		if (!taglist[ii].tpTi || !taglist[ii].tpRt || i >= (unsigned)tTotal)
		continue;

		VARTYPE tvVt = tv[i].tvValue.vt;
		hr = VariantChangeType(&v, &values[ii], 0, tvVt);
		char *dm;

		if(hr == S_OK) 
		{
			switch (tvVt) 
			{
				case VT_I2:
				{
					sprintf(cmdData, "%+0d", v.iVal);
				}
				break;
				case VT_R4:
				{
					sprintf(cmdData, "%+0f", v.fltVal);
					if (ldm != '.' && (dm = strchr(cmdData, ldm)))
					*dm = '.';
				}
				break;
				case VT_BSTR:
				default:
				{
					WideCharToMultiByte(CP_ACP,
					0,
					v.bstrVal,
					-1,
					cmdData,
					DEV_DATALEN_MAX,
					NULL, NULL);
				}
			}

			//TODO: execute write on device hardware or send cmdData in command direction
		}

		VariantClear(&v);

		if (S_OK != hr) 
		{
			*master = S_FALSE;
			error[ii] = hr;

			UL_TRACE((LOGID, "%!l WriteTags(Rt=%u Ti=%u %s %s)", hr, taglist[ii].tpRt, taglist[ii].tpTi, tn[i], cmdData));
		}

		taglist[ii].tpTi = clean; /* clean if ok */
	}

	LeaveCriticalSection(&lk_values);

	return loDW_TOCACHE; /* put to the cache all tags unhandled here */
	// loDW_ALLDONE; 
}


loTrid ReadTags(const loCaller *,
		unsigned  count,
		loTagPair taglist[],
		VARIANT   values[],
		WORD      qualities[],
		FILETIME  stamps[],
		HRESULT   errs[],
		HRESULT  *master_err,
		HRESULT  *master_qual,
		const VARTYPE vtype[],
		LCID lcid
		)
{
  return loDR_STORED;
}


void activation_monitor(const loCaller *ca, int count, loTagPair *til)
{
  int act = 1;
  if (0 > count)
    act = 0, count = -count;
  while(count--)
    {
      UL_DEBUG((LOGID, "MON: %u %s %s", til[count].tpTi,
                tn[(int) til[count].tpRt], act ? "On" : "Off"));
    }
}

int init_tags(void)
{
  int devi, i;
  unsigned rights;
  LONG ecode =  S_OK;
  FILETIME ft;

  GetSystemTimeAsFileTime(&ft);

  EnterCriticalSection(&lk_values);

  ti = new loTagId[tTotal];
  tv = new loTagValue[tTotal];
  tn = new char*[tTotal];
  
  for(i = 0; i < tTotal; i++)
  {
    tn[i] = new char[TAGNAME_LEN];
  }

  
  for(devi = 0, i = 0; devi < devp->idnum; devi++) 
  {
    int ei;
    for(ei = 0; ei < devp->cv_size && !ecode; ei++, i++) 
	{
      int id = devp->cv_id[ei];
      sprintf(tn[i], "%s/id%0.2d/%s", "device", devp->ids[devi], DeviceDatabase[id].name);

      rights = (DeviceDatabase[id].readable ? OPC_READABLE : 0)  | (DeviceDatabase[id].writeable ? OPC_WRITEABLE : 0);

      VariantInit(&tv[i].tvValue);

      switch (DeviceDatabase[ei].dtype) 
	  {
		  case VT_I2:
		  {
				V_I2(&tv[i].tvValue) = 0;
				V_VT(&tv[i].tvValue) = VT_I2;
				ecode = loAddRealTag_a(our_service,
							   &ti[i], /* returned TagId */
							   (loRealTag)(i+1), /* != 0 */
							   tn[i],
							   0, /* loTF_XXX */
							   rights,
							   &tv[i].tvValue,
							   -99999,
							   99999);
		  }
		  break;
		  case VT_R4:
		  {
				V_R4(&tv[i].tvValue) = 0.0;
				V_VT(&tv[i].tvValue) = VT_R4;
				ecode = loAddRealTag_a(our_service,
							   &ti[i], /* returned TagId */
							   (loRealTag)(i+1), /* != 0 */
							   tn[i],
							   0, /* loTF_XXX */
							   rights,
							   &tv[i].tvValue,
							   -99999.0,
							   99999.0);
		  }
		  break;
		  case VT_BSTR:
		  default:
		  {
				V_BSTR(&tv[i].tvValue) = SysAllocString(L"");
				V_VT(&tv[i].tvValue) = VT_BSTR;
				ecode = loAddRealTag(our_service,
							 &ti[i], /* returned TagId */
							 (loRealTag)(i+1), /* != 0 */
							 tn[i],
							 0, /* loTF_XXX */
							 rights,
							 &tv[i].tvValue,
							 0,
							 0);
		  }
      }

      UL_TRACE((LOGID, "%!e loAddRealTag(%s) = %u", ecode, tn[i], ti[i]));

      tv[i].tvTi = ti[i];
      tv[i].tvState.tsTime = ft;
      tv[i].tvState.tsError = S_OK;
      tv[i].tvState.tsQuality = OPC_QUALITY_NOT_CONNECTED;

    }
  }

  LeaveCriticalSection(&lk_values);

  if(ecode) 
  {
    UL_ERROR((LOGID, "%!e driver_init()=", ecode));
    return -1;
  }
  return 0;
}

///////////////////Leave here only for DEMO////////////////////
int count = 0;
///////////////////////////////////////////////////////////////

LONG get_val(const int id, const unsigned int cmdnum, LPSTR dat)
{
  DevRecord *cmd;
  LONG ecode = ERROR_SUCCESS;
  unsigned int retCmd = 0;

  if(cmdnum >= sizeof(DeviceDatabase)/sizeof(DevRecord)) 
  {
    UL_ERROR((LOGID, "Invalid command number <%d>", cmdnum));
    exit(1);
  }

  cmd = &(DeviceDatabase[cmdnum]);

  ///////////////////Leave here only for DEMO////////////////////
  count++;
  itoa(count, dat ,10);
  ///////////////////////////////////////////////////////////////

  return ecode;
}

LONG poll_producer(const int id, DEV *dev)
{
  char buf[DEV_DATALEN_MAX+1];
  LONG ecode = ERROR_SUCCESS;
  
  int i;

  for(i = 0; i < dev->cv_size; i++) 
  {
	ecode = get_val(id, dev->cv_id[i], buf);

    if(ecode == ERROR_SUCCESS) 
	{
      strcpy(dev->cv[i], buf);
      dev->cv_status[i] = TRUE;
    } 
	else if(ecode != ERROR_INVALID_OPERATION)
	{
      dev->cv_status[i] = FALSE;
	}
  }

  time(&dev->mtime);

  return ERROR_SUCCESS;
}

void poll_device(void)
{
  FILETIME ft;
  LONG ecode = 0;
  int devi, i;

  //Get data from real time database or from hardware driver

  for(devi = 0, i = 0; devi < devp->idnum; devi++) 
  {
    ecode = poll_producer(devp->ids[i], devp);

    UL_DEBUG((LOGID, "Driver poll <%d:%d>", devp->ids[i], ecode));

    //if(ecode == ERROR_SUCCESS)
	//{
    //  timetToFileTime(devp->mtime, &ft);
	//}
    //else
	//{
      GetSystemTimeAsFileTime(&ft);
	//}

    EnterCriticalSection(&lk_values);

    int ei;
    for (ei = 0; ei < devp->cv_size; ei++, i++)
	{
      WCHAR buf[64];
      LCID lcid = MAKELCID(0x0409, SORT_DEFAULT);

      if(ecode == ERROR_SUCCESS)
	  {
			MultiByteToWideChar(CP_ACP, 0, devp->cv[ei], strlen(devp->cv[ei]) + 1, buf, sizeof(buf)/sizeof(buf[0]));

			UL_DEBUG((LOGID, "set tag <i,status,value>=<%d,%d,%s>", i, devp->cv_status[ei], devp->cv[ei]));

			if(devp->cv_status[ei]) 
			{
			  VARTYPE tvVt = tv[i].tvValue.vt;
			  VariantClear(&tv[i].tvValue);

			  switch (tvVt) 
			  {
				  case VT_I2:
					short vi2;
					VarI2FromStr(buf, lcid, 0, &vi2);
					V_I2(&tv[i].tvValue) = vi2;
					break;
				  case VT_R4:
					float vr4;
					VarR4FromStr(buf, lcid, 0, &vr4);
					V_R4(&tv[i].tvValue) = vr4;
					break;
				  case VT_BSTR:
				  default:
					V_BSTR(&tv[i].tvValue) = SysAllocString(buf);
			  }

			  V_VT(&tv[i].tvValue) = tvVt;

			  //if(VariantChangeType(&tv[i].tvValue, &tv[i].tvValue, 0, tvVt))
			  //UL_ERROR((LOGID, "%!L VariantChangeType(%s)", tn[i]));

			  tv[i].tvState.tsQuality = OPC_QUALITY_GOOD;
			} 
			else if(ecode == ERROR_INVALID_OPERATION)
			{
			  tv[i].tvState.tsQuality = OPC_QUALITY_UNCERTAIN;
			}
			else
			{
			  tv[i].tvState.tsQuality = OPC_QUALITY_OUT_OF_SERVICE;
			}
		} 
		else
		{
			tv[i].tvState.tsQuality = OPC_QUALITY_DEVICE_FAILURE;
		}

        tv[i].tvState.tsTime = ft;
    }

	/** MANDATORY: send all the values into the cache: */
    loCacheUpdate(our_service, tTotal, tv, 0);
    LeaveCriticalSection(&lk_values);
  }
  
  Sleep(1000);

}

int driver_init(int lflags)
{
  loDriver ld;
  int ecode;

  if (our_service) {
      UL_ERROR((LOGID, "Driver already initialized!"));
      return 0;
  }

  tTotal = devp->cv_size*devp->idnum;

  memset(&ld, 0, sizeof(ld));
  //set refresh rate according to reactivity of DEV
  ld.ldRefreshRate = 1000;
  ld.ldRefreshRate_min = 1000;
  ld.ldWriteTags = WriteTags;
  ld.ldReadTags = ReadTags;
  ld.ldSubscribe = activation_monitor;
  //ld.ldConvertTags = ConvertTags;
  //ld.ldAskItemID = AskItemID;
  ld.ldFlags = lflags | loDF_IGNCASE;
  ld.ldBranchSep = '.'; /* Hierarchial branch separator */

  ecode = loServiceCreate(&our_service, &ld, tTotal);
  UL_TRACE((LOGID, "%!e loServiceCreate()=", ecode));
  if (ecode) return -1;

  InitializeCriticalSection(&lk_values);
  if (init_tags()) return -1;

  return 0;
}

void driver_destroy(void)
{
  if (our_service)
    {
      int ecode = loServiceDestroy(our_service);
      UL_INFO((LOGID, "%!e loServiceDestroy(%p) = ", ecode));

      for(ecode = 0; ecode < tTotal; ecode++)
        VariantClear(&tv[ecode].tvValue);

      for(ecode = 0; ecode < tTotal; ecode++)
	delete[] tn[ecode];

      delete[] ti;
      delete[] tn;
      delete[] tv;


      DeleteCriticalSection(&lk_values);

      our_service = 0;
    }
}

int show_error(LPCTSTR msg)
{
  fprintf(stderr, "%s", msg);
  fflush(stderr);
  return 1;
}

int show_msg(LPCTSTR msg)
{
  fprintf(stderr, "%s", msg);
  fflush(stderr);
  return 1;
}
