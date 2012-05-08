/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2011 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */



#include "modbus_driver_instance.h"
#include "modbus_driverthread.h"

/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
#define TICKS_PER_SEC 1
void Modbus_driver_Instance::Start() 
{
	IT_IT("Modbus_driver_Instance::Start");

	State = STATE_RESET;
	QString cmd = "select * from UNITS where UNITTYPE='modbus_driver' and NAME in(" + DriverInstance::FormUnitList()+ ");";
	GetConfigureDb()->DoExec(this,cmd,tListUnits);
};
/*
*Function: Stop
*Inputs:none
*Outputs:none
*Returns:none
*/
void Modbus_driver_Instance::Stop() // stop everything under this driver's control
{	
	IT_IT("Modbus_driver_Instance::Stop");

	pTimer->stop();

	Disconnect(); //Stop consumer thread
}
/*
*Function: QueryResponse
*Inputs:none
*Outputs:none
*Returns:none
*/
void Modbus_driver_Instance::QueryResponse(QObject *p, const QString &c, int id, QObject*caller) // handles database responses
{
	IT_IT("Modbus_driver_Instance::QueryResponse");
	
	if(p != this) return;
	switch(id)
	{
		case tListUnits:
		{
			//
			// get the properties - need to be receipe aware 
			//
			unit_name = GetConfigureDb()->GetString("NAME");

			QString pc = 
			"select * from PROPS where SKEY='" + Name + 
			"' and (IKEY = '(default)' or IKEY='"+ GetReceipeName() + "') order by IKEY desc;"; 
			//
			// get the properties SKEY = unit name IKEY = receipe name
			// 
			GetConfigureDb()->DoExec(this,pc,tUnitProperties);

			
			if(GetReceipeName() == "(default)")
			{
				// get the properties for this receipe  - fall back to default
				QString cmd = "select * from SAMPLE where UNIT='" + GetConfigureDb()->GetString("NAME") + "' and ENABLED=1;"; 
				GetConfigureDb()->DoExec(this,cmd,tList);
			}
			else
			{
				QString cmd = 
				"select * from SAMPLE where UNIT='" + GetConfigureDb()->GetString("NAME") + 
				"' and NAME in (" + DriverInstance::FormSamplePointList() + ");"; 
				// only the enabled sample points
				GetConfigureDb()->DoExec(this,cmd,tList);
			};
		}
		break;
		case tList: // list of sample points controlled by the unit
		{
			//  
			// copy the list to local dictionary 
			// now get the properties for each of the sample points
			QString nl; // name list for getting properties
			// 
			for(int ii = 1; ii <= IecItems; ii++) // initialise
			{
				Values[ii].Name = "";
				Values[ii].clear();
			};
			//  
			// 
			int n = GetConfigureDb()->GetNumberResults();
			for(int i = 0; i < n; i++,GetConfigureDb()->FetchNext())
			{
				int idx = GetConfigureDb()->GetInt("IPINDEX");
				if (idx > 0 && idx <= IecItems)
				{
					Values[idx].Name = GetConfigureDb()->GetString("NAME"); // save the name
				};

				if(i)
				{
					nl += ",";
				};

				nl += "'" + GetConfigureDb()->GetString("NAME") + "'";
			};
			
			QString cmd = "select * from PROPS where SKEY='SAMPLEPROPS' and IKEY in ("+nl+");";
			GetConfigureDb()->DoExec(this,cmd,tSamplePointProperties);
		};
		break;
		case tSamplePointProperties://properties specific to a sample point
		{
			int n = GetConfigureDb()->GetNumberResults();
			for(int j = 0; j < n; j++,GetConfigureDb()->FetchNext()) // may get a boat load of properties back
			{  
				// look for the entry in the table
				for(int k = 1; k <= IecItems;k++)
				{
					if(Values[k].Name == GetConfigureDb()->GetString("IKEY"))
					{
//						QString s = UndoEscapeSQLText(GetConfigureDb()->GetString("DVAL"));
//						QTextIStream is(&s); // extract the values
//						QString a;  
//						is >> a; // interval
//						Values[k].SamplePeriod = QStringToInterval(a);
//						Values[k].clear(); // set up
//						is >> a; // now get the 
//						Values[k].Low  = a.toDouble(); // 4 - 20 low value 
//						is >> a;
//						Values[k].High = a.toDouble(); // 4 - 20 high value
//						is >> a;
//						Values[k].fSpotValue = a.toInt(); 
						break;                    
					};
				};
			};
		};
		break;
		case tUnitProperties:
		{
			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				// 
				QString s = UndoEscapeSQLText(GetConfigureDb()->GetString("DVAL")); // the top one is either the receipe or (default)
				QTextIStream is(&s); // extract the values
				//
				is >> IecItems;	  // how many modbus items there are in the RTU or PLC
				is >> Cfg.SampleTime; // how long we sample for in milliseconds
				is >> Cfg.MODBUSServerIPAddress; // MODBUS server IP Address
				is >> Cfg.MODBUSServerIPPort; // MODBUS server TCP port

				Countdown = 1;

				if(Values)
				{
					delete[] Values;
					Values = NULL;
				}

				Values = new Track[IecItems+1];
				//
				if(InTest())
				{
					Cfg.SampleTime = 1000; // override sampling period
				};
				//

				//Start MODBUS client driver
				if(!Connect())
				{
					QSLogAlarm(Name,tr("Failed to start MODBUS client driver"));
				}
			}
		}
		break;
		case tGetSamplePointNamefromIOA:
		{
			QSTransaction &t = GetConfigureDb()->CurrentTransaction();

			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				// 
				#ifdef DEPRECATED_MODBUS_CONFIG
				QString SamplePointName = UndoEscapeSQLText(GetConfigureDb()->GetString("IKEY"));
				#else
				QString SamplePointName = UndoEscapeSQLText(GetConfigureDb()->GetString("NAME"));
				#endif

				double v = 0.0;

				if(strlen((const char*)t.Data1) > 0)
				{
					v = atof((const char*)t.Data1);
					PostValue(SamplePointName, "BIT", v); //Post the value directly in memory database
				}

				printf("SamplePointName = %s, IOA = %s, value = %lf\n", (const char*)SamplePointName, (const char*)t.Data2, v);
			}
		}
		break;
		case tGetIOAfromSamplePointName:
		{
			QSTransaction &t = GetConfigureDb()->CurrentTransaction();

			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				// 
				#ifdef DEPRECATED_MODBUS_CONFIG
				QString IOACommand = UndoEscapeSQLText(GetConfigureDb()->GetString("DVAL"));
				#else
				QString IOACommand = UndoEscapeSQLText(GetConfigureDb()->GetString("PARAMS"));
				#endif
				
				int command_value = 0;
				int ioa_command = 0;

				ioa_command = atoi((const char*)IOACommand);

				if(strlen((const char*)t.Data1) > 0)
				{
					command_value = atoi((const char*)t.Data1);
				}

				printf("IOA command = %d, value = %d\n", ioa_command, command_value);

				//Send C_SC_NA_1//////////////////////////////////////////////////////////////////////////
				struct iec_item item_to_send;
				memset(&item_to_send,0x00, sizeof(struct iec_item));
				item_to_send.iec_type = C_SC_NA_1;
				item_to_send.iec_obj.ioa = ioa_command;
				item_to_send.iec_obj.o.type45.scs = command_value;
				item_to_send.msg_id = msg_sent_in_control_direction++;
				item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
				fifo_put(fifo_control_direction, (char *)&item_to_send, sizeof(struct iec_item));
				///////////////////////////////////////////////////////////////////////////////////////////
			}
		}
		break;
		default:
		break;
	}
}

