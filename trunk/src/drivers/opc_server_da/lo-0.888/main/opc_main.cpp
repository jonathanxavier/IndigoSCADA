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

#define _WIN32_DCOM					// Enables DCOM extensions
#define INITGUID					// Initialize OLE constants
#define ECL_SID "Enscada OPC server"// identificator of OPC server
#define PROG_ID "OPCServer.Enscada" //OPC Server ProgID

#include <stdio.h>
#include <math.h>				// some mathematical function
#include "variable.h"			// server variable
#include "unilog.h"				// universal utilites for creating log-files
#include <locale.h>				// set russian codepage
#include "opcda.h"				// basic function for OPC DA
#include "lightopc.h"			// light OPC library header file

static const loVendorInfo vendor = {1, 0, 3, 0, ECL_SID};	// OPC vendor info (Major/Minor/Build/Reserv)
static void a_server_finished(VOID*, loService*, loClient*);	// OnServer finish his work
static int OPCstatus = OPC_STATUS_RUNNING;						// status of OPC server
loService *our_service;											// name of light OPC Service

UINT casduNum = 0;					// main casdu nums
UINT tag_num = 0;					// tags counter
UINT tTotal = 0;					// total quantity of tags
UINT updated_item = 0;
UINT pos = 0;

DataEN DEN[MAX_CASDU_NUM][TAGS_NUM_MAX];			// data

unilog	*logg = NULL;				// new structure of unilog
FILE	*CfgFile;				// pointer to .ini file

VOID poll_device();	// poll device func
INT	 init_tags(UINT nCasdu);		// Init tags
UINT InitDriver();				// func of initialising port and creating service
UINT DestroyDriver();			// function of detroying driver and service

static CRITICAL_SECTION lk_values;	// protects ti[] from simultaneous access 
static INT opc_server_main(HINSTANCE hInstance, INT argc, CHAR *argv[]);
static INT show_error(LPCTSTR msg);		// just show messagebox with error
static INT show_msg(LPCTSTR msg);		// just show messagebox with message
CHAR* ReadIni (CHAR *SectionName,CHAR *Value);	// read parametr from .ini file

WCHAR wargv0[FILENAME_MAX + 32];		// lenght of command line (file+path (260+32))
CHAR argv0[FILENAME_MAX + 32];			// lenght of command line (file+path (260+32))

static CHAR *tn[TAGS_NUM_MAX];		// Tag name
static loTagValue tv[TAGS_NUM_MAX];	// Tag value
static loTagId ti[TAGS_NUM_MAX];	// Tag id

//-OPC Server CLSID

// {78BB6572-B8BA-47f9-83B3-4EC99DF9B1A3}
static const GUID GID_EnscadaOPCserverExe = 
{ 0x78bb6572, 0xb8ba, 0x47f9, { 0x83, 0xb3, 0x4e, 0xc9, 0x9d, 0xf9, 0xb1, 0xa3 } };

VOID timetToFileTime( time_t t, LPFILETIME pft )
{   
	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD) ll;
    pft->dwHighDateTime =  (ULONG)(ll >>32);
}

CHAR *absPath(CHAR *fileName)					// return abs path of file
{ 
	  static char path[sizeof(argv0)]="\0";			// path - massive of comline
	  CHAR *cp;
	  if(*path=='\0') strcpy(path, argv0);
	  if(NULL==(cp=strrchr(path,'\\'))) cp=path; else cp++;
	  strcpy(cp, fileName);
	  return path;
}

inline void init_common(void)		// create log-file
{ 
	logg = unilog_Create(ECL_SID, LOG_FNAME, NULL, 0, ll_DEBUG); // level [ll_FATAL...ll_DEBUG] 
	UL_INFO((LOGID, "Enscada OPC server start"));
}

inline void cleanup_common(void)	// delete log-file
{ 
	UL_INFO((LOGID, "Finish"));
	unilog_Delete(logg); logg = NULL;
	UL_INFO((LOGID, "total Finish"));
}

