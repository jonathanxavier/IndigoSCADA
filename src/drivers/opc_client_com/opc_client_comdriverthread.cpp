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
#include "opc_client_com_instance.h"
#include "opc_client_comdriverthread.h"
/*
*Function:run
*runs the thread
*Inputs:none
*Outputs:none
*Returns:none
*/

//Opc_client_com_DriverThread Opc_client_com_DriverThread::OpcClients[3];
bool  Opc_client_com_DriverThread::g_bWriteComplete = true;
bool  Opc_client_com_DriverThread::g_bReadComplete = true;
bool  Opc_client_com_DriverThread::g_bPoll = false; // poll for values or async updates
DWORD Opc_client_com_DriverThread::g_dwReadTransID = 1;
DWORD Opc_client_com_DriverThread::g_dwWriteTransID = 2;
//FILE *Opc_client_com_DriverThread::g_stream = NULL; // file log handle
IOPCServer *Opc_client_com_DriverThread::g_pIOPCServer = NULL;

DriverInstance *Opc_client_com_DriverThread::StaticParent = NULL;
DriverThread *Opc_client_com_DriverThread::StaticThis = NULL;

struct structItem* Opc_client_com_DriverThread::Item;

bool  Opc_client_com_DriverThread::mandare_eventi = false;

unsigned int msg_sent_in_control_direction = 0;

fifo_h Opc_client_com_DriverThread::fifo_control_direction = NULL; //fifo in control direction: SCADA-------------> RTU

void Opc_client_com_DriverThread::run()
{
	IT_IT("Opc_client_com_DriverThread::run");

	IT_COMMENT("Opc_client_com_DriverThread Running");

	int nRet = OpcStart(); // connect to an OPC server
	if(nRet) return;

	nRet = AddItems(); // add some items
	if(nRet) return;

	//OPC DA 1.0
	//SyncRead(false);//Sincronous read from opc server (Read from Device)
	//AsyncRead(false); //Asincronous read from opc server (Read from Device)
	//AsyncUpdate();
	/////////////////////////////////////////General interrogation//////////////////////////////////////////////////////////////////////////////////////////////////////
	//OPC DA 2.0 This function sends all items (i.e IEC 101 General Interrogation)
	//This function is called IF AND ONLY IF operator ask general interrogation
	//Async2Read(false); 
	/////////////////////////////////////////Variazioni come spontanee//////////////////////////////////////////////////////////////////////////////////////////////////////
	//OPC DA 2.0 This function on the first transaction send all items 
	//the arrives spontaneous variations (i.e. IEC 101 Spontaneaous variations)
	Async2Update();

	OpcStop();

	UnitFail("OPC driver stopped");
}