/*
*Function: driverEvent
*handles asynchronous notifications from a child driver thread
*Inputs:opcode , data as int, data as QString
*Outputs:none
*Returns:none
*/
//Realtime method
void Modbus_driver_Instance::driverEvent(DriverEvent *p)
{
	IT_IT("Modbus_driver_Instance::driverEvent");
	
	switch(p->opcode())
	{
		case DriverEvent::OpEndTransaction:
		{
			//Gestire evento di comando eseguito
		}
		break;
		case DriverEvent::OpSendEventString:
		{
			//
			QSLogEvent(Name, p->text());
			//
		};
		break;
		case DriverEvent::OpSendAlarmString:
		{
			//
			QSLogAlarm(Name, p->text());
			//
		};
		break;
		default:
		break;
	}

	DriverInstance::driverEvent(p);
}

/*
*Function: expect
*checks a given answer
*Inputs:none
*Outputs:none
*Returns:none
*/

//Realtime method
bool Modbus_driver_Instance::expect(unsigned int cmd)
{
	IT_IT("Modbus_driver_Instance::expect");

    if(InQueue.count())
	{
		SendRecePacket *packet = InQueue.head(); // get the current transaction
		if(packet->dwAnswerType != cmd) 
		{
			IT_COMMENT2("Error: Get answer %d, instead of %d", packet->dwAnswerType, cmd);
			return false;
		}
		return true;
	}

	IT_COMMENT("Error: No answer yet!");
	return false;
}