INT show_error(LPCTSTR msg)
{ 
	fprintf(stderr,"%s\n", msg);
	fflush(stderr);
	return 1;
}

INT show_msg(LPCTSTR msg)
{ 
	fprintf(stderr,"%s\n", msg);
	fflush(stderr);
	return 1;
}

inline void cleanup_all(DWORD objid)
{ 
  // Informs OLE that a class object, previously registered is no longer available for use  
  if (FAILED(CoRevokeClassObject(objid)))  UL_WARNING((LOGID, "CoRevokeClassObject() failed..."));
  DestroyDriver();					// close port and destroy driver
  CoUninitialize();					// Closes the COM library on the current thread
  cleanup_common();					// delete log-file
}

#include "our_ClassFactory.h"

INT APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,INT nCmdShow)
{ 
  static char *argv[3] = {"dummy.exe", NULL, NULL};	// defaults arguments
  argv[1] = lpCmdLine;									// comandline - progs keys
  return opc_server_main(hInstance, 2, argv);
}

INT main(INT argc, CHAR *argv[])
{  
	return opc_server_main(GetModuleHandle(NULL), argc, argv); 
}

INT opc_server_main(HINSTANCE hInstance, INT argc, CHAR *argv[]) 
{
	const char eClsidName [] = ECL_SID;				// desription 
	const char eProgID [] = PROG_ID;				// name
	CHAR *cp;

	DWORD objid=::GetModuleFileName(NULL, (char*)wargv0, sizeof(wargv0));	// function retrieves the fully qualified path for the specified module

	WideCharToMultiByte(CP_ACP, 0, wargv0, -1, argv0, 300, NULL, NULL);

	if(objid == 0 || objid + 50 > sizeof(argv0)) return 0;		// not in border

	init_common();									// create log-file

	UL_ERROR((LOGID, "system path [%s]", argv0));	// in bad case write error in log

	if((cp = setlocale(LC_ALL, ".1251")) == NULL)		// sets all categories, returning only the string cp-1251
	{ 
		UL_ERROR((LOGID, "setlocale() - Can't set 1251 code page"));	// in bad case write error in log
		cleanup_common();							// delete log-file
		return 0;
	}

	cp = argv[1];		

	if(cp)						// check keys of command line 
	{
		INT finish = 1;			// flag of comlection

		if(strstr(cp, "/r"))	// attempt registred server
		{
			 if(loServerRegister(&GID_EnscadaOPCserverExe, eProgID, eClsidName, argv0, 0)) 
			 { 
				 show_error((const char *)L"Registration Failed");  UL_ERROR((LOGID, "Registration <%s> <%s> Failed", eProgID, argv0));  
			 } 
			 else 
			 { 
				 show_msg((const char *)L"Enscada OPC Registration Ok"); UL_INFO((LOGID, "Registration <%s> <%s> Ok", eProgID, argv0));
			 }
		} 
		else if(strstr(cp, "/u")) 
		{
			 if(loServerUnregister(&GID_EnscadaOPCserverExe, eClsidName)) 
			 { 
				 show_error((const char *)L"UnRegistration Failed"); UL_ERROR((LOGID, "UnReg <%s> <%s> Failed", eClsidName, argv0)); 
			 } 
			 else 
			 { 
				 show_msg((const char *)L"Enscada OPC Server Unregistered"); UL_INFO((LOGID, "UnReg <%s> <%s> Ok", eClsidName, argv0));		
			 }
		} 
		else if(strstr(cp, "/?")) 
		{
			show_msg((const char *)L"Use: \nKey /r to register server.\nKey /u to unregister server.\nKey /? to show this help.");
		}
		else
		{
			 UL_WARNING((LOGID, "Ignore unknown option <%s>", cp));
			 finish = 0;		// nehren delat
		}

		if(finish) 
		{      
			cleanup_common();      
			return 0;    
		} 
	}

	if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) 
	{	
		// Initializes the COM library for use by the calling thread
		 UL_ERROR((LOGID, "CoInitializeEx() failed. Exiting..."));
		 cleanup_common();	// close log-file
		 return 0;
	}

	UL_INFO((LOGID, "CoInitializeEx() [ok]"));	// write to log

	if(InitDriver()) 
	{
		CoUninitialize();	// Closes the COM library on the current thread
		cleanup_common();	// close log-file
		return 0;
	}

	UL_INFO((LOGID, "InitDriver() [ok]"));	// write to log

	if(FAILED(CoRegisterClassObject(GID_EnscadaOPCserverExe, &our_CF, 
				   CLSCTX_LOCAL_SERVER|CLSCTX_REMOTE_SERVER|CLSCTX_INPROC_SERVER, 
				   REGCLS_MULTIPLEUSE, &objid)))
    { 
		UL_ERROR((LOGID, "CoRegisterClassObject() [failed]"));
		cleanup_all(objid);		// close socket and unload all librares
		return 0; 
	}

	UL_INFO((LOGID, "CoRegisterClassObject() [ok]"));	// write to log

	Sleep(1000);
	our_CF.Release();		// avoid locking by CoRegisterClassObject() 

	if(OPCstatus != OPC_STATUS_RUNNING)	
	{	
		while(our_CF.in_use())
		{
			Sleep(1000);	// wait
		}

		UL_INFO((LOGID, "cleanup_all()"));	// write to log
		cleanup_all(objid);
		return 0;	
	}

	while(our_CF.in_use())	// while server created or client connected
	{
		poll_device();		// polling devices else do nothing (and be nothing)	 
	}

	UL_INFO((LOGID, "end cleanup_all()"));	// write to log

	cleanup_all(objid);

	return 0;
}

