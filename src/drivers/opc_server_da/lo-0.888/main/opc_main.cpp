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
#define ECL_SID "opc.enscada"		// identificator of OPC server
#define _CRT_SECURE_NO_WARNINGS 1 

#define COMM_BUFFER_SIZE	500
#define SMALL_BUFFER_SIZE	30

#include <stdio.h>
#include <math.h>				// some mathematical function
#include "variable.h"			// server variable
#include "unilog.h"				// universal utilites for creating log-files
#include <locale.h>				// set russian codepage
#include "opcda.h"				// basic function for OPC:DA
#include "lightopc.h"			// light OPC library header file

static const loVendorInfo vendor = {1,0,3,0,"Enscada OPC Server" };	// OPC vendor info (Major/Minor/Build/Reserv)
static void a_server_finished(VOID*, loService*, loClient*);	// OnServer finish his work
static int OPCstatus=OPC_STATUS_RUNNING;						// status of OPC server
loService *our_service;											// name of light OPC Service


CHAR	ip_contr[MAX_ENSCADA_NUM][16];	// controllers ip
UINT	ip_sock[MAX_ENSCADA_NUM];

UINT IP_number=0;
UINT devNum=0;					// main device nums
UINT tag_num=0;					// tags counter
UINT tag_add=0;					// tags counter
UINT prm_num=0;					// prm counter
UINT tTotal=0;					// total quantity of tags
UINT flats_num=0,dev_num=0;
BOOL WorkEnable=TRUE;
UINT pos=0;


DataEN DEN[MAX_ENSCADA_NUM][TAGS_NUM_MAX];			// data
FlatEN FEN[MAX_ENSCADA_NUM][TAGS_NUM_MAX];			// data
PrmEN  PEN[MAX_ENSCADA_NUM][TAGS_NUM_MAX];			// data

unilog	*logg=NULL;				// new structure of unilog
FILE	*CfgFile;				// pointer to .ini file


UINT ScanBus();					// bus scanned programm
VOID poll_device();	// poll device func
INT	 init_tags(UINT nSock);		// Init tags
UINT InitDriver();				// func of initialising port and creating service
UINT DestroyDriver();			// function of detroying driver and service


static CRITICAL_SECTION lk_values;	// protects ti[] from simultaneous access 
static INT opc_server_main(HINSTANCE hInstance, INT argc, CHAR *argv[]);
static INT show_error(LPCTSTR msg);		// just show messagebox with error
static INT show_msg(LPCTSTR msg);		// just show messagebox with message
CHAR* ReadParam (CHAR *SectionName,CHAR *Value);	// read parametr from .ini file


WCHAR wargv0[FILENAME_MAX + 32];		// lenght of command line (file+path (260+32))
CHAR argv0[FILENAME_MAX + 32];			// lenght of command line (file+path (260+32))
static CHAR *tn[TAGS_NUM_MAX];		// Tag name
static loTagValue tv[TAGS_NUM_MAX];	// Tag value
static loTagId ti[TAGS_NUM_MAX];	// Tag id

// {7C8D5D34-297F-4b56-B01A-9A84EF59AC2E}
static const GUID GID_EnscadaOPCserverDll = 
{ 0x7c8d5d34, 0x297f, 0x4b56, { 0xb0, 0x1a, 0x9a, 0x84, 0xef, 0x59, 0xac, 0x2e } };

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
{ // Informs OLE that a class object, previously registered is no longer available for use  
  if (FAILED(CoRevokeClassObject(objid)))  UL_WARNING((LOGID, "CoRevokeClassObject() failed..."));
  DestroyDriver();					// close port and destroy driver
  CoUninitialize();					// Closes the COM library on the current thread
  cleanup_common();					// delete log-file
}

#include "our_ClassFactory.h"