//Realtime method
void Modbus_driver_Instance::removeTransaction()
{
	IT_IT("Modbus_driver_Instance::removeTransaction");

	if(InQueue.count() > 0)
	{
		InQueue.remove(); //end of the life cycle of InQueue SendRecePacket
		pending_transactions--;
		IT_COMMENT1("PENDING COMMANDS %d", pending_transactions);

		if(InQueue.count() > 0)
		{
//			pConnect->SetCommand(InQueue.head()); // send the next transaction
		}
	}
}

void strip_white_space(char *dst, const char *src, int len)
{
    int i, j, n = strlen(src);
    
    memset(dst, 0x00, len);

    for (i = 0, j = 0; i < n; i++)
	{
		if (src[i] != ' ')
		{
			dst[j++] = src[i];
		}
	}
}

/*
*Function: Tick
*checks for triggered events - run every second or so
*Inputs:none
*Outputs:none
*Returns:none
*/

void Modbus_driver_Instance::Tick()
{
	IT_IT("Modbus_driver_Instance::Tick");

	//This code runs inside main monitor.exe thread

	//cp56time2a time;
	//signed __int64 epoch_in_millisec;

	unsigned char buf[sizeof(struct iec_item)];
	int len;
	const unsigned wait_limit_ms = 1;
	struct iec_item* p_item;

	for(int i = 0; (len = fifo_get(fifo_monitor_direction, (char*)buf, sizeof(struct iec_item), wait_limit_ms)) >= 0; i += 1)	
	{ 
		p_item = (struct iec_item*)buf;
			
		//printf("Receiving %d th message \n", p_item->msg_id);

		//for (int j = 0; j < len; j++) 
		//{ 
			//assert((unsigned char)buf[i] == len);
			//unsigned char c = *((unsigned char*)buf + j);
			//printf("rx <--- 0x%02x-\n", c);
			//fprintf(fp,"rx <--- 0x%02x-\n", c);
			//fflush(fp);

			//IT_COMMENT1("rx <--- 0x%02x-\n", c);
		//}

		//printf("---------------\n");

		unsigned char rc = clearCrc((unsigned char *)buf, sizeof(struct iec_item));
		if(rc != 0)
		{
			ExitProcess(0);
		}

		QString value;

		switch(p_item->iec_type)
		{
			case M_SP_NA_1:
			{
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type1 var = p_item->iec_obj.o.type1;
				
				SpValue v(VALUE_TAG, &var, M_SP_NA_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type1.sp);

				#endif
				
			}
			break;
			case M_DP_NA_1:
			{
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type3 var = p_item->iec_obj.o.type3;
				
				SpValue v(VALUE_TAG, &var, M_DP_NA_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type3.dp);

				#endif
			}
			break;
			//case M_BO_NA_1:
			//{
			//}
			//break;
			case M_ME_NA_1:
			{
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type9 var = p_item->iec_obj.o.type9;
				
				SpValue v(VALUE_TAG, &var, M_ME_NA_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type9.mv);

				#endif
			}
			break;
			case M_ME_NB_1:
			{
				
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type11 var = p_item->iec_obj.o.type11;
				
				SpValue v(VALUE_TAG, &var, M_ME_NB_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type11.mv);

				#endif
			}
			break;
			case M_ME_NC_1:
			{
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type13 var = p_item->iec_obj.o.type13;
				
				SpValue v(VALUE_TAG, &var, M_ME_NC_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type13.mv);

				#endif
			}
			break;
			case M_SP_TB_1:
			{
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type30 var = p_item->iec_obj.o.type30;
				
				SpValue v(VALUE_TAG, &var, M_SP_TB_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type30.sp);

				#endif
			}
			break;
			case M_DP_TB_1:
			{
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type31 var = p_item->iec_obj.o.type31;
				
				SpValue v(VALUE_TAG, &var, M_DP_TB_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type31.dp);

				#endif
			}
			break;
			case M_BO_TB_1:
			{
				//value.sprintf("%d", p_item->iec_obj.o.type33.stcd);
			}
			break;
			case M_ME_TD_1:
			{
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type34 var = p_item->iec_obj.o.type34;
				
				SpValue v(VALUE_TAG, &var, M_ME_TD_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type34.mv);

				#endif
			}
			break;
			case M_ME_TE_1:
			{
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type35 var = p_item->iec_obj.o.type35;
				
				SpValue v(VALUE_TAG, &var, M_ME_TE_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type35.mv);

				#endif
			}
			break;
			case M_ME_TF_1:
			{
				#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

				iec_type36 var = p_item->iec_obj.o.type36;
				
				SpValue v(VALUE_TAG, &var, M_ME_TF_1);
				TODO:05-07-2011 Get name here
				post_val(v, name);

				#else

				value.sprintf("%d", p_item->iec_obj.o.type36.mv);

				#endif
			}
			break;
			//case M_IT_TB_1:
			//{
			//}
			//break;
            case C_EX_IT_1:
			{
                printf("Child process exiting...\n");
			}
			break;
			default:
			{
				printf("Not supported type\n");
				value.sprintf("%d", 0);
			}
			break;
		}
		
		QString ioa;
		ioa.sprintf("%d", p_item->iec_obj.ioa);

		#ifdef DEPRECATED_MODBUS_CONFIG
		QString cmd = "select IKEY from PROPS where DVAL='"+ ioa + "' and SKEY='SAMPLEPROPS';";
		#else
		QString cmd = "select NAME from TAGS where PARAMS='"+ ioa + "' and UNIT='"+ Name + "';";
		#endif

		GetConfigureDb()->DoExec(this, cmd, tGetSamplePointNamefromIOA, value, ioa);

		//printf("ioa %s, value %s\n", (const char*)ioa, (const char*)value);

		if(i > 50)
		{
			break;
		}
	}
}