loTrid ReadTags(const loCaller *, unsigned  count, loTagPair taglist[],
		VARIANT   values[],	WORD      qualities[],	FILETIME  stamps[],
		HRESULT   errs[],	HRESULT  *master_err,	HRESULT  *master_qual,
		const VARTYPE vtype[],	LCID lcid)
{  
	return loDR_STORED;
}


INT WriteTags(const loCaller *ca,
              unsigned count, loTagPair taglist[],
              VARIANT values[], HRESULT error[], HRESULT *master, LCID lcid)
{  
	return loDW_TOCACHE; 
}

/* OPTIONAL: show client's interests */

VOID activation_monitor(const loCaller *ca, INT count, loTagPair *til)
{
  int act = 1;

  if(0 > count)
  {
    act = 0, count = -count;
  }

  while(count--)
  {
      UL_DEBUG((LOGID, "MON: %u %s %s", til[count].tpTi, tn[(int) til[count].tpRt], act ? "On" : "Off"));
  }
}

UINT InitDriver()
{
	 UL_INFO((LOGID, "InitDriver()"));
	 loDriver ld;		// driver description
	 LONG ecode;		// error code 
	 tTotal = TAGS_NUM_MAX;		// total tag quantity

	 if(our_service)
	 {
		  UL_ERROR((LOGID, "Driver already initialized!"));
		  return 0;
	 }

	 memset(&ld, 0, sizeof(ld));   
	 ld.ldRefreshRate = 5000;		// polling time 
	 ld.ldRefreshRate_min = 4000;	// minimum polling time
	 ld.ldWriteTags = WriteTags;	// pointer to function write tag
	 ld.ldReadTags = ReadTags;		// pointer to function read tag
	 ld.ldSubscribe = activation_monitor;	// callback of tag activity
	 ld.ldFlags = loDF_IGNCASE;				// ignore case
	 ld.ldBranchSep = '/';					// hierarchial branch separator

	 ecode = loServiceCreate(&our_service, &ld, tTotal);		//	creating loService 

	 UL_TRACE((LOGID, "%!e loServiceCreate()=", ecode));	// write to log returning code
	 if (ecode) return 1;									// error to create service	
	 InitializeCriticalSection(&lk_values);
	 
	 //UL_INFO((LOGID, "Start polling data from real time server"));

	 casduNum = 1;

	 tag_num = 10; //TODO: Initialize with the number of items in the server

	 init_tags(casduNum);

	 return 0;
}

