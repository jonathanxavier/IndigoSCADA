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



#include "opc_client_da_instance.h"
#include "opc_client_dadriverthread.h"

/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
#define TICKS_PER_SEC 1
void Opc_client_da_Instance::Start() 
{
	IT_IT("Opc_client_da_Instance::Start");

	State = STATE_RESET;
	QString cmd = "select * from UNITS where UNITTYPE='opc_client_da' and NAME in(" + DriverInstance::FormUnitList()+ ");";
	GetConfigureDb()->DoExec(this,cmd,tListUnits);
};
/*
*Function: Stop
*Inputs:none
*Outputs:none
*Returns:none
*/
void Opc_client_da_Instance::Stop() // stop everything under this driver's control
{	
	IT_IT("Opc_client_da_Instance::Stop");

	pTimer->stop();

	Disconnect(); //Stop consumer thread
}
/*
*Function: QueryResponse
*Inputs:none
*Outputs:none
*Returns:none
*/
void Opc_client_da_Instance::QueryResponse(QObject *p, const QString &c, int id, QObject*caller) // handles database responses
{
	IT_IT("Opc_client_da_Instance::QueryResponse");
	
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
void Opc_client_da_Instance::driverEvent(DriverEvent *p)
{
	IT_IT("Opc_client_da_Instance::driverEvent");
	
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
bool Opc_client_da_Instance::expect(unsigned int cmd)
{
	IT_IT("Opc_client_da_Instance::expect");

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
void Opc_client_da_Instance::removeTransaction()
{
	IT_IT("Opc_client_da_Instance::removeTransaction");

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
void Opc_client_da_Instance::Tick()
{
	IT_IT("Opc_client_da_Instance::Tick");

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
			if(pConnect)
			{
				if(pConnect->mandare_eventi == false)
				{
					USES_CONVERSION;

					for(unsigned i = 0; i < pConnect->g_dwNumItems; i ++)
					{
						if(pConnect->Item)
						{
							DWORD dwAccessRights = pConnect->Item[i].dwAccessRights;

							dwAccessRights = dwAccessRights & OPC_WRITEABLE;

							int writable = 0;
		
							if(dwAccessRights == OPC_WRITEABLE)
							{
								writable = 1;
							}

							QString opcType;

							switch(pConnect->Item[i].vt)
							{
								case VT_EMPTY:
								{
									opcType.sprintf("%s", "VT_EMPTY");
								}
								break;
								case VT_I1:
								{
									opcType.sprintf("%s", "VT_I1");
								}
								break;
								case VT_UI1:
								{
									opcType.sprintf("%s", "VT_UI1");
								}
								break;
								case VT_I2:
								{
									opcType.sprintf("%s", "VT_I2");
								}
								break;
								case VT_UI2:
								{
									opcType.sprintf("%s", "VT_UI2");
								}
								break;
								case VT_I4:
								{
									opcType.sprintf("%s", "VT_I4");
								}
								break;
								case VT_UI4:
								{
									opcType.sprintf("%s", "VT_UI4");
								}
								break;
								case VT_I8:
								{
									opcType.sprintf("%s", "VT_I8");
								}
								break;
								case VT_UI8:
								{
									opcType.sprintf("%s", "VT_UI8");
								}
								break;
								case VT_R4:
								{
									opcType.sprintf("%s", "VT_R4");
								}
								break;
								case VT_R8:
								{
									opcType.sprintf("%s", "VT_R8");
								}
								break;
								case VT_CY:
								{
									opcType.sprintf("%s", "VT_CY");
								}
								break;
								case VT_BOOL:
								{
									opcType.sprintf("%s", "VT_BOOL");
								}
								break;
								case VT_DATE:
								{
									opcType.sprintf("%s", "VT_DATE");
								}
								break;
								case VT_BSTR:
								{
									opcType.sprintf("%s", "VT_BSTR");
								}
								break;
								case VT_VARIANT:
								{
									opcType.sprintf("%s", "VT_VARIANT");
								}
								break;
								case VT_ARRAY | VT_I1:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_I1");
								}
								break;
								case VT_ARRAY | VT_UI1:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_UI1");
								}
								break;
								case VT_ARRAY | VT_I2:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_I2");
								}
								break;
								case VT_ARRAY | VT_UI2:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_UI2");
								}
								break;
								case VT_ARRAY | VT_I4:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_I4");
								}
								break;
								case VT_ARRAY | VT_UI4:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_UI4");
								}
								break;
								case VT_ARRAY | VT_I8:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_I8");
								}
								break;
								case VT_ARRAY | VT_UI8:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_UI8");
								}
								break;
								case VT_ARRAY | VT_R4:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_R4");
								}
								break;
								case VT_ARRAY | VT_R8:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_R8");
								}
								break;
								case VT_ARRAY | VT_CY:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_CY");
								}
								break;
								case VT_ARRAY | VT_BOOL:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_BOOL");
								}
								break;
								case VT_ARRAY | VT_DATE:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_DATE");
								}
								break;
								case VT_ARRAY | VT_BSTR:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_BSTR");
								}
								break;
								case VT_ARRAY | VT_VARIANT:
								{
									opcType.sprintf("%s", "VT_ARRAY|VT_VARIANT");
								}
								break;
								default:
								{
									opcType.sprintf("%s", "Illegal");
								}
								break;
							}

							char str[10];
							// update the tags
							QString cmd;
							
							cmd = QString("update TAGS set PARAMS='");
							//const char* a = W2T(pConnect->Item[i].wszName);
							char src[150];
							strcpy(src, W2T(pConnect->Item[i].wszName));
							char dst[150];
							strip_white_space(dst, src, 150);
							
							cmd += QString(dst);
							cmd += QString(" ");
							cmd += opcType;
							cmd += QString(" ");
							cmd += QString(itoa(writable,str,10));
							cmd += "' where NAME='" + QString(pConnect->Item[i].spname) + "';";

							GetConfigureDb()->DoExec(0,cmd ,0);
						}
					}
				}

				pConnect->mandare_eventi = true;
			}
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
bool Opc_client_da_Instance::event(QEvent *e)
{
	IT_IT("Opc_client_da_Instance::event");

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

bool Opc_client_da_Instance::Connect() 
{	
	IT_IT("Opc_client_da_Instance::Connect");
	//return 0 on fail
	//retunr 1 on success

	if(pConnect) delete pConnect;
	
	pConnect = new Opc_client_da_DriverThread(this); // create the connection

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
bool  Opc_client_da_Instance::Disconnect()        
{
	IT_IT("Opc_client_da_Instance::Disconnect");
	
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
bool  Opc_client_da_Instance::DoExec(SendRecePacket *t)
{
	IT_IT("Opc_client_da_Instance::DoExec");
	
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
void Opc_client_da_Instance::Command(const QString & name, BYTE cmd, LPVOID lpPa, DWORD pa_length, DWORD ipindex) // process a command for a named unit 
{
	IT_IT("Opc_client_da_Instance::Command");

	dispatcher_extra_params* params = (dispatcher_extra_params *)lpPa;

	QString sample_point_name = QString(params->string2);

	IT_COMMENT3("Ricevuto comando per instance %s, sample point: %s, value: %lf: %s", (const char*)name, (const char*)sample_point_name, (params->res[0]).value);


	if(pConnect)
	{
		USES_CONVERSION;

		//Invio C_SC_NA_1
		char buf[sizeof(struct iec_item)];
		struct iec_item item_to_send;
		struct iec_item* p_item;
		u_int message_checksum = 0;
		int kk;

		memset(&item_to_send,0x00, sizeof(struct iec_item));

		item_to_send.iec_type = C_SC_NA_1;

		for(unsigned i = 0; i < pConnect->g_dwNumItems; i ++)
		{
			if(sample_point_name == pConnect->Item[i].spname)
			{
				//DWORD dwAccessRights = pConnect->Item[i].dwAccessRights;

				//dwAccessRights = dwAccessRights & OPC_WRITEABLE;

				//if(dwAccessRights == OPC_WRITEABLE)
				//{
					switch(pConnect->Item[i].vt)
					{
						case VT_BSTR:
						{
							strcpy(item_to_send.command_string, params->string3);
						}
						break;
						default:
						{
							item_to_send.commandValue = (params->res[0]).value;
						}
						break;
					}
					
					item_to_send.hClient = pConnect->Item[i].hClient;

					msg_sent_in_control_direction++;

					item_to_send.msg_id = msg_sent_in_control_direction;
					
					//Send message to ocp_client.exe ///////////////////////////////////////////////////////////////////
					memcpy(buf, &item_to_send, sizeof(struct iec_item));
					//////calculate checksum with checsum byte set to value zero////
										
					for(kk = 0;kk < sizeof(struct iec_item); kk++)
					{
						message_checksum = message_checksum + buf[kk];
					}
					p_item = (struct iec_item*)buf;
					p_item->checksum = message_checksum%256;
					////////////////////////////////////////////////////////////////
					fifo_put(fifo_control_direction, buf, sizeof(struct iec_item));
					//////////////////////////////////////////////////////////////////////////////

				//}
			}
		}
	}
}