void Opc_client_com_DriverThread::SendEvent(const OPCITEMHEADER1* h, VARIANT *pValue, const FILETIME* ft)
{
	IT_IT("Opc_client_com_DriverThread::SendEvent");

	QString name;
	double value;

	USES_CONVERSION;
	
	if(Opc_client_com_DriverThread::mandare_eventi)  //27-10-09
	{
		if(h->wQuality == OPC_QUALITY_GOOD)
		{
			//ItemID = QString((const char*)W2T(Item[h->hClient].wszName));

			name = QString(Item[h->hClient].spname);
			//name = QString(Item[h->hClient - 1].spname); mettere il -1 ?

			switch(V_VT(pValue))
			{
				case VT_BOOL:
				{
					value = V_BOOL(pValue);
				}
				break;
				case VT_I2:
				{
					value = V_I2(pValue);
				}
				break;
				case VT_I4:
				{
					value = V_I4(pValue);
				}
				break;
				case VT_R4:
				{
					value = V_R4(pValue);
				}
				break;
				case VT_R8:
				{
					value = V_R8(pValue);
				}
				break;
				case VT_BSTR:
				{
					//TODO: 27-11-09 gestire il tipo stringa
					//printf("%ls\t", V_BSTR(pValue));
					IT_COMMENT1("Value STRING = %ls", V_BSTR(pValue));
				}
				break;
				default:
				{
					if(SUCCEEDED(::VariantChangeType(pValue,pValue,0,VT_I4)))
					{
						value = V_I4(pValue);
					}
					else
					{
						//printf("***\t");
					}
				}
				break;
			}
		}
		else
		{
			value = 0;
		}

		//QString epoch_in_millisec = Epoch_from_FILETIME(ft);
		
		//TODO:	send epoch_in_millisec to Parent
		
		SpValue v(VALUE_TAG, &value);
		SpValueList * l = new SpValueList;
		l->insert(l->end(),v);
		//
		DriverEvent * d = new DriverEvent(StaticThis, DriverEvent::OpPostList,0,name);
		d->SetPointer((void *)l); // set the data
		if(StaticParent) QThread::postEvent(StaticParent,d); // parent must delete data SpValueList
	}
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

void Opc_client_com_DriverThread::post_val(SpValue &v, QString &name)
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

void Opc_client_com_DriverThread::SendEvent2(VARIANT *pValue, const FILETIME* ft, DWORD pwQualities, OPCHANDLE phClientItem)
{
	IT_IT("Opc_client_com_DriverThread::SendEvent2");
	
	QString name;

	#ifndef PROVE_TIPI
	double value;
	#else
	cp56time2a time;
	#endif
	signed __int64 epoch_in_millisec;

	USES_CONVERSION;

	IT_COMMENT1("pwQualities = %d", pwQualities);
	IT_COMMENT1("phClientItem = %d", phClientItem);

	epoch_in_millisec = Epoch_from_FILETIME(ft);
	
	//if(Opc_client_com_DriverThread::mandare_eventi)  //27-10-09
	{
		//ItemID = QString((const char*)W2T(Item[phClientItem - 1].wszName));

		name = QString(Item[phClientItem - 1].spname);

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
				/*
				is_type156 var;

				memset(&var, 0x00, sizeof(is_type156));

				var.mv = V_I8(pValue);

				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				//var.mv = pValue->llVal;

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TT_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %ld", pValue->llVal);
				*/
				
			}
			break;
			case VT_UI8:
			{
				printf("Not supported with CV++ 6.0");
				IT_COMMENT("Not supported with CV++ 6.0");
				/*
				is_type155 var;

				memset(&var, 0x00, sizeof(is_type155));

				var.mv = V_UI8(pValue);

  				if(pwQualities != OPC_QUALITY_GOOD)
				{
					printf("pwQualities = %d\n", pwQualities);
					var.iv = 1;
				}
				else
				{
					var.iv = 0;
				}

				//var.mv = pValue->ullVal;

				epoch_to_cp56time2a(&time, epoch_in_millisec);

				var.time = time;

				SpValue v(VALUE_TAG, &var, M_ME_TS_1);

				post_val(v, name);
				
				IT_COMMENT1("Value = %d", pValue->ullVal);
				*/
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
			/*
			case VT_VARIANT:
			{					

			}
			break;
			case VT_ARRAY | VT_I1:
			{	
				
			}
			break;
			case VT_ARRAY | VT_UI1:
			{	
				
			}
			break;
			case VT_ARRAY | VT_I2:
			{	
				
			}
			break;
			case VT_ARRAY | VT_UI2:
			{
				
			}
			break;
			case VT_ARRAY | VT_I4:
			{
				
			}
			break;
			case VT_ARRAY | VT_UI4:
			{
				
			}
			break;
			case VT_ARRAY | VT_I8:
			{
				
			}
			break;
			case VT_ARRAY | VT_UI8:
			{
				
			}
			break;
			case VT_ARRAY | VT_R4:
			{
				
			}
			break;
			case VT_ARRAY | VT_R8:
			{
				
			}
			break;
			case VT_ARRAY | VT_CY:
			{
				
			}
			break;
			case VT_ARRAY | VT_BOOL:
			{
				
			}
			break;
			case VT_ARRAY | VT_DATE:
			{
				
			}
			break;
			case VT_ARRAY | VT_BSTR:
			{
				
			}
			break;
			case VT_ARRAY | VT_VARIANT:
			{
				
			}
			break;
			*/
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
	}
}

//The FILETIME structure is a 64-bit value representing the number 
//of 100-nanosecond intervals since January 1, 1601.
//
//Epoc is a 64-bit value representing the number of milliseconds 
//elapsed since January 1, 1970

signed __int64 Opc_client_com_DriverThread::Epoch_from_FILETIME(const FILETIME *fileTime)
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

	//char buffer[20];
	//_i64toa(epoch_in_millisec, buffer, 10);
	//str = QString(buffer);

	//IT_COMMENT((const char *)str);
	//return str;

	return epoch_in_millisec;
}