INT APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,INT nCmdShow)
{ 
  static char *argv[3] = { "dummy.exe", NULL, NULL };	// defaults arguments
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
	const char eProgID [] = ECL_SID;				// name
	CHAR *cp;
	DWORD objid=::GetModuleFileName(NULL, (char*)wargv0, sizeof(wargv0));	// function retrieves the fully qualified path for the specified module
	WideCharToMultiByte(CP_ACP,0,wargv0,-1,argv0,300,NULL, NULL);
	if(objid==0 || objid+50 > sizeof(argv0)) return 0;		// not in border
	init_common();									// create log-file
	UL_ERROR((LOGID, "system path [%s]",argv0));	// in bad case write error in log

	if(NULL==(cp = setlocale(LC_ALL, ".1251")))		// sets all categories, returning only the string cp-1251
	{ 
		UL_ERROR((LOGID, "setlocale() - Can't set 1251 code page"));	// in bad case write error in log
		cleanup_common();							// delete log-file
		return 0;
	}

	cp = argv[1];		

	if(cp)						// check keys of command line 
	{
		INT finish = 1;			// flag of comlection
		if(strstr(cp, "/r"))	//	attempt registred server
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
		else 
		if (strstr(cp, "/u")) 
		{
			 if (loServerUnregister(&GID_EnscadaOPCserverExe, eClsidName)) 
				{ show_error((const char *)L"UnRegistration Failed"); UL_ERROR((LOGID, "UnReg <%s> <%s> Failed", eClsidName, argv0)); } 
			 else 
				{ show_msg((const char *)L"Enscada OPC Server Unregistered"); UL_INFO((LOGID, "UnReg <%s> <%s> Ok", eClsidName, argv0));		}
		} 
		else  // only /r and /u options
			if (strstr(cp, "/?")) 
				 show_msg((const char *)L"Use: \nKey /r to register server.\nKey /u to unregister server.\nKey /? to show this help.");
				 else
				{
					 UL_WARNING((LOGID, "Ignore unknown option <%s>", cp));
					 finish = 0;		// nehren delat
				}

		if (finish) 
		{      
			cleanup_common();      
			return 0;    
		} 
	}

	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) 
	{	// Initializes the COM library for use by the calling thread
		 UL_ERROR((LOGID, "CoInitializeEx() failed. Exiting..."));
		 cleanup_common();	// close log-file
		 return 0;
	}

	UL_INFO((LOGID, "CoInitializeEx() [ok]"));	// write to log

	if (InitDriver()) 
	{		// open sockets and attempt connect to servers
		CoUninitialize();	// Closes the COM library on the current thread
		cleanup_common();	// close log-file
		return 0;
	}

	UL_INFO((LOGID, "InitDriver() [ok]"));	// write to log

	if (FAILED(CoRegisterClassObject(GID_EnscadaOPCserverExe, &our_CF, 
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
			Sleep(1000);	// wait

		cleanup_all(objid);
		return 0;	
	}

	while(our_CF.in_use())	// while server created or client connected
	{
		if(WorkEnable)
			poll_device();		// polling devices else do nothing (and be nothing)	 
	}

	UL_INFO((LOGID, "end cleanup_all()"));	// write to log
	cleanup_all(objid);		// destroy himself
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


VOID activation_monitor(const loCaller *ca, INT count, loTagPair *til){}

UINT InitDriver()
{
	 UL_INFO((LOGID, "InitDriver()"));
	 loDriver ld;		// structure of driver description
	 LONG ecode;		// error code 
	 tTotal = TAGS_NUM_MAX;		// total tag quantity

	 if (our_service) 
	 {
		  UL_ERROR((LOGID, "Driver already initialized!"));
		  return 0;
	 }

	 memset(&ld, 0, sizeof(ld));   
	 ld.ldRefreshRate =5000;		// polling time 
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
	 // -------------------------------------------------------------------------------------------------
	 UL_INFO((LOGID, "attempt to read controller information (%d)",IP_number));
	 if (ScanBus()) { UL_INFO((LOGID, "total %d devices found",devNum)); return 0; }
	 else			{ UL_ERROR((LOGID, "no devices found")); 	  return 1; }
}


UINT ScanBus()
{
	devNum=0;
/*
 devNum=0; tag_num=0; tag_add=0; UINT ecode=0;
 UL_INFO((LOGID, "scanBus started (%d)",IP_number));
 DWORD start=GetTickCount();
 // EtherNet
 for (UINT i=0; i<IP_number; i++) 
 {
	 flats_num=dev_num=prm_num=0;
	 hConnection (i, ip_sock[i], server_socket[i], http_cln_socket[i]);
	 HandleHTTPRequest ("req=flats_info",i,1);
	 hConnection (i, ip_sock[i], server_socket[i], http_cln_socket[i]);
	 HandleHTTPRequest ("req=dev_info",i,2);

	 hConnection (i, ip_sock[i], server_socket[i], http_cln_socket[i]);
	 HandleHTTPRequest ("req=list_dev",i,3);
	 hConnection (i, ip_sock[i], server_socket[i], http_cln_socket[i]);
	 HandleHTTPRequest ("req=flats_info",i,4);
	 hConnection (i, ip_sock[i], server_socket[i], http_cln_socket[i]);
	 HandleHTTPRequest ("req=prm_info",i,5);

	 tag_num+=flats_num+dev_num;
	 if (flats_num+dev_num>10) 
	 {
		 devNum++;
		 WorkEnable=TRUE;
		 init_tags(i);
	 }
	 //closesocket(server_socket[i]);
  }

	UL_INFO((LOGID, "Total %d devices",devNum));
*/
	return devNum;
}


VOID poll_device()
{
	/*
	FILETIME ft;
	INT ecode=0; DWORD start=0;
	UL_DEBUG((LOGID, "poll_device ()"));
 
	for (UINT i=0; i<devNum; i++) 
	{
	 flats_num=dev_num=0;
	 hConnection (i, ip_sock[i], server_socket[i], http_cln_socket[i]);
	 HandleHTTPRequest ("req=flats_info",i,1);
	 hConnection (i, ip_sock[i], server_socket[i], http_cln_socket[i]);
	 HandleHTTPRequest ("req=dev_info",i,2);
	 UL_DEBUG((LOGID, "Data to tag (%d)",tag_num));

	 EnterCriticalSection(&lk_values);
	 for (UINT ci=0; ci<flats_num; ci++)
		{
		 //UL_DEBUG((LOGID, "[%d][%d] ci = %d | v = %s (%d)",i,FEN[i][ci].tn,ci,FEN[i][ci].value,FEN[i][ci].status));
		 GetSystemTimeAsFileTime(&ft);
		 VARTYPE tvVt = tv[FEN[i][ci].tn].tvValue.vt;
		 VariantClear(&tv[FEN[i][ci].tn].tvValue);
		 //CHAR   *stopstring;
		 //V_R4(&tv[FEN[i][ci].tn].tvValue) = (FLOAT) strtod(FEN[i][ci].value, &stopstring);
		 V_R4(&tv[FEN[i][ci].tn].tvValue) = (float)atof(FEN[i][ci].value);
		 V_VT(&tv[FEN[i][ci].tn].tvValue) = tvVt;
		 if (!FEN[i][ci].status) tv[FEN[i][ci].tn].tvState.tsQuality = OPC_QUALITY_GOOD;
		 else tv[FEN[i][ci].tn].tvState.tsQuality = OPC_QUALITY_UNCERTAIN;
		 //if (FEN[i][ci].status>0) tv[FEN[i][ci].tn].tvState.tsQuality = OPC_QUALITY_CONFIG_ERROR;
		 tv[FEN[i][ci].tn].tvState.tsTime = ft;
		}
	 
		for(ci=0; ci<dev_num; ci++)
		{
		 GetSystemTimeAsFileTime(&ft);
		 //UL_DEBUG((LOGID, "[%d][%d] ci = %d | v = %s (%d)",i,DEN[i][ci].tn,ci,DEN[i][ci].value,DEN[i][ci].status));
		 VARTYPE tvVt = tv[DEN[i][ci].tn].tvValue.vt;
		 //UL_DEBUG((LOGID, "[%d][%d] %d (%f)",DEN[i][ci].tn,tvVt,tv[DEN[i][ci].tn].tvValue.vt,atof(DEN[i][ci].value)));
		 VariantClear(&tv[DEN[i][ci].tn].tvValue);	  
		 //CHAR   *stopstring;
		 V_R4(&tv[DEN[i][ci].tn].tvValue) = (float)atof(DEN[i][ci].value);//(FLOAT) strtod(DEN[i][ci].value, &stopstring);
		 V_VT(&tv[DEN[i][ci].tn].tvValue) = tvVt;
		 if (!DEN[i][ci].status) tv[DEN[i][ci].tn].tvState.tsQuality = OPC_QUALITY_GOOD;
		 else tv[DEN[i][ci].tn].tvState.tsQuality = OPC_QUALITY_UNCERTAIN;
		 //if (DEN[i][ci].status>0) tv[DEN[i][ci].tn].tvState.tsQuality = OPC_QUALITY_CONFIG_ERROR;
		 tv[DEN[i][ci].tn].tvState.tsTime = ft;
		}
	 LeaveCriticalSection(&lk_values);
	 Sleep(100);
	}
  */

  /** MANDATORY: send all the values into the cache: */
  loCacheUpdate(our_service, tag_num, tv, 0);
  Sleep(1000);

}


UINT DestroyDriver()
{
	if (our_service)		
    {
      INT ecode = loServiceDestroy(our_service);
      UL_INFO((LOGID, "%!e loServiceDestroy(%p) = ", ecode));	// destroy derver
      DeleteCriticalSection(&lk_values);						// destroy CS
      our_service = 0;		
    }
	return	1;
}


CHAR* ReadParam (CHAR *SectionName,CHAR *Value)
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


INT init_tags(UINT nSock)
{
  UL_INFO((LOGID, "init_tags(%d)",flats_num+dev_num));
  FILETIME ft;		// 64-bit value representing the number of 100-ns intervals since January 1,1601
  UINT rights=0;	// tag type (read/write)
  UINT ecode;
  BOOL arch=FALSE;
  GetSystemTimeAsFileTime(&ft);	// retrieves the current system date and time
  EnterCriticalSection(&lk_values);

  if(!tag_num) return 0;

  for(UINT i = 0; i < flats_num; i++)
  {
	 rights=0;
	 rights = rights | OPC_READABLE;
	 tn[tag_add] = new char[DATALEN_MAX];	// reserve memory for massive

	 for (UINT j=0; j<=prm_num; j++)
	 {
		 if (FEN[nSock][i].prm==PEN[nSock][j].prm)
		 {
				sprintf(tn[tag_add],"%s/flats/%s/%s",ip_contr[nSock], FEN[nSock][i].name, PEN[nSock][j].name);
				break;
		 }
		 else sprintf(tn[tag_add],"%s/flats/%s/parametr %d",ip_contr[nSock], FEN[nSock][i].name, FEN[nSock][i].prm);
	 }

	 strncpy (tn[tag_add],tn[tag_add],100);
	 VariantInit(&tv[tag_add].tvValue);
	 WCHAR buf[DATALEN_MAX];
	 LCID lcid = MAKELCID(0x0409, SORT_DEFAULT); // This macro creates a locale identifier from a language identifier. Specifies how dates, times, and currencies are formatted
 	 MultiByteToWideChar(CP_ACP, 0,tn[tag_add], strlen(tn[tag_add])+1,	buf, sizeof(buf)/sizeof(buf[0])); // function maps a character string to a wide-character (Unicode) string
	 V_R4(&tv[tag_add].tvValue) = 0.0;
	 V_VT(&tv[tag_add].tvValue) = VT_R4;
	 //UL_ERROR((LOGID, "DEN[i].type = %d",DEN[i].type));
	 ecode = loAddRealTag_aW(our_service, &ti[tag_add], (loRealTag)(tag_add+1), buf, 0, rights, &tv[tag_add].tvValue, 0, 0);
	 tv[tag_add].tvTi = ti[tag_add];
	 FEN[nSock][i].tn=ti[tag_add];
	 tv[tag_add].tvState.tsTime = ft;
	 tv[tag_add].tvState.tsError = S_OK;
	 tv[tag_add].tvState.tsQuality = OPC_QUALITY_NOT_CONNECTED;
	 //FEN[nSock][tag_add].tn=ti[tag_add];
	 UL_TRACE((LOGID, "%!e loAddRealTag(%s) = %u [%d]", ecode, tn[tag_add], ti[tag_add], tag_add));
	 tag_add++;
  }

    for(i = 0; i < dev_num; i++)
	{
	 rights=0;
	 rights = rights | OPC_READABLE;
	 tn[tag_add] = new char[DATALEN_MAX];	// reserve memory for massive

	 //for (UINT j=0; j<=prm_num; j++) UL_TRACE((LOGID, "%d (%s)", PEN[nSock][j].prm, PEN[nSock][j].name));
	 for (UINT j=0; j<=prm_num; j++)
		 if (DEN[nSock][i].prm==PEN[nSock][j].prm)
		 {
			 sprintf(tn[tag_add],"%s/devices/%s/%s (%d)",ip_contr[nSock], DEN[nSock][i].name, PEN[nSock][j].name, DEN[nSock][i].pipe); 
			 break;
		 }
		 else sprintf(tn[tag_add],"%s/devices/%s/parametr %d (%d)",ip_contr[nSock], DEN[nSock][i].name, DEN[nSock][i].prm, DEN[nSock][i].pipe); 

	 strncpy (tn[tag_add],tn[tag_add],100);
	 VariantInit(&tv[tag_add].tvValue);
	 WCHAR buf[DATALEN_MAX];
	 LCID lcid = MAKELCID(0x0409, SORT_DEFAULT); // This macro creates a locale identifier from a language identifier. Specifies how dates, times, and currencies are formatted
 	 MultiByteToWideChar(CP_ACP, 0,tn[tag_add], strlen(tn[tag_add])+1,	buf, sizeof(buf)/sizeof(buf[0])); // function maps a character string to a wide-character (Unicode) string
	 V_R4(&tv[tag_add].tvValue) = 0.0;
	 V_VT(&tv[tag_add].tvValue) = VT_R4;
	 //UL_ERROR((LOGID, "DEN[i].type = %d",DEN[i].type));
	 ecode = loAddRealTag_aW(our_service, &ti[tag_add], (loRealTag)(tag_add+1), buf, 0, rights, &tv[tag_add].tvValue, 0, 0); 
	 tv[tag_add].tvTi = ti[tag_add];
	 tv[tag_add].tvState.tsTime = ft;
	 tv[tag_add].tvState.tsError = S_OK;
	 tv[tag_add].tvState.tsQuality = OPC_QUALITY_NOT_CONNECTED;
	 DEN[nSock][i].tn=ti[tag_add];
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

/*

UINT GetInput (CHAR* buf, UINT nSock, UINT type)
{
 UINT pos2=0,ps=0;
 CHAR *token;
 CHAR value[20][50];
 CHAR data[TAGS_NUM_MAX][250];
 //UL_INFO((LOGID,"buf (%s)",buf));
 token = strtok(buf,"\n");

 while(token != NULL)
	{
	 sprintf(data[pos2],token);
	 //UL_INFO((LOGID,"data[%d] (%s)",pos2,data[pos2]));
	 token = strtok(NULL,"\n");
	 if ((type==5 && strlen(data[pos2])>5) || strlen(data[pos2])>15) pos2++;
	}
 if (type==5) pos2++;
 //UL_INFO((LOGID,"pos=[%d]",pos2));
 if (pos2>4)
 for (UINT i=pos; i<pos+pos2-1; i++)
 if (strlen(data[i-pos])>5)
	{
	 ps=0;
	 //if (i<4) i+=3;
	 token = strtok(data[i-pos],",\n");
	 //UL_INFO((LOGID,"[%d/%d] (%s)",i,pos+pos2,data[i-pos]));
	 while(token!=NULL)
		{
		 strcpy (value[ps],token);
		 //UL_INFO((LOGID,"[%d]value[%d]=%s",i,ps,value[ps]));
		 token = strtok(NULL,",\n");
		 ps++;
		}

	 if (type==1) // dev_info
	 if (value[0] && atoi(value[0])>0)
		{ // 
		 if (value[0]) DEN[nSock][dev_num].device=atoi(value[0]);
		 else DEN[nSock][dev_num].device=0;
		 if (value[1]) DEN[nSock][dev_num].prm=atoi(value[1]);
		 else DEN[nSock][dev_num].prm=0;
		 sprintf(DEN[nSock][dev_num].name,"[0x%x]",DEN[nSock][dev_num].device);
		 if (value[2]) sprintf(DEN[nSock][dev_num].value,"%s",value[2]);
		 else strcpy(DEN[nSock][dev_num].value,"0");

		 for (UINT t=0; t<=strlen(DEN[nSock][dev_num].value); t++) if (DEN[nSock][dev_num].value[t]=='.') DEN[nSock][dev_num].value[t]=',';

		 if (value[3]) DEN[nSock][dev_num].status=atoi(value[3]);
		 else DEN[nSock][dev_num].status=0;
		 if (value[4]) DEN[nSock][dev_num].pipe=atoi(value[4]);
		 else DEN[nSock][dev_num].pipe=0;

		 //UL_INFO((LOGID,"[%d] (0x%x)[%d] %s(%d)",dev_num,DEN[nSock][dev_num].device,DEN[nSock][dev_num].prm,DEN[nSock][dev_num].name),DEN[nSock][dev_num].pipe));
		 if (DEN[nSock][dev_num].device>0) dev_num++;
		}
	 if (type==2) // flats_info
	 if (value[0] && atoi(value[0])>0)
		{ //
		 if (value[0]) FEN[nSock][flats_num].flat=atoi(value[0]);
		 else FEN[nSock][flats_num].flat=0;
		 if (value[1]) FEN[nSock][flats_num].prm=atoi(value[1]);
		 else FEN[nSock][flats_num].prm=0;
		 sprintf(FEN[nSock][flats_num].name,"n%d",FEN[nSock][flats_num].flat);
		 if (value[2]) sprintf(FEN[nSock][flats_num].value,"%s",value[2]);
		 else strcpy(FEN[nSock][flats_num].value,"0");
		 //UL_INFO((LOGID,"[%d] %d",flats_num,FEN[nSock][flats_num].tn));
		 for (UINT t=0; t<=strlen(FEN[nSock][flats_num].value); t++) if (FEN[nSock][flats_num].value[t]=='.') FEN[nSock][flats_num].value[t]=',';

		 if (value[3]) FEN[nSock][flats_num].status=atoi(value[3]);
		 else FEN[nSock][flats_num].status=0;
		 if (FEN[nSock][flats_num].flat>0) flats_num++;
		}

	 if (type==5) // prm_info
	 if (value[0] && atoi(value[0])>0)
		{ //
		 if (value[0]) PEN[nSock][prm_num].prm=atoi(value[0]);
		 else PEN[nSock][prm_num].prm=0;
		 if (value[1]) strcpy(PEN[nSock][prm_num].name,value[1]);
		 else strcpy(PEN[nSock][prm_num].name,"prm");
		 //UL_INFO((LOGID,"[%d] [%d] %s",prm_num,PEN[nSock][prm_num].prm,PEN[nSock][prm_num].name));
		 prm_num++;
		}
	  if (type==3)
	  for (UINT r=0; r<TAGS_NUM_MAX; r++)
	  if (DEN[nSock][r].device==(unsigned int)atoi(value[0])) 
		  if (value[14] && DEN[nSock][r].device>0) 
		  {		   
		   strcpy(DEN[nSock][r].name, value[14]);
		   if (strlen(value[15])>4) sprintf(DEN[nSock][r].name,"%s,%s",value[14],value[15]);
		   //UL_INFO((LOGID,"[%d/%d] (0x%x)[%d] %s",r,dev_num,DEN[nSock][r].device,DEN[nSock][r].prm,DEN[nSock][r].name));
		  }
	  if (type==4)
	  for (UINT r=0; r<TAGS_NUM_MAX; r++)
		  if (FEN[nSock][r].flat==(unsigned int)atoi(value[0])) 
			if (value[4] && FEN[nSock][r].flat>0)
			{
			 strcpy(FEN[nSock][r].name, value[4]);
			 //UL_INFO((LOGID,"[%d] (%d)[%d] %s",flats_num,FEN[nSock][r].flat,FEN[nSock][r].prm,FEN[nSock][r].name));
			}
	}
 return pos2;
}

*/