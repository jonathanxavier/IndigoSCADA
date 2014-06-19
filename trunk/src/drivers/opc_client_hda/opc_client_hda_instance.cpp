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
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
#define TICKS_PER_SEC 1
void Opc_client_hda_Instance::Start() 
{
	IT_IT("Opc_client_hda_Instance::Start");

	State = STATE_RESET;
	QString cmd = "select * from UNITS where UNITTYPE='opc_client_hda_driver' and NAME in(" + DriverInstance::FormUnitList()+ ");";
	GetConfigureDb()->DoExec(this,cmd,tListUnits);
};
/*
*Function: Stop
*Inputs:none
*Outputs:none
*Returns:none
*/
void Opc_client_hda_Instance::Stop() // stop everything under this driver's control
{	
	IT_IT("Opc_client_hda_Instance::Stop");

	pTimer->stop();

	Disconnect(); //Stop consumer thread
}
/*
*Function: QueryResponse
*Inputs:none
*Outputs:none
*Returns:none
*/
void Opc_client_hda_Instance::QueryResponse(QObject *p, const QString &c, int id, QObject*caller) // handles database responses
{
	IT_IT("Opc_client_hda_Instance::QueryResponse");
	
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
			for(int ii = 1; ii <= OpcItems; ii++) // initialise
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
				if (idx > 0 && idx <= OpcItems)
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
				for(int k = 1; k <= OpcItems;k++)
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
				is >> OpcItems;	  // how many OPC items there are in the TRU or PLC
				is >> Cfg.SampleTime; // how long we sample for in milliseconds
				is >> Cfg.OpcServerProgID;    // Opc Server ProgID
				is >> Cfg.OpcServerIPAddress; // Opc Server IP Address
				is >> Cfg.OpcclassId; // Opc Server Class ID

				Countdown = 1;

				if(Values)
				{
					delete[] Values;
					Values = NULL;
				}

				Values = new Track[OpcItems+1];
				//
				if(InTest())
				{
					Cfg.SampleTime = 1000; // override sampling period
				};
				//

				//Start opc client driver
				if(!Connect())
				{
					QSLogAlarm(Name,tr("Failed to start OPC client driver"));
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
				#ifdef DEPRECATED_OPC_CLIENT_HDA_CONFIG
				QString SamplePointName = UndoEscapeSQLText(GetConfigureDb()->GetString("IKEY"));
				#else
				QString SamplePointName = UndoEscapeSQLText(GetConfigureDb()->GetString("NAME"));
				#endif

				double v = 0.0;

				if(strlen((const char*)t.Data1) > 0)
				{
					v = atof((const char*)t.Data1);
                    PostValue(SamplePointName, "VALUE", v); //Post the value directly in memory database
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
				int IOACommand = GetConfigureDb()->GetInt("IOA");
				
				double command_value = 0.0;

				if(strlen((const char*)t.Data1) > 0)
				{
					command_value = atof((const char*)t.Data1);
				}

				QString cmd_time_stamp = t.Data3;

				__int64 epoch_in_millisec = _atoi64((const char*)cmd_time_stamp);

				struct cp56time2a iec_cmd_time;
				
				epoch_to_cp56time2a(&iec_cmd_time, epoch_in_millisec);

				printf("Command from %s, IOA = %d, value = %lf\n", (const char*)t.Data2, IOACommand, command_value);

				//Send C_SE_TC_1//////////////////////////////////////////////////////////////////////////
				struct iec_item item_to_send;
				memset(&item_to_send,0x00, sizeof(struct iec_item));
				item_to_send.iec_type = C_SE_TC_1;
				item_to_send.iec_obj.ioa = IOACommand;
				item_to_send.iec_obj.o.type63.sv = (float)command_value;
				
				item_to_send.iec_obj.o.type63.time = iec_cmd_time;

				item_to_send.msg_id = msg_sent_in_control_direction++;
				item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
				///////////////////////////////////////////////////////////////////////////////////////////

				////////////////////Middleware/////////////////////////////////////////////
				//prepare published data
				//memset(&instanceSend,0x00, sizeof(iec_item_type));
				//instanceSend.iec_type = item_to_send.iec_type;
				//memcpy(&(instanceSend.iec_obj), &(item_to_send.iec_obj), sizeof(struct iec_object));
				//instanceSend.msg_id = item_to_send.msg_id;
				//instanceSend.checksum = item_to_send.checksum;

				//ORTEPublicationSend(publisher);
				//////////////////////////Middleware/////////////////////////////////////////
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
void Opc_client_hda_Instance::driverEvent(DriverEvent *p)
{
	IT_IT("Opc_client_hda_Instance::driverEvent");
	
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
bool Opc_client_hda_Instance::expect(unsigned int cmd)
{
	IT_IT("Opc_client_hda_Instance::expect");

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
void Opc_client_hda_Instance::removeTransaction()
{
	IT_IT("Opc_client_hda_Instance::removeTransaction");

	if(InQueue.count() > 0 && pConnect)
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
//Realtime timer
void Opc_client_hda_Instance::Tick()
{
	IT_IT("Opc_client_hda_Instance::Tick");

	switch(State)
	{
		case STATE_RESET:
		{
			State = STATE_IDLE;
			Countdown = 2;
		}
		break;
		case STATE_IDLE:
		{
			if(!Countdown)
			{
				State = STATE_DONE;
			}
			else
			{
				Countdown--;
			}
		}
		break;
		//
		case STATE_DONE:
		{
			
		};
		break;
		//
		case STATE_FAIL:
		{
			State = STATE_IDLE;
		};
		break;
		//
		default:
		break;
	};
};

/*
*Function:event
*event handler
*Inputs:none
*Outputs:none
*Returns:none
*/

//Realtime method
bool Opc_client_hda_Instance::event(QEvent *e)
{
	IT_IT("Opc_client_hda_Instance::event");

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

/*
*Function: Connect
*Inputs:none
*Outputs:none
*Returns:none
*/

bool Opc_client_hda_Instance::Connect() 
{	
	IT_IT("Opc_client_hda_Instance::Connect");
	//return 0 on fail
	//retunr 1 on success

	if(pConnect) delete pConnect;
	
	pConnect = new Opc_client_hda_DriverThread(this); // create the connection

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
bool  Opc_client_hda_Instance::Disconnect()        
{
	IT_IT("Opc_client_hda_Instance::Disconnect");
	
	bool res = true;
	
	InQueue.clear();

	if(pConnect) delete pConnect;	//added on 27-11-09
	pConnect = NULL;	//added on 27-11-09

	return res;
};

/*
*Function:DoExec
*Inputs:clien tobject, command string,transaction id,data1 and data 2
*Outputs:none
*Returns:none
*/
//Realtime method
bool  Opc_client_hda_Instance::DoExec(SendRecePacket *t)
{
	IT_IT("Opc_client_hda_Instance::DoExec");
	
	bool res = false;
/*
	if(pConnect)
	{
		IT_COMMENT3("OPC TRANSACTION de %d, cmd %d, lpPa %s", t->Dest, t->CommandType, (char*)(t->lpParams));

		if(!InQueue.count()) 
		{
			// we have a zero count so must trigger the send receive loop
			pConnect->SetCommand(t);
		}
		
		pending_transactions++;

		IT_COMMENT1("PENDING COMMANDS %d", pending_transactions);

		InQueue.enqueue(t); 
	}
*/
	return res;
};

/*
*Function:Command
*Inputs:none
*Outputs:none
*Returns:none
*/
void Opc_client_hda_Instance::Command(const QString & name, BYTE cmd, LPVOID lpPa, DWORD pa_length, DWORD ipindex) // process a command for a named unit 
{
	IT_IT("Opc_client_hda_Instance::Command");

	if(pConnect)
	{
		dispatcher_extra_params* params = (dispatcher_extra_params *)lpPa;

		QString sample_point_name = QString(params->string2);

		IT_COMMENT3("Received command for instance %s, sample point: %s, value: %lf", (const char*)name, (const char*)sample_point_name, params->value);

		QString pc = "select * from TAGS where NAME='" + sample_point_name + "';";

		QString value_for_command;
		value_for_command.sprintf("%lf", params->value);

		/////////////////////////////////command time stamp/////////////////////////////////////////////////
		__int64 command_arrive_time_in_ms = Epoch_in_millisec_from_cp56time2a(&(params->time_stamp));

		char buffer[20];
		_i64toa(command_arrive_time_in_ms, buffer, 10);
		QString cmd_epoch_in_ms = QString(buffer);
		////////////////////////////////////////////////////////////////////////////////////////////////////

		GetConfigureDb()->DoExec(this, pc, tGetIOAfromSamplePointName, value_for_command, sample_point_name, cmd_epoch_in_ms);
	}
}

void Opc_client_hda_Instance::epoch_to_cp56time2a(cp56time2a *time, signed __int64 epoch_in_millisec)
{
	struct tm	*ptm;
	int ms = (int)(epoch_in_millisec%1000);
	time_t seconds;
	
	memset(time, 0x00,sizeof(cp56time2a));
	seconds = (long)(epoch_in_millisec/1000);
	ptm = localtime(&seconds);
		
    if(ptm)
	{
		time->hour = ptm->tm_hour;					//<0.23>
		time->min = ptm->tm_min;					//<0..59>
		time->msec = ptm->tm_sec*1000 + ms; //<0.. 59999>
		time->mday = ptm->tm_mday; //<1..31>
		time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
		time->month = ptm->tm_mon + 1; //<1..12>
		time->year = ptm->tm_year - 100; //<0.99>
		time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
		time->su = (u_char)ptm->tm_isdst; //<0..1> SUmmer time: <0> is standard time, <1> is summer time
	}

    return;
}