INT init_tags(UINT nCasdu)
{
  UL_INFO((LOGID, "init_tags(%d)", updated_item));
  FILETIME ft;		// 64-bit value representing the number of 100-ns intervals since January 1,1601
  UINT rights = 0;	// tag type (read/write)
  UINT ecode;
  BOOL arch = FALSE;
  GetSystemTimeAsFileTime(&ft);	// retrieves the current system date and time
  UINT tag_add = 0;					// tags counter
  
  EnterCriticalSection(&lk_values);

  if(!tag_num) return 0;

    for(UINT i = 0; i < tag_num; i++)
	{
		rights = 0;
		rights = rights | OPC_READABLE;
		tn[tag_add] = new char[DATALEN_MAX];

		//strncpy(tn[tag_add], <<item id name from cfg file>>, 100);

		VariantInit(&tv[tag_add].tvValue);

		WCHAR buf[DATALEN_MAX];

		LCID lcid = MAKELCID(0x0409, SORT_DEFAULT); // This macro creates a locale identifier from a language identifier. Specifies how dates, times, and currencies are formatted
		MultiByteToWideChar(CP_ACP, 0, tn[tag_add], strlen(tn[tag_add])+1,	buf, sizeof(buf)/sizeof(buf[0])); // function maps a character string to a wide-character (Unicode) string

		V_R4(&tv[tag_add].tvValue) = 0.0;
		V_VT(&tv[tag_add].tvValue) = VT_R4;

		//UL_ERROR((LOGID, "DEN[i].type = %d",DEN[i].type));
		ecode = loAddRealTag_aW(our_service, &ti[tag_add], (loRealTag)(tag_add+1), buf, 0, rights, &tv[tag_add].tvValue, 0, 0); 

		tv[tag_add].tvTi = ti[tag_add];
		tv[tag_add].tvState.tsTime = ft;
		tv[tag_add].tvState.tsError = S_OK;
		tv[tag_add].tvState.tsQuality = OPC_QUALITY_NOT_CONNECTED;

		DEN[nCasdu][i].ti = ti[tag_add];

		UL_TRACE((LOGID, "%!e loAddRealTag(%s) = %u [%d]", ecode, tn[tag_add], ti[tag_add], tag_add));

		tag_add++;
	}

	LeaveCriticalSection(&lk_values);

	if(ecode) 
	{
		UL_ERROR((LOGID, "%!e driver_init()=", ecode));
		return -1;
	}

	return 0;
}

