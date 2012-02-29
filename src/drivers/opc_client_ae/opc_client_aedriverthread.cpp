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
#include "opc_client_ae_instance.h"
#include "opc_client_aedriverthread.h"
/*
*Function:run
*runs the thread
*Inputs:none
*Outputs:none
*Returns:none
*/

bool  Opc_client_ae_DriverThread::g_bWriteComplete = true;
bool  Opc_client_ae_DriverThread::g_bReadComplete = true;
DWORD Opc_client_ae_DriverThread::g_dwReadTransID = 1;
DWORD Opc_client_ae_DriverThread::g_dwWriteTransID = 2;
//FILE *Opc_client_ae_DriverThread::g_stream = NULL; // file log handle
IOPCEventServer *Opc_client_ae_DriverThread::g_pIOPCServer = NULL;

DriverInstance *Opc_client_ae_DriverThread::StaticParent = NULL;
DriverThread *Opc_client_ae_DriverThread::StaticThis = NULL;

struct structItem* Opc_client_ae_DriverThread::Item;

unsigned int msg_sent_in_control_direction = 0;

fifo_h Opc_client_ae_DriverThread::fifo_control_direction = NULL; //fifo in control direction: SCADA-------------> RTU

void Opc_client_ae_DriverThread::run()
{
	IT_IT("Opc_client_ae_DriverThread::run");

	IT_COMMENT("Opc_client_ae_DriverThread Running");

    //for(int i = 0;;i++) //Retry loop on connection fault with OPC server
    //{
        //if(i > 0)
        //{
        //    UnitUnFail("Trying to reconnect to OPC server...");
        //}

	    int nRet = OpcStart(); // connect to an OPC server
	    if(nRet) return;
	    
	    Update();

	    OpcStop();

	    UnitFail("OPC driver stopped");

        //if(fExit)
        //{
        //    break; //Exit retry loop
        //}

        //Sleep(20000);
    //}
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

void Opc_client_ae_DriverThread::post_val(SpValue &v, QString &name)
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

void Opc_client_ae_DriverThread::SendEvent2(ONEVENTSTRUCT* pEvent)
{
	IT_IT("Opc_client_ae_DriverThread::SendEvent2");
	
	QString name;
	char event_source[100];
	FILETIME* ftTime;
	FILETIME* ftActiveTime;
	
	#ifndef PROVE_TIPI
	double value;
	#else
	cp56time2a time;
	cp56time2a active_time;
	#endif
	signed __int64 epoch_in_millisec;

	char show_msg[250];

	USES_CONVERSION;

	IT_COMMENT1("pwQualities = %d", pEvent->wQuality);

	strcpy(event_source, W2T(pEvent->szSource));
	//TODO: obtain name from lookup table, using event_source as index

	ftTime = &(pEvent->ftTime);
	epoch_in_millisec = Epoch_from_FILETIME(ftTime);
	epoch_to_cp56time2a(&time, epoch_in_millisec);

	ftActiveTime = &(pEvent->ftActiveTime);
	epoch_in_millisec = Epoch_from_FILETIME(ftActiveTime);
	epoch_to_cp56time2a(&active_time, epoch_in_millisec);

	printf("%d, %d, %ls, h:%i m:%i s:%i ms:%i %02i-%02i-%02i iv %i su %i,\
 %ls, %d, %d, %d, %ls, %ls, %d, %d, %d, h:%i m:%i s:%i ms:%i %02i-%02i-%02i iv %i su %i, %d, %d, %ls\n", 

				pEvent->wChangeMask,
				pEvent->wNewState,
				pEvent->szSource,
				//FILETIME pEvent->ftTime,
				time.hour,
				time.min,
				time.msec/1000,
				time.msec%1000,
				time.mday,
				time.month,
				time.year,
				time.iv,
				time.su,
				pEvent->szMessage,
				pEvent->dwEventType,
				pEvent->dwEventCategory,
				pEvent->dwSeverity,
				pEvent->szConditionName,
				pEvent->szSubconditionName,
				pEvent->wQuality,
				pEvent->wReserved,
				pEvent->bAckRequired,
				//FILETIME pEvent->ftActiveTime,
				active_time.hour,
				active_time.min,
				active_time.msec/1000,
				active_time.msec%1000,
				active_time.mday,
				active_time.month,
				active_time.year,
				active_time.iv,
				active_time.su,
				pEvent->dwCookie,
				pEvent->dwNumEventAttrs,
				//[size_is] VARIANT *pEventAttributes,
				pEvent->szActorID);

				sprintf(show_msg, "%d, %d, %ls, h:%i m:%i s:%i ms:%i %02i-%02i-%02i iv %i su %i,\
 %ls, %d, %d, %d, %ls, %ls, %d, %d, %d, h:%i m:%i s:%i ms:%i %02i-%02i-%02i iv %i su %i, %d, %d, %ls", 
				pEvent->wChangeMask,
				pEvent->wNewState,
				pEvent->szSource,
				//FILETIME pEvent->ftTime,
				time.hour,
				time.min,
				time.msec/1000,
				time.msec%1000,
				time.mday,
				time.month,
				time.year,
				time.iv,
				time.su,
				pEvent->szMessage,
				pEvent->dwEventType,
				pEvent->dwEventCategory,
				pEvent->dwSeverity,
				pEvent->szConditionName,
				pEvent->szSubconditionName,
				pEvent->wQuality,
				pEvent->wReserved,
				pEvent->bAckRequired,
				//FILETIME pEvent->ftActiveTime,
				active_time.hour,
				active_time.min,
				active_time.msec/1000,
				active_time.msec%1000,
				active_time.mday,
				active_time.month,
				active_time.year,
				active_time.iv,
				active_time.su,
				pEvent->dwCookie,
				pEvent->dwNumEventAttrs,
				//[size_is] VARIANT *pEventAttributes,
				pEvent->szActorID);

				ShowMessage(S_OK, "", show_msg);

/*
	fprintf(stderr,"Event time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					time.hour,
					time.min,
					time.msec/1000,
					time.msec%1000,
					time.mday,
					time.month,
					time.year,
					time.iv,
					time.su);
	fflush(stderr);
	
	fprintf(stderr,"Event active time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					active_time.hour,
					active_time.min,
					active_time.msec/1000,
					active_time.msec%1000,
					active_time.mday,
					active_time.month,
					active_time.year,
					active_time.iv,
					active_time.su);
	fflush(stderr);
*/

	{
		//ItemID = QString((const char*)W2T(Item[phClientItem - 1].wszName));

		//name = QString(Item[phClientItem - 1].spname);

		/*
		switch(V_VT(pValue))
		{
			#ifndef PROVE_TIPI

			case VT_BOOL:
			{
				value = V_BOOL(pValue);
				
				IT_COMMENT1("Value = %d", V_BOOL(pValue));
			}
			break;
			case VT_I2:
			{
				value = V_I2(pValue);
				IT_COMMENT1("Value = %d", V_I2(pValue));
			}
			break;
			case VT_I4:
			{
				value = V_I4(pValue);
				IT_COMMENT1("Value = %d", V_I4(pValue));
			}
			break;
			case VT_R4:
			{
				value = V_R4(pValue);
				IT_COMMENT1("Value = %f", V_R4(pValue));
			}
			break;
			case VT_R8:
			{
				value = V_R8(pValue);
				IT_COMMENT1("Value = %lf", V_R8(pValue));
			}
			break;
			case VT_BSTR:
			{
				//printf("ItemID = %s: ", (const char*)W2T(Item[phClientItem - 1].wszName));

				//printf("%ls\t", V_BSTR(pValue));
				//IT_COMMENT1("Value STRING = %ls", V_BSTR(pValue));

				//Definizione di BSTR:
				//typedef OLECHAR *BSTR;

				//Conversioni:

				//Da const char* a OLE

				//TCHAR* pColumnName
				//OLECHAR*    pOleColumnName = T2OLE(pColumnName);
									
				//Da OLE a const char*

				TCHAR* str = OLE2T(pValue->bstrVal);

				//printf("%s\n", str);
				value = atof(str);
				//printf("%lf\n", value);
				IT_COMMENT1("Value STRING = %s", str);
			}
			break;

			#else //PROVE_TIPI

			case VT_EMPTY:
			{
				//IT_COMMENT1("Value = %d", V_EMPTY(pValue));
			}
			break;
			case VT_I1:
			{
				is_type153 var;

				memset(&var, 0x00, sizeof(is_type153));

				var.mv = V_I1(pValue);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}
				
				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TQ_1);

				post_val(v, name);
				
				
				IT_COMMENT1("Value = %d", V_I1(pValue));
			}
			break;
			case VT_UI1:
			{
				iec_type35 var;

				memset(&var, 0x00, sizeof(iec_type35));

				var.mv = V_UI1(pValue);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TE_1);

				post_val(v, name);
			
				IT_COMMENT1("Value = %d", V_UI1(pValue));
			}
			break;
			case VT_I2:
			{
				is_type153 var;

				memset(&var, 0x00, sizeof(is_type153));

				var.mv = V_I2(pValue);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TQ_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %d", V_I2(pValue));
			}
			break;
			case VT_UI2:
			{
				iec_type35 var;

				memset(&var, 0x00, sizeof(iec_type35));

				var.mv = V_UI2(pValue);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TE_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %d", V_UI2(pValue));
			}
			break;
			case VT_I4:
			{
				is_type152 var;

				memset(&var, 0x00, sizeof(is_type152));

				var.mv = V_I4(pValue);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TP_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %d", V_I4(pValue));
			}
			break;
			case VT_UI4:
			{
				is_type154 var;

				memset(&var, 0x00, sizeof(is_type154));

				var.mv = V_UI4(pValue);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TR_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %d", V_UI4(pValue));
			}
			break;
			case VT_I8:
			{
				printf("Not supported with CV++ 6.0");
				IT_COMMENT("Not supported with CV++ 6.0");
			}
			break;
			case VT_UI8:
			{
				printf("Not supported with CV++ 6.0");
				IT_COMMENT("Not supported with CV++ 6.0");
			}
			break;
			case VT_R4:
			{
				iec_type36 var;

				memset(&var, 0x00, sizeof(iec_type36));

				var.mv = V_R4(pValue);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TF_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %f", V_R4(pValue));
			}
			break;
			case VT_R8:
			{
				is_type150 var;

				memset(&var, 0x00, sizeof(is_type150));

				var.mv = V_R8(pValue);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TN_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %lf", V_R8(pValue));
			}
			break;
			case VT_CY:
			{
				//Currency
			}
			break;
			case VT_BOOL:
			{
				iec_type30 var;

				memset(&var, 0x00, sizeof(iec_type30));

				var.sp = (V_BOOL(pValue) < 0 ? 1 : 0);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);
				
				var.time = time;

				SpValue v(VALUE_TAG, &var, M_SP_TB_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %d", V_BOOL(pValue));
			}
			break;
			case VT_DATE:
			{
				
			}
			break;
			case VT_BSTR:
			{
				//printf("ItemID = %s: ", (const char*)W2T(Item[phClientItem - 1].wszName));

				//printf("%ls\t", V_BSTR(pValue));
				//IT_COMMENT1("Value STRING = %ls", V_BSTR(pValue));

				//Definizione di BSTR:
				//typedef OLECHAR *BSTR;

				//Conversioni:

				//Da const char* a OLE

				//TCHAR* pColumnName
				//OLECHAR*    pOleColumnName = T2OLE(pColumnName);
									
				//Da OLE a const char*

				TCHAR* str = OLE2T(pValue->bstrVal);

				//printf("%s\n", str);
				
				is_type150 var;

				memset(&var, 0x00, sizeof(is_type150));

				var.mv = atof(str);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TN_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %lf", V_R8(pValue));

				//printf("%lf\n", var.mv);
				IT_COMMENT1("Value STRING = %s", str);

			}
			break;
			default:
			{
				IT_COMMENT1("V_VT(pValue) %d not supported", V_VT(pValue));

				//char err[150];

				//sprintf(err, "V_VT(pValue) %d not supported", V_VT(pValue));

				//ShowError(S_FALSE, err);

				is_type150 var;

				memset(&var, 0x00, sizeof(is_type150));

				var.mv = 0.0;

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TN_1);

				post_val(v, name);
									
				//if(SUCCEEDED(::VariantChangeType(pValue,pValue,0,VT_I4)))
				//{
					//value = V_I4(pValue);
					//IT_COMMENT1("Value = %d", value);
				//
				//}
				//else
				//{
					//printf("***\t");
				//}
			}
			break;

			#endif //#ifdef PROVE_TIPI
		}
		*/
	}
}

//The FILETIME structure is a 64-bit value representing the number 
//of 100-nanosecond intervals since January 1, 1601.
//
//Epoc is a 64-bit value representing the number of milliseconds 
//elapsed since January 1, 1970

signed __int64 Opc_client_ae_DriverThread::Epoch_from_FILETIME(const FILETIME *fileTime)
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

	if(sec < 0)
	{
		return 0;
	}

	epoch_in_millisec =  (signed __int64)sec;

	epoch_in_millisec =  epoch_in_millisec*1000 + sysTime.wMilliseconds;

	return epoch_in_millisec;
}