/*
*Function:event
*event handler
*Inputs:none
*Outputs:none
*Returns:none
*/

//Realtime method
bool Modbus_driver_Instance::event(QEvent *e)
{
	IT_IT("Modbus_driver_Instance::event");

	if(e->type() == QEvent::User)
	{
		// handle it
		// add the next item from the queue 
					
		//if(InQueue.count() > 0 && pConnect)
		//{
			//SendRecePacket *p = InQueue.head(); // get the current transaction
			//DriverEvent *d = ((DriverEvent *)e); // handle the driver event
			//driverEvent(d);

			//InQueue.remove(); // delete current transaction
			//pending_transactions--;
			//IT_COMMENT1("PENDING COMMANDS %d", pending_transactions);

			//if(InQueue.count() > 0)
			//{
			//	pConnect->SetCommand(InQueue.head()); // send the next transaction
			//}
		//}
		
		DriverEvent *d = ((DriverEvent *)e); // handle the driver event
		driverEvent(d);

		return true;
	}
	return QObject::event(e);
};

#include <signal.h>

char* get_date_time()
{
	static char sz[128];
	time_t t = time(NULL);
	struct tm *ptm = localtime(&t);
	
	strftime(sz, sizeof(sz)-2, "%m/%d/%y %H:%M:%S", ptm);

	strcat(sz, "|");
	return sz;
}

void iec_call_exit_handler(int line, char* file, char* reason)
{
	FILE* fp;
	char program_path[_MAX_PATH];
	char log_file[_MAX_FNAME+_MAX_PATH];
	IT_IT("iec_call_exit_handler");

	program_path[0] = '\0';
#ifdef WIN32
	if(GetModuleFileName(NULL, program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\bin off path
    }
#elif __unix__
	if(getcwd(program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '/')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '/')) = '\0';        // Strip \\bin off path
    }
#endif

	strcpy(log_file, program_path);

#ifdef WIN32
	strcat(log_file, "\\logs\\modbus.log");
#elif __unix__
	strcat(log_file, "/logs/modbus.log");	
#endif

	fp = fopen(log_file, "a");

	if(fp)
	{
		if(line && file && reason)
		{
			fprintf(fp, "PID:%d time:%s exit process at line: %d, file %s, reason:%s\n", GetCurrentProcessId, get_date_time(), line, file, reason);
		}
		else if(line && file)
		{
			fprintf(fp, "PID:%d time:%s exit process at line: %d, file %s\n", GetCurrentProcessId, get_date_time(), line, file);
		}
		else if(reason)
		{
			fprintf(fp, "PID:%d time:%s exit process for reason %s\n", GetCurrentProcessId, get_date_time(), reason);
		}

		fflush(fp);
		fclose(fp);
	}

	//raise(SIGABRT);   //raise abort signal which in turn starts automatically a separete thread and call exit SignalHandler
	ExitProcess(0);

	IT_EXIT;
}


/*
*Function: Connect
*Inputs:none
*Outputs:none
*Returns:none
*/

bool Modbus_driver_Instance::Connect() 
{	
	IT_IT("Modbus_driver_Instance::Connect");
	//return 0 on fail
	//return 1 on success

	if(pConnect) delete pConnect;
	
	pConnect = new Modbus_DriverThread(this); // create the connection

	if(pConnect->Ok()) 
	{
		pConnect->start();
	}

	return true;
}

/*
*Function:Disconnect
*Inputs:none
*Outputs:none
*Returns:none
*/
bool  Modbus_driver_Instance::Disconnect()        
{
	IT_IT("Modbus_driver_Instance::Disconnect");
	
	bool res = true;
	
	InQueue.clear();

    pConnect->TerminateProtocol();

	if(pConnect) delete pConnect;
	pConnect = NULL;

	return res;
};

/*
*Function:DoExec
*Inputs:clien tobject, command string,transaction id,data1 and data 2
*Outputs:none
*Returns:none
*/
//Realtime method
bool  Modbus_driver_Instance::DoExec(SendRecePacket *t)
{
	IT_IT("Modbus_driver_Instance::DoExec");
	
	bool res = false;

	return res;
};

/*
*Function:Command
*Inputs:none
*Outputs:none
*Returns:none
*/
void Modbus_driver_Instance::Command(const QString & name, BYTE cmd, LPVOID lpPa, DWORD pa_length, DWORD ipindex) // process a command for a named unit 
{
	IT_IT("Modbus_driver_Instance::Command");

	dispatcher_extra_params* params = (dispatcher_extra_params *)lpPa;

	QString sample_point_name = QString(params->string2);

	IT_COMMENT3("Received command for instance %s, sample point: %s, value: %lf", (const char*)name, (const char*)sample_point_name, (params->res[0]).value);

	#ifdef DEPRECATED_MODBUS_CONFIG
	QString pc = "select * from PROPS where IKEY='" + sample_point_name + "';"; 
	#else
	QString pc = "select * from TAGS where NAME='" + sample_point_name + "';";
	#endif

	QString value_for_command;
	value_for_command.sprintf("%lf", (params->res[0]).value);
	// 
	GetConfigureDb()->DoExec(this, pc, tGetIOAfromSamplePointName, value_for_command);
}

