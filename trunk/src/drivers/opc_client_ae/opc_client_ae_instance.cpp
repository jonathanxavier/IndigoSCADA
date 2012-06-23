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
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
#define TICKS_PER_SEC 1
void Opc_client_ae_Instance::Start() 
{
	IT_IT("Opc_client_ae_Instance::Start");

	State = STATE_RESET;
	QString cmd = "select * from UNITS where UNITTYPE='opc_client_ae_driver' and NAME in(" + DriverInstance::FormUnitList()+ ");";
	GetConfigureDb()->DoExec(this,cmd,tListUnits);
};
/*
*Function: Stop
*Inputs:none
*Outputs:none
*Returns:none
*/
void Opc_client_ae_Instance::Stop() // stop everything under this driver's control
{	
	IT_IT("Opc_client_ae_Instance::Stop");

	pTimer->stop();

	Disconnect(); //Stop consumer thread
}
/*
*Function: QueryResponse
*Inputs:none
*Outputs:none
*Returns:none
*/
void Opc_client_ae_Instance::QueryResponse(QObject *p, const QString &c, int id, QObject*caller) // handles database responses
{
	IT_IT("Opc_client_ae_Instance::QueryResponse");
	
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
				#ifdef DEPRECATED_OPC_CLIENT_AE_CONFIG
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
				#ifdef DEPRECATED_OPC_CLIENT_AE_CONFIG
				QString IOACommand = UndoEscapeSQLText(GetConfigureDb()->GetString("DVAL"));
				#else
				int IOACommand = GetConfigureDb()->GetInt("IOA");
				#endif
				
				int command_value = 0;

				if(strlen((const char*)t.Data1) > 0)
				{
					command_value = atoi((const char*)t.Data1);
				}

				printf("Command from %s, IOA = %d, value = %d\n", (const char*)t.Data2, IOACommand, command_value);

				//Send C_SC_NA_1//////////////////////////////////////////////////////////////////////////
				struct iec_item item_to_send;
				memset(&item_to_send,0x00, sizeof(struct iec_item));
				item_to_send.iec_type = C_SC_NA_1;
				item_to_send.iec_obj.ioa = IOACommand;
				item_to_send.iec_obj.o.type45.scs = command_value;

				struct cp56time2a actual_time;
				get_utc_host_time(&actual_time);
				item_to_send.iec_obj.o.type58.time = actual_time;

				item_to_send.msg_id = msg_sent_in_control_direction++;
				item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));
				///////////////////////////////////////////////////////////////////////////////////////////

				////////////////////Middleware/////////////////////////////////////////////
				//prepare published data
				memset(&instanceSend,0x00, sizeof(iec_item_type));
				instanceSend.iec_type = item_to_send.iec_type;
				memcpy(&(instanceSend.iec_obj), &(item_to_send.iec_obj), sizeof(struct iec_object));
				instanceSend.msg_id = item_to_send.msg_id;
				instanceSend.checksum = item_to_send.checksum;

				ORTEPublicationSend(publisher);
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
void Opc_client_ae_Instance::driverEvent(DriverEvent *p)
{
	IT_IT("Opc_client_ae_Instance::driverEvent");
	
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
bool Opc_client_ae_Instance::expect(unsigned int cmd)
{
	IT_IT("Opc_client_ae_Instance::expect");

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
void Opc_client_ae_Instance::removeTransaction()
{
	IT_IT("Opc_client_ae_Instance::removeTransaction");

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
void Opc_client_ae_Instance::Tick()
{
	IT_IT("Opc_client_ae_Instance::Tick");

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
bool Opc_client_ae_Instance::event(QEvent *e)
{
	IT_IT("Opc_client_ae_Instance::event");

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

bool Opc_client_ae_Instance::Connect() 
{	
	IT_IT("Opc_client_ae_Instance::Connect");
	//return 0 on fail
	//retunr 1 on success

	if(pConnect) delete pConnect;
	
	pConnect = new Opc_client_ae_DriverThread(this); // create the connection

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
bool  Opc_client_ae_Instance::Disconnect()        
{
	IT_IT("Opc_client_ae_Instance::Disconnect");
	
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
bool  Opc_client_ae_Instance::DoExec(SendRecePacket *t)
{
	IT_IT("Opc_client_ae_Instance::DoExec");
	
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
void Opc_client_ae_Instance::Command(const QString & name, BYTE cmd, LPVOID lpPa, DWORD pa_length, DWORD ipindex) // process a command for a named unit 
{
	IT_IT("Opc_client_ae_Instance::Command");

	dispatcher_extra_params* params = (dispatcher_extra_params *)lpPa;

	QString sample_point_name = QString(params->string2);

	IT_COMMENT3("Ricevuto comando per instance %s, sample point: %s, value: %lf: %s", (const char*)name, (const char*)sample_point_name, (params->res[0]).value);


	if(pConnect)
	{
		dispatcher_extra_params* params = (dispatcher_extra_params *)lpPa;

		QString sample_point_name = QString(params->string2);

		IT_COMMENT3("Received command for instance %s, sample point: %s, value: %lf", (const char*)name, (const char*)sample_point_name, (params->res[0]).value);

		#ifdef DEPRECATED_OPC_CLIENT_AE_CONFIG
		QString pc = "select * from PROPS where IKEY='" + sample_point_name + "';"; 
		#else
		QString pc = "select * from TAGS where NAME='" + sample_point_name + "';";
		#endif

		QString value_for_command;
		value_for_command.sprintf("%lf", (params->res[0]).value);
		// 
		GetConfigureDb()->DoExec(this, pc, tGetIOAfromSamplePointName, value_for_command, sample_point_name);
	}
}

/////////////////////////////////////Middleware///////////////////////////////////////////
Boolean  quite=ORTE_FALSE;
int	regfail=0;

//event system
void onRegFail(void *param) 
{
  printf("registration to a manager failed\n");
  regfail = 1;
}

void rebuild_iec_item_message(struct iec_item *item2, iec_item_type *item1)
{
	unsigned char checksum;

	///////////////Rebuild struct iec_item//////////////////////////////////
	item2->iec_type = item1->iec_type;
	memcpy(&(item2->iec_obj), &(item1->iec_obj), sizeof(struct iec_object));
	item2->cause = item1->cause;
	item2->msg_id = item1->msg_id;
	item2->ioa_control_center = item1->ioa_control_center;
	item2->casdu = item1->casdu;
	item2->is_neg = item1->is_neg;
	item2->checksum = item1->checksum;
	///////and check the 1 byte checksum////////////////////////////////////
	checksum = clearCrc((unsigned char *)item2, sizeof(struct iec_item));

//	fprintf(stderr,"new checksum = %u\n", checksum);

	//if checksum is 0 then there are no errors
	if(checksum != 0)
	{
		//log error message
		ExitProcess(0);
	}

	/*
	fprintf(stderr,"iec_type = %u\n", item2->iec_type);
	fprintf(stderr,"iec_obj = %x\n", item2->iec_obj);
	fprintf(stderr,"cause = %u\n", item2->cause);
	fprintf(stderr,"msg_id =%u\n", item2->msg_id);
	fprintf(stderr,"ioa_control_center = %u\n", item2->ioa_control_center);
	fprintf(stderr,"casdu =%u\n", item2->casdu);
	fprintf(stderr,"is_neg = %u\n", item2->is_neg);
	fprintf(stderr,"checksum = %u\n", item2->checksum);
	*/
}

void recvCallBack(const ORTERecvInfo *info,void *vinstance, void *recvCallBackParam) 
{
	Opc_client_ae_Instance * cl = (Opc_client_ae_Instance*)recvCallBackParam;
	iec_item_type *item1 = (iec_item_type*)vinstance;

	switch (info->status) 
	{
		case NEW_DATA:
		{
		  if(!quite)
		  {
			  struct iec_item item2;
			  rebuild_iec_item_message(&item2, item1);
			  //TODO: detect losts messages when item2.msg_id are NOT consecutive
//			  cl->get_items(&item2);
			  //TODO: remove comment of the next statement
			  // fifo_put(cl->fifo_monitor_direction, (char *)&item2, sizeof(struct iec_item));
		  }
		}
		break;
		case DEADLINE:
		{
			printf("deadline occurred\n");
		}
		break;
	}
}

void Opc_client_ae_Instance::get_items(struct iec_item* p_item)
{
	printf("Receiving %d th opc da message from line = %d\n", p_item->msg_id, instanceID + 1);

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
	
	unsigned char rc = clearCrc((unsigned char *)p_item, sizeof(struct iec_item));

	if(rc != 0)
	{
		ExitProcess(1);
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

			value.sprintf("%f", p_item->iec_obj.o.type36.mv);

			#endif
		}
		break;
		case M_IT_TB_1:
		{
			#ifdef USE_IEC_TYPES_AND_IEC_TIME_STAMP

			iec_type37 var = p_item->iec_obj.o.type37;
			
			SpValue v(VALUE_TAG, &var, M_ME_TN_1);
			TODO:05-07-2011 Get name here
			post_val(v, name);

			#else

			value.sprintf("%d", p_item->iec_obj.o.type37.counter);

			#endif
		}
		break;
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

	#ifdef DEPRECATED_OPC_CLIENT_DA_CONFIG
	QString cmd = "select IKEY from PROPS where DVAL='"+ ioa + "' and SKEY='SAMPLEPROPS';";
	#else
	QString cmd = "select NAME from TAGS where IOA="+ ioa + " and UNIT='"+ Name + "';";
	#endif

	GetConfigureDb()->DoExec(this, cmd, tGetSamplePointNamefromIOA, value, ioa);

	//printf("ioa %s, value %s\n", (const char*)ioa, (const char*)value);
}

#include <time.h>
#include <sys/timeb.h>

void Opc_client_ae_Instance::get_utc_host_time(struct cp56time2a* time)
{
	struct timeb tb;
	struct tm	*ptm;
		
	IT_IT("get_utc_host_time");

    ftime (&tb);
	ptm = gmtime(&tb.time);
		
	time->hour = ptm->tm_hour;					//<0..23>
	time->min = ptm->tm_min;					//<0..59>
	time->msec = ptm->tm_sec*1000 + tb.millitm; //<0..59999>
	time->mday = ptm->tm_mday; //<1..31>
	time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
	time->month = ptm->tm_mon + 1; //<1..12>
	time->year = ptm->tm_year - 100; //<0..99>
	time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
	time->su = (u_char)tb.dstflag; //<0..1> SUmmer time: <0> is standard time, <1> is summer time

	IT_EXIT;
    return;
}
/////////////////////////////////////Middleware/////////////////////////////////////////////