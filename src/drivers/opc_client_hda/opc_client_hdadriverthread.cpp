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
#include "opc_client_hda_instance.h"
#include "opc_client_hdadriverthread.h"
/*
*Function:run
*runs the thread
*Inputs:none
*Outputs:none
*Returns:none
*/

bool  Opc_client_hda_DriverThread::g_bWriteComplete = true;
bool  Opc_client_hda_DriverThread::g_bReadComplete = true;
DWORD Opc_client_hda_DriverThread::g_dwReadTransID = 1;
DWORD Opc_client_hda_DriverThread::g_dwWriteTransID = 2;
//FILE *Opc_client_hda_DriverThread::g_stream = NULL; // file log handle
IOPCHDA_Server *Opc_client_hda_DriverThread::g_pIOPCServer = NULL;

DriverInstance *Opc_client_hda_DriverThread::StaticParent = NULL;
DriverThread *Opc_client_hda_DriverThread::StaticThis = NULL;

struct structItem* Opc_client_hda_DriverThread::Item;

unsigned int msg_sent_in_control_direction = 0;

fifo_h Opc_client_hda_DriverThread::fifo_control_direction = NULL; //fifo in control direction: SCADA-------------> RTU

void Opc_client_hda_DriverThread::run()
{
	IT_IT("Opc_client_hda_DriverThread::run");

	IT_COMMENT("Opc_client_hda_DriverThread Running");

	int nRet = OpcStart(); // connect to an OPC server
	if(nRet) return;
	
	Update();

	OpcStop();

	UnitFail("OPC driver stopped");
}

void epoch_to_cp56time2a(cp56time2a *time, signed __int64 epoch_in_millisec)
{
	struct tm	*ptm;
	int ms = (int)(epoch_in_millisec%1000);
	time_t seconds;

	IT_IT("epoch_to_cp56time2a");
	
	memset(time, 0x00,sizeof(cp56time2a));
	seconds = (long)(epoch_in_millisec/1000);
	ptm = localtime(&seconds);
		
	time->hour = ptm->tm_hour;					//<0.23>
	time->min = ptm->tm_min;					//<0..59>
	time->msec = ptm->tm_sec*1000 + ms; //<0.. 59999>
	time->mday = ptm->tm_mday; //<1..31>
	time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
	time->month = ptm->tm_mon + 1; //<1..12>
	time->year = (ptm->tm_year + 1900)%100; //<0.99>
	time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
	time->su = (u_char)ptm->tm_isdst; //<0..1> SUmmer time: <0> is standard time, <1> is summer time

    return;
}

#define PROVE_TIPI

void Opc_client_hda_DriverThread::post_val(SpValue &v, QString &name)
{

	#ifndef PROVE_TIPI
	SpValue v(VALUE_TAG, &value);
	#endif
	SpValueList * l = new SpValueList;
	l->insert(l->end(),v);
	//
	DriverEvent * d = new DriverEvent(StaticThis, DriverEvent::OpPostList,0,name);
	d->SetPointer((void *)l); // set the data
	if(StaticParent) QThread::postEvent(StaticParent,d); // parent must delete data SpValueList
}

void Opc_client_hda_DriverThread::SendEvent2(VARIANT *pValue, const FILETIME* ft, DWORD pwQualities, OPCHANDLE phClientItem)
{
	IT_IT("Opc_client_hda_DriverThread::SendEvent2");
}

//The FILETIME structure is a 64-bit value representing the number 
//of 100-nanosecond intervals since January 1, 1601.
//
//Epoc is a 64-bit value representing the number of milliseconds 
//elapsed since January 1, 1970

signed __int64 Opc_client_hda_DriverThread::Epoch_from_FILETIME(const FILETIME *fileTime)
{
	IT_IT("Epoch_from_FILETIME");
	
	FILETIME localTime;
	struct tm	t;

	//QString str;
	time_t sec;
	signed __int64 epoch_in_millisec;

	if(fileTime == NULL)
	{
		return 0;
	}
	
	// first convert file time (UTC time) to local time
	if (!FileTimeToLocalFileTime(fileTime, &localTime))
	{
		return 0;
	}

	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))
	{
		return 0;
	}
	
	memset(&t, 0x00, sizeof(struct tm));
	
	t.tm_hour = sysTime.wHour;
	t.tm_min = sysTime.wMinute;
	t.tm_sec = sysTime.wSecond;
	t.tm_mday = sysTime.wDay;
	t.tm_mon = sysTime.wMonth - 1;
	t.tm_year = sysTime.wYear - 1900;
	t.tm_isdst = -1; //to force mktime to check for dst
	
	sec = mktime(&t);

	epoch_in_millisec =  (signed __int64)sec;

	epoch_in_millisec =  epoch_in_millisec*1000 + sysTime.wMilliseconds;

	return epoch_in_millisec;
}