VOID poll_device()
{
	FILETIME ft;
	INT ecode = 0; DWORD start = 0;
	UL_DEBUG((LOGID, "poll_device ()"));
	CHAR value[20][50]; //Remove ASAP the value string array
	int ti_index;
		 
	for(UINT i = 0; i < casduNum; i++) 
	{
		 updated_item = 0;

		 //Get data from real time database

		 if(value[0] && atoi(value[0])>0)
		 { // 
			 if (value[0]) DEN[i][updated_item].device = atoi(value[0]);
			 else DEN[i][updated_item].device=0;

			 if (value[1]) DEN[i][updated_item].prm = atoi(value[1]);
			 else DEN[i][updated_item].prm=0;

			 sprintf(DEN[i][updated_item].name,"[0x%x]",DEN[i][updated_item].device);

			 if (value[2]) sprintf(DEN[i][updated_item].value,"%s",value[2]);
			 else strcpy(DEN[i][updated_item].value,"0");

			 for (UINT t=0; t<=strlen(DEN[i][updated_item].value); t++) 
				 if (DEN[i][updated_item].value[t]=='.') 
					 DEN[i][updated_item].value[t]=',';

			 if (value[3]) DEN[i][updated_item].status = atoi(value[3]);
			 else DEN[i][updated_item].status=0;

			 if (value[4]) DEN[i][updated_item].pipe = atoi(value[4]);
			 else DEN[i][updated_item].pipe=0;

			 //UL_INFO((LOGID,"[%d] (0x%x)[%d] %s(%d)",updated_item,DEN[i][updated_item].device,DEN[i][updated_item].prm,DEN[i][updated_item].name),DEN[i][updated_item].pipe));
			 if (DEN[i][updated_item].device>0) updated_item++;
		 }

		 EnterCriticalSection(&lk_values);
	 
		for(UINT ci = 0; ci < updated_item; ci++)
		{
			 ti_index = DEN[i][ci].ti;

			 GetSystemTimeAsFileTime(&ft);
			 //UL_DEBUG((LOGID, "[%d][%d] ci = %d | v = %s (%d)",i,ti_index,ci,DEN[i][ci].value,DEN[i][ci].status));

			 VARTYPE tvVt = tv[ti_index].tvValue.vt;
			 //UL_DEBUG((LOGID, "[%d][%d] %d (%f)",ti_index,tvVt,tv[ti_index].tvValue.vt,atof(DEN[i][ci].value)));

			 VariantClear(&tv[ti_index].tvValue);
			 //CHAR   *stopstring;

			 V_R4(&tv[ti_index].tvValue) = (float)atof(DEN[i][ci].value);//(FLOAT) strtod(DEN[i][ci].value, &stopstring);
			 V_VT(&tv[ti_index].tvValue) = tvVt;

			 if (!DEN[i][ci].status) 
				 tv[ti_index].tvState.tsQuality = OPC_QUALITY_GOOD;
			 else 
				 tv[ti_index].tvState.tsQuality = OPC_QUALITY_UNCERTAIN;

			 //if (DEN[i][ci].status>0) tv[ti_index].tvState.tsQuality = OPC_QUALITY_CONFIG_ERROR;
			 tv[ti_index].tvState.tsTime = ft;
		}

		LeaveCriticalSection(&lk_values);
		//Sleep(100);
	}
	

  /** MANDATORY: send all the values into the cache: */
  loCacheUpdate(our_service, tag_num, tv, 0);
  Sleep(1000);
}

UINT DestroyDriver()
{
	if(our_service)		
    {
      INT ecode = loServiceDestroy(our_service);
      UL_INFO((LOGID, "%!e loServiceDestroy(%p) = ", ecode));	// destroy derver
      DeleteCriticalSection(&lk_values);						// destroy CS
      our_service = 0;		
    }
	return	1;
}

CHAR* ReadIni(CHAR *SectionName,CHAR *Value)
{
	CHAR buf[150]; CHAR string1[50];
	CHAR *pbuf=buf;
	UINT s_ok=0;
	CfgFile = fopen(CFG_FILE, "r+");

	if(CfgFile)	
	{	
		UL_ERROR((LOGID, "Error open .ini file"));	// in bad case write error in log
		return pbuf;
	}

	sprintf(string1,"[%s]",SectionName);
	rewind (CfgFile);

	  while(!feof(CfgFile))
		 if(fgets(buf,50,CfgFile)!=NULL)
			if (strstr(buf,string1))
				{ s_ok=1; break; }
	if (s_ok)
	{
		while(!feof(CfgFile))
		{
			if(fgets(buf,100,CfgFile)!=NULL&&strstr(buf,"[")==NULL&&strstr(buf,"]")==NULL)
			{
			 for (s_ok=0;s_ok<strlen(buf)-1;s_ok++)
				 if (buf[s_ok]==';') buf[s_ok+1]='\0';
				if (strstr(buf,Value))
				{
					for (s_ok=0;s_ok<strlen(buf)-1;s_ok++)
						if (s_ok>strlen(Value)) buf[s_ok-strlen(Value)-1]=buf[s_ok];
							 buf[s_ok-strlen(Value)-1]='\0';
						//UL_INFO((LOGID, "Section name %s, value %s, che %s",SectionName,Value,buf));	// write in log
						fclose(CfgFile);
						return pbuf;
				}
			}
		}

 		fclose(CfgFile);
		return pbuf;
	}
	else
	{
		sprintf(buf, "error");			// if something go wrong return error
		fclose(CfgFile);
		return pbuf;
	}	
}
