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

/*
*Header For: Archiver engine 
*/
#include "qplatformdefs.h"
#include "common.h"
#include "archiver.h"
#include "messages.h"
#include "results.h"
#include "utilities.h"
#include "dispatch.h"
#include "general_defines.h"
#include "IndentedTrace.h"

//bitmaps
#include "logout.xpm"
#include "start.xpm"
#include "quit.xpm"
#include "db.xpm"

/*
*Function: Archiver
*Inputs:none
*Outputs:none
*Returns:none
*/
Archiver * Archiver::Instance = 0;
//
Archiver::Archiver(QObject *parent) : QObject(parent),
fStarted(false),SequenceNumber(0),fHalt(0),translation(0),
MaxRetryReconnectToDispatcher(0),MaxRetryReconnectToRealTimeDb(0),
MaxRetryReconnectToHistoricDb(0),MaxRetryReconnectToSpareDispatcher(0),
MaxRetryReconnectToSpareRealTimeDb(0)
{
	IT_IT("Archiver::Archiver");
	
	Instance = this;
	//MidnightReset = 1;

	// Connect to the databases
	//real time
	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (ConfigQueryResponse (QObject *, const QString &, int, QObject*)));	
	//real time
	connect (GetCurrentDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (CurrentQueryResponse (QObject *, const QString &, int, QObject*)));	
	//real time
	connect (GetResultDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (ResultsQueryResponse (QObject *, const QString &, int, QObject*)));

	if(GetHistoricResultDb() != NULL)
	{
		//historic
		connect (GetHistoricResultDb (),
		SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
		SLOT (HistoricResultsQueryResponse (QObject *, const QString &, int, QObject*)));
	}

	//Connect to dispatcher on Host A (primary)
	connect (GetDispatcher(),
	SIGNAL (ReceivedNotify(int, const char *)), this,
	SLOT (ReceivedNotify(int, const char *)));

	if(GetSpareDispatcher() != NULL)
	{
		//Connect to dispatcher on Host B (spare)
		connect (GetSpareDispatcher(),
		SIGNAL (ReceivedNotify(int, const char *)), this,
		SLOT (ReceivedNotify(int, const char *)));
	}

	ResetTables(); 
	//
	//pSchedule =  new Schedule(this); // create the event scheduler task
	//
	//
	QTimer *pT = new QTimer(this);
	connect(pT,SIGNAL(timeout()),this,SLOT(Tick()));
	pT->start(2000); // run every 2 seconds 
	//	
};
/*-Function:~Archiver
*Inputs:none
*Outputs:none
*Returns:none
*/
Archiver::~Archiver()
{
	IT_IT("Archiver::~Archiver");
	
	Stop();
	Instance = 0;
};
/*
*Function:UpdateCurrentValue
*Inputs:sample point to update
*Outputs:none
*Returns:none
*/
/*
void Archiver::UpdateCurrentValue( const QString &name, SamplePoint &sp)
{
	IT_IT("Archiver::UpdateCurrentValue");
	
	//
	// update the sample point
	//	
	QString cmd;
	QString updated = (const char*)0;
	//
	// now update the current tags
	// 
	SamplePoint::TagDict::iterator i = sp.Tags.begin();
	for(;!(i == sp.Tags.end());i++)
	{
		if((*i).second.changed && (*i).second.enabled) // updated ?
		{ 
			(*i).second.changed = 0;
			cmd = "update TAGS_DB set ";
			cmd += "UPDTIME=";
				
			updated = QDATE_TIME_ISO_DATE((*i).second.updated);

			cmd += updated;
			cmd += ",VAL="+QString::number((*i).second.value); 
			cmd += ",MINVAL="+QString::number((*i).second.stats.Min());
			cmd += ",MAXVAL="+QString::number((*i).second.stats.Max());
			cmd += ",SUMVAL="+QString::number((*i).second.stats.sum());
			cmd += ",SUM2VAL="+QString::number((*i).second.stats.sum2());
			cmd += ",NVAL="+QString::number((*i).second.stats.samples());
			cmd += ",STATE="+QString::number((*i).second.state);
			cmd += ",SEQNO="+QString::number(SequenceNumber); // machine independent idea of time ordering
			cmd += " where NAME='"+name+"' and TAGNAME='"+(*i).first+"';";  
			GetCurrentDb()->DoExec(0,cmd,0);
		};
	};

	if(updated.isNull())
	{
		updated = DATETIME_NOW;
	}

	cmd = "update CVAL_DB set UPDTIME=" + updated + ", ALMTIME=" + QDATE_TIME_ISO_DATE(sp.alarmtime)
	+ ",NMEASURE="+QString::number(sp.nmeasures)+",NALARM="+QString::number(sp.nalarms)+
	",NWARNING="+QString::number(sp.nwarnings)+",STATE="+QString::number(sp.AlarmState)+
	",COMMENT='"+ EscapeSQLText(sp.Comment)+"'" +
	",FAILTIME="+QDATE_TIME_ISO_DATE(sp.failtime);
	//
	if(sp.fAckTriggered)
	{
		cmd += ",ACKFLAG="+ QString::number(sp.fAckTriggered);
		sp.fAckTriggered = 0;
	};
	cmd += ",SEQNO="+QString::number(SequenceNumber);
	cmd += " where NAME='"+name+"';";
	GetCurrentDb()->DoExec(this,cmd,tUpdateDone,name);
	//
	sp.fChanged = 0; // mark as not changed
	//
	//
};

*/

/*
*Function: Tick
*Inputs:none
*Outputs:none
*Returns:none
*/
void Archiver::Tick()
{
	IT_IT("Archiver::Tick");

	if(GetSpareDispatcher() != NULL)
	{
		if(!GetSpareDispatcher()->Ok())
		{
			QSLogEvent("Archiver", "Spare dispatcher client connection error");
			QSLogEvent("Archiver", "Attempt to restore connection with spare dispatcher server");

			DisconnectFromSpareDispatcher();
			ConnectToSpareDispatcher();
			++MaxRetryReconnectToSpareDispatcher;
		}
		else
		{
			MaxRetryReconnectToSpareDispatcher = 0;
		}
	}

	if(!GetDispatcher()->Ok())
	{
		QSLogEvent("Archiver", "Dispatcher client connection error");
		QSLogEvent("Archiver", "Attempt to restore connection with dispatcher server");

		DisconnectFromDispatcher();
		ConnectToDispatcher();
		++MaxRetryReconnectToDispatcher;
	}
	else
	{
		MaxRetryReconnectToDispatcher = 0;
	}

	if(!GetConfigureDb()->Ok() || !GetResultDb()->Ok() || !GetCurrentDb()->Ok())
	{
		if(!GetConfigureDb()->Ok())
		{
			QString msg = QString("Real time client error: ") + GetConfigureDb()->GetErrorMessage();
			QSLogEvent("Archiver", msg);
			GetConfigureDb()->AcnoledgeError();
		}

		if(!GetResultDb()->Ok())
		{
			QString msg = QString("Real time client error: ") + GetResultDb()->GetErrorMessage();
			QSLogEvent("Archiver", msg);	
			GetResultDb()->AcnoledgeError();
		}
		
		if(!GetCurrentDb()->Ok())
		{
			QString msg = QString("Real time client error: ") + GetCurrentDb()->GetErrorMessage();
			QSLogEvent("Archiver", msg);	
			GetCurrentDb()->AcnoledgeError();
		}

		QSLogEvent("Archiver", "Attempt to restore connection with realtime database server");

		DisconnectFromRealTimeDatabases();
		ConnectToRealTimeDatabases();
		++MaxRetryReconnectToRealTimeDb;
	}
	else
	{
		MaxRetryReconnectToRealTimeDb = 0;
	}

	if((GetSpareConfigureDb() != NULL) && (GetSpareCurrentDb() != NULL)&&(GetSpareResultDb() != NULL))
	{
		if(!GetSpareConfigureDb()->Ok() || !GetSpareResultDb()->Ok() || !GetSpareCurrentDb()->Ok())
		{
			if(!GetSpareConfigureDb()->Ok())
			{
				QString msg = QString("Spare real time client error: ") + GetSpareConfigureDb()->GetErrorMessage();
				QSLogEvent("Monitor", msg);
				GetSpareConfigureDb()->AcnoledgeError();
			}

			if(!GetSpareResultDb()->Ok())
			{
				QString msg = QString("Spare real time client error: ") + GetSpareResultDb()->GetErrorMessage();
				QSLogEvent("Monitor", msg);	
				GetSpareResultDb()->AcnoledgeError();
			}
			
			if(!GetSpareCurrentDb()->Ok())
			{
				QString msg = QString("Spare real time client error: ") + GetSpareCurrentDb()->GetErrorMessage();
				QSLogEvent("Monitor", msg);	
				GetSpareCurrentDb()->AcnoledgeError();
			}

			QSLogEvent("Monitor", "Attempt to restore connection with spare realtime database server");

			DisconnectFromSpareRealTimeDatabases();
			ConnectToSpareRealTimeDatabases();
			++MaxRetryReconnectToSpareRealTimeDb;
		}
		else
		{
			MaxRetryReconnectToSpareRealTimeDb = 0;
		}
	}

	if(GetHistoricResultDb() != NULL)
	{
		if(!GetHistoricResultDb()->Ok())
		{
			QString msg = QString("Historical client error: ") + GetHistoricResultDb()->GetErrorMessage();
			QSLogEvent("Archiver", msg);
			GetHistoricResultDb()->AcnoledgeError();
			QSLogEvent("Archiver", "Attempt to restore connection with historical database server");

			DisconnectFromHistoricDatabases();
			ConnectToHistoricDatabases();
			++MaxRetryReconnectToHistoricDb;
		}
		else
		{
			MaxRetryReconnectToHistoricDb = 0;
		}
	}

	if((MaxRetryReconnectToHistoricDb > 50) || 
		(MaxRetryReconnectToRealTimeDb > 50) ||
		(MaxRetryReconnectToDispatcher > 50) || 
		(MaxRetryReconnectToSpareRealTimeDb > 50) ||
		(MaxRetryReconnectToSpareDispatcher > 50) 
		)
	{
		//We have no more connection with some server
		//What do we do?
	}


	//
	// update alarm groups    
	bool fUpdated = false;
	SequenceNumber++; // increment the tick count
	// 
	Results::GroupDict &g = Results::GetGroups();
	Results::GroupDict::iterator i = g.begin();
	//
	for(; !(i == g.end()); i++)
	{
		// 
		// walk the group table 
		if((*i).second.Changed)
		{
			(*i).second.Changed = 0;
			(*i).second.State = 0;
			//
			Results::StateDict::iterator j = (*i).second.begin();

			/*
			for(; !(j == (*i).second.end());j++)
			{
				//
				// now update those sample points that are member of an alarm group 
				// that have not yet been updated
				SamplePointDictWrap &d = Results::GetEnabledPoints();
				SamplePointDictWrap::iterator k = d.find((*j).first);
				//
				if(!(k == d.end()))
				{
					if((*k).second.fChanged)
					{
						QString cmd = "update ALM_GRP_STATE set STATE=" + QString::number((*k).second.AlarmState);
						cmd += ",UPDTIME=";
						cmd += DATETIME_NOW;
						if((*k).second.fAckTriggered)
						{
							cmd += ", ACKSTATE=1";
						};
						cmd += ",SEQNO="+QString::number(SequenceNumber); // machine independent idea of time ordering
						cmd +=  " where SPNAME='" + (*k).first +"';";
						GetCurrentDb()->DoExec(0,cmd,0); // do it
						//
						// now write the sample point's current values out
						//
						//UpdateCurrentValue((*k).first,(*k).second);
						(*k).second.fChanged = 0;
						//
						//
					}
					// track largest alarm level
					if((*k).second.AlarmState > (*i).second.State)
					{
						(*i).second.State = (*k).second.AlarmState;
					}            
				}
			}

			// write it out  
			QString cmd = "update ALM_GRP set STATE=" + QString::number((*i).second.State);
			cmd += ",UPDTIME=";
			cmd +=  DATETIME_NOW;
			if((*i).second.AckState)
			{
				cmd += ", ACKSTATE=1"; // set the ack flag if so triggered
				(*i).second.AckState = 0;
			}	      
			cmd += ",SEQNO="+QString::number(SequenceNumber); // machine independent idea of time ordering
			cmd += " where NAME='"+(*i).first+"';";
			//
			fUpdated=true;
			GetCurrentDb()->DoExec(0,cmd,0); // do it

			*/
		}    
	}
	//
	// now write out the sample point's current states that do not belong to a group
	//
	{
		SamplePointDictWrap &d = Results::GetEnabledPoints();
		SamplePointDictWrap::iterator k = d.begin();
		for(;!(k == d.end());k++)
		{
			if((*k).second.fChanged)
			{
				//UpdateCurrentValue((*k).first,(*k).second);
				(*k).second.fChanged = 0;
				fUpdated = true;
			}
		}
	}
	//
	//
	//
	//
	if(fUpdated)
	{
		//GetCurrentDb()->DoExec(this,"select * from ALM_GRP limit 1;",tAllUpdated); //from 2.51

		/*
		if(MidnightReset)
		{
			if((lastHour == 23) && (QTime::currentTime().hour() == 0))
			{
				//
				// reset the stats
				ResetStatistics();
				// 
			}
			lastHour = QTime::currentTime().hour();
		}
		*/
	}

	//broadcast
	GetDispatcher()->DoExec(NotificationEvent::ARCHIVER_TICK_NOTIFY, (const char*)QString::number(SequenceNumber));
	//
	// Handle request to halt
	//
	if(fHalt)
	{
		Stop();
		QTimer::singleShot(2000,qApp,SLOT(quit()));
	}
}
/*
*Function: Start
*tell all drivers to start
*Inputs:none
*Outputs:none
*Returns:none
*/
void Archiver::Start()
{
	IT_IT("Archiver::Start");
	//
	// start everything
	//
	if(!fStarted)
	{ 
		/*
		DDict::iterator i = drivers.begin();
		for(; !(i == drivers.end()); i++)
		{
			if((*i).second)
			{
				IT_COMMENT2("Archiver Starting", "%s",(const char *)((*i).first));

				QSMessage(tr("Starting") + " " + (*i).first); 
				QSLogEvent("Archiver",tr("Starting") + " "+ (*i).first);  
				(*i).second->Start(); // signal the driver to start 
			};
		};
		*/

		//
		// log we have started archiving
		//
		QSLogEvent("Archiver",tr("Archiver started"));
		//
		fStarted = true;
		//
		//broadcast
		GetDispatcher()->DoExec(NotificationEvent::ARCHIVER_STARTED_NOTIFY); // notify started archiving
	};
};
/*
*Function: Stop
*Stop all drivers
*Inputs:none
*Outputs:none
*Returns:none
*/
void Archiver::Stop()
{
	IT_IT("Archiver::Stop");

	if(fStarted)
	{
		/*
		DDict::iterator i = drivers.begin();

		for(; !(i == drivers.end()); i++)
		{
			if((*i).second)
			{
				IT_COMMENT2("Archiver Stopping", "%s",(const char *)((*i).first));

				QSMessage(tr("Stopping") + " " + (*i).first);   
				(*i).second->Stop(); // signal the driver to stop
			};
		};
		drivers.clear();
		fStarted = false;
		*/

		//broadcast
		GetDispatcher()->DoExec(NotificationEvent::ARCHIVER_STOPPED_NOTIFY);
		QSLogEvent("Archiver",tr("Stopped Archiving"));
		//
		//DriverInstance::Props.clear(); // clear and properties or semaphores
	}
};
/*
*Function:ConfigQueryResponse
*Inputs:client , command, transactionid
*Outputs:none
*Returns:none
*/
void Archiver::ConfigQueryResponse (QObject *p,const QString &c, int id, QObject* caller)  // handles configuration responses
{
	if(p != this) return;

	IT_IT("Archiver::ConfigQueryResponse");

	switch(id)
	{
		case tUnitTypes:
		{
			//Stop();
			// we now have a list of drivers to load - load them 
			// set a one shot to go off in a second or so
			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				/*
				int n = GetConfigureDb()->GetNumberResults();
				//
				// Load the drivers
				//
				for(int i = 0 ; i < n ; i++, GetConfigureDb()->FetchNext())
				{
					QString s = GetConfigureDb()->GetString("UNITTYPE");
					Driver *p = FindDriver(s);
					if(p)
					{
						QSLogEvent("Archiver",tr("Loading Module") + " " +s);
						DDict::value_type pr(s,p);
						drivers.insert(pr); // put in the dictionary
						//
						QObject::connect(this,SIGNAL(DoCommand(const QString &, BYTE, LPVOID , DWORD, DWORD)),
						p,SLOT(Command(const QString &, BYTE , LPVOID, DWORD, DWORD ))); // connect the command slot
						//
						// connect the trace line to the top level application
						// 
						QObject::connect(p,SIGNAL(TraceOut(const QString &,const QString &)),
						this,SLOT(Trace(const QString &, const QString &)));
					};    
				};

				*/

				//
				// now we get the receipe if it is not (default)
				//
				Results::GetEnabledPoints().clear(); //<------crash on exit of Archiver applivation 17-05-2005 APA. With STLPort no more crash
				//DriverInstance::EnabledUnits.clear(); 
				//
				//if(GetReceipeName() != "(default)")
				//{
				//	QString cmd = "select * from RECEIPE where NAME='"+ GetReceipeName() +"';";
				//	GetConfigureDb()->DoExec(this,cmd,tReceipeRecord); // request the record
				//}
				//else
				{ 
					//
					// the default enabled units and sample points
					// 
					GetConfigureDb()->DoExec(this,"select * from UNITS where ENABLED=1;",tUnits);
					GetConfigureDb()->DoExec(this,"select * from SAMPLE where ENABLED=1;",tSamples);
				};
			};
		};
		break;
		case tReceipeRecord:
		{
			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				/*
				// we now have the list of enabled units and stuff
				QString s = GetConfigureDb()->GetString("UNITS"); // enabled units
				QTextIStream is(&s);
				is >> ws;
				while (!is.atEnd ())
				{
					//
					QString a;
					is >> a;
					DriverInstance::EnabledUnits << a.stripWhiteSpace ();
				};
				*/
				//
				// we now have the list of enabled sample points 
				QString s = GetConfigureDb()->GetString("SAMPLES"); // enabled sample points
				QTextIStream ss(&s);
				ss >> ws;
				//
				SamplePointDictWrap &d = Results::GetEnabledPoints();
				d.clear();
				//
				while (!ss.atEnd ())
				{
					//
					QString a;
					ss >> a;
					//
					//SamplePoint smp(GetConfigureDb()->GetString("NAME"));
					//SamplePointDictWrap::value_type pr(GetConfigureDb()->GetString("NAME"),smp);
					SamplePoint smp(a.stripWhiteSpace());
					SamplePointDictWrap::value_type pr(a.stripWhiteSpace(), smp);

					d.insert(pr);
				};
				//
				//QString cmd = "select * from SAMPLE where NAME in (" + DriverInstance::FormSamplePointList() + ");";
				//GetConfigureDb()->DoExec(this,cmd,tSamples); // get the sample point configuration
				//
			};
		};
		break;
		case tUnits:
		{
			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				int n = GetConfigureDb()->GetNumberResults();
				for(int i = 0; i < n; i++, GetConfigureDb()->FetchNext())
				{
					//DriverInstance::EnabledUnits << GetConfigureDb()->GetString("NAME");
				};
			};
		};
		break;
		case tSamples:
		{
			//vengono caricati i sample points abilitati
						
			Results::GetEnabledPoints().clear();  //<------crash on exit of Archiver applivation 17-05-2005 APA With STLPort no more crash

			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				int n = GetConfigureDb()->GetNumberResults();

				for(int i = 0; i < n; i++, GetConfigureDb()->FetchNext())
				{
					SamplePoint s(GetConfigureDb()->GetString("NAME"));
					SamplePointDictWrap &d = Results::GetEnabledPoints();
					SamplePointDictWrap::value_type pr(GetConfigureDb()->GetString("NAME"),s);
					
					d.insert(pr); //10-10-09 ,questa va in crash
	 				SamplePointDictWrap::iterator j = d.find(GetConfigureDb()->GetString("NAME"));
					//
					(*j).second.Unit = GetConfigureDb()->GetString("UNIT"); // which unit is assicated with this SP
					(*j).second.Fileable = GetConfigureDb()->GetBool("FILEABLE"); // do we store the results
					(*j).second.Retriggerable = GetConfigureDb()->GetBool("RETRIGGER"); // do we have retriggerable alarms
					(*j).second.AlarmThreshold = GetConfigureDb()->GetInt("ALARMTHRESHOLD"); // what is the alarm threshold count
					(*j).second.Type = GetConfigureDb()->GetString("QTYPE"); // the type
					(*j).second.InputIndex = GetConfigureDb()->GetString("IPINDEX"); // input index
					//
				}
				//
				if(GetReceipeName() != "(default)")
				{
					QString cmd = "select * from TAGS where ((RECEIPE='" + GetReceipeName() + 
					"') or (RECEIPE='(default)')) order by RECEIPE asc;";
					
					GetConfigureDb()->DoExec(this,cmd,tTags); // get all the tags used 
				}
				else
				{
					GetConfigureDb()->DoExec(this,
					"select * from TAGS where RECEIPE='(default)';",tTags); // get all the tags used 
				}
				// the (default) receipe will come first then the current receipe
			}
		}
		break;
		case tTags:
		{
			//vengono caricati i tags abilitati
			/*			
			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				int n = GetConfigureDb()->GetNumberResults();
				for(int i = 0; i < n; i++, GetConfigureDb()->FetchNext())
				{
					SamplePointDictWrap &d = Results::GetEnabledPoints();
					SamplePointDictWrap::iterator j = d.find(GetConfigureDb()->GetString("NAME"));

					if(!(j == d.end()))
					{
						SamplePoint::TagDict::iterator k = (*j).second.Tags.find(GetConfigureDb()->GetString("TAG"));
						if(k == (*j).second.Tags.end())
						{
							SamplePoint::TagDict::value_type pr(GetConfigureDb()->GetString("TAG"),TagItem());
							(*j).second.Tags.insert(pr);
							k = (*j).second.Tags.find(GetConfigureDb()->GetString("TAG"));
						}
						//
						// set the alarm limits
						//
						//   
						(*k).second.UpperAlarm.Enabled = GetConfigureDb()->GetBool("UAENABLE");
						(*k).second.UpperAlarm.Limit= GetConfigureDb()->GetDouble("UPPERALARM");
						//
						(*k).second.UpperWarning.Enabled = GetConfigureDb()->GetBool("UWENABLE");
						(*k).second.UpperWarning.Limit= GetConfigureDb()->GetDouble("UPPERWARN");
						//
						(*k).second.LowerWarning.Enabled = GetConfigureDb()->GetBool("LWENABLE");
						(*k).second.LowerWarning.Limit= GetConfigureDb()->GetDouble("LOWERWARN");
						//
						(*k).second.LowerAlarm.Enabled = GetConfigureDb()->GetBool("LAENABLE");
						(*k).second.LowerAlarm.Limit= GetConfigureDb()->GetDouble("LOWERALARM");  
						//
						//
						(*k).second.enabled = GetConfigureDb()->GetInt("ENABLED") ? true : false; 
						//
						//
					}
				}
				//
				QString cmd = "select * from CVAL_DB;";
				GetCurrentDb()->DoExec(this,cmd,tSamplesCurrent); // get the sample point current values
				//
				// now get the tags afterwards
				GetCurrentDb()->DoExec(this,"select * from TAGS_DB;",tTagsCurrent); // after this has completed we start
				//
			}
			*/
		}
		break;
		/*
		case tReceipe:
		{
			IT_COMMENT("tReceipe");
			
			// set the receipe name
			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				for(unsigned i = 0; i < GetConfigureDb()->GetNumberResults(); i++,GetConfigureDb()->FetchNext())
				{
					if(GetConfigureDb()->GetString("IKEY") == "Receipe")
					{
						QSLogEvent("Archiver",tr("Setting Receipe to") + " " + UndoEscapeSQLText(GetConfigureDb()->GetString("DVAL")));  
						SetReceipeName (UndoEscapeSQLText(GetConfigureDb()->GetString("DVAL"))); 
					}
					else if(GetConfigureDb()->GetString("IKEY") == "MidnightReset")
					{
//						MidnightReset = GetConfigureDb()->GetInt("DVAL");
					};

				};
			};
		};
		break;
		*/
		default:
		break;
	};
};

/*
*Function:HistoricResultsQueryResponse
*for talking to the results database
*Inputs:client, command, transaction id
*Outputs:none
*Returns:none
*/
void Archiver::HistoricResultsQueryResponse (QObject *p,const QString &c, int id, QObject* caller) // results responses
{
	if(p != this) return;

	IT_IT("Archiver::HistoricResultsQueryResponse");
};

/*
*Function:ResultsQueryResponse
*for talking to the results database
*Inputs:client, command, transaction id
*Outputs:none
*Returns:none
*/
void Archiver::ResultsQueryResponse (QObject *p,const QString &c, int id, QObject* caller) // results responses
{
	if(p != this) return;

	IT_IT("Archiver::ResultsQueryResponse");
};

/*
*Function: CurrentQueryResponse
*talking to the current values database
*Inputs:client, command, transaction id
*Outputs:none
*Returns:none
*/
void Archiver::CurrentQueryResponse (QObject *p,const QString &c, int id, QObject* caller) // current value responses
{
	if(p != this) return;

	IT_IT("Archiver::CurrentQueryResponse");
/*
	switch(id)
	{
		//case tUpdateDone:
		//{
			//QSTransaction &t = GetCurrentDb()->CurrentTransaction();
			//broadcast
			//GetDispatcher()->DoExec(NotificationEvent::UPDATE_NOTIFY,(const char*) t.Data1);
		//}
		//break;
		//case tAllUpdated: // post a notification to update alarm and current values
		//{
			//broadcast
			//GetDispatcher()->DoExec(NotificationEvent::ALARMGROUP_NOTIFY);
			//GetDispatcher()->DoExec(NotificationEvent::CURRENT_NOTIFY);
		//}
		//break;

		case tTagsCurrent:
		{	
			// get the current tag values
			
			if(GetCurrentDb()->GetNumberResults() > 0)
			{
				int n = GetCurrentDb()->GetNumberResults();
				for(int i = 0; i < n; i++, GetCurrentDb()->FetchNext())
				{
					// 
					// update the current values with the values from the last time we started
					//
					SamplePointDictWrap &d = Results::GetEnabledPoints();
					SamplePointDictWrap::iterator k =  d.find(GetCurrentDb()->GetString("NAME"));
					if(!(k == d.end()))
					{
						SamplePoint::TagDict::iterator j = (*k).second.Tags.find(GetCurrentDb()->GetString("TAGNAME"));
						if(!(j == (*k).second.Tags.end()))
						{
							if((*j).second.enabled)
							{
								(*j).second.value = GetCurrentDb()->GetDouble("VAL"); // current value
								(*j).second.state = GetCurrentDb()->GetInt("STATE"); // current state
								(*j).second.updated =  GetCurrentDb()->GetDateTime("UPDTIME"); // when it was updated
								(*j).second.stats.reset();
								//(*j).second.stats.set
								//(
								//GetCurrentDb()->GetInt("NVAL"),
								//GetCurrentDb()->GetDouble("SUMVAL"),
								//GetCurrentDb()->GetDouble("SUMVAL2"),
								//GetCurrentDb()->GetDouble("MINVAL"),
								//GetCurrentDb()->GetDouble("MAXVAL")
								//); // the stats
							}
						}  
					}
					
					//16-08-2003: Reset of TAGS_DB
					GetCurrentDb()->DoExec(0, "update TAGS_DB set SEQNO=0 where NAME='"+ GetCurrentDb()->GetString("NAME") +"' and TAGNAME='"+ GetCurrentDb()->GetString("TAGNAME") +"';",0);
				}
			}

			Start(); // we can go now
		}
		break;
		default:
		break;
	};
*/
};

/*
*Function: ReceivedNotify
*notifications to control the montior task
*Inputs:notification code
*Outputs:none
*Returns:none
*/
void Archiver::ReceivedNotify(int ntf, const char * data)
{
	IT_IT("Archiver::ReceivedNotify");

	switch(ntf)
	{
		case NotificationEvent::CMD_ARCHIVER_START:
		{
			IT_COMMENT("CMD_ARCHIVER_START - archving start command is received");
			// start by getting the unit types -> drivers
			ResetTables(); 
			//pSchedule->Restart();
			//
		}
		break;
		case NotificationEvent::CMD_ARCHIVER_STOP:
		{
			IT_COMMENT("CMD_ARCHIVER_STOP - archving stop command is received");
			Stop(); 
		}
		break;
		case  NotificationEvent::CMD_SHUTDOWN_ARCHIVER:
		{
			QTimer::singleShot(500,qApp,SLOT(quit())); // quit in 1 seconds
		}
		break;
		default:
		break;
	}
};


/*
*Function: ResetStatistics()
*Inputs:none
*Outputs:none
*Returns:none
*/
/*
void Archiver::ResetStatistics()
{
	IT_IT("Archiver::ResetStatistics");
	
	SamplePointDictWrap &d = Results::GetEnabledPoints();
	SamplePointDictWrap::iterator k =  d.begin();
	//
	for(;!(k == d.end());k++)
	{
		SamplePoint::TagDict::iterator j = (*k).second.Tags.begin();

		for(;!(j == (*k).second.Tags.end());j++)
		{
			(*j).second.stats.reset();
		};  
	};
	
	//16-08-2003 Ho disabilitato la sucessiva update

	//QString cmd = "update CVAL_DB set NMEASURE=0,NALARM=0,NWARNING=0;";
	//GetCurrentDb()->DoExec(0,cmd,0);

	//QSLogEvent("Archiver",tr("Reset Statistics"));
	//
};

*/

/*
*Function:ResetTables
*Inputs:none
*Outputs:none
*Returns:none
*/
void Archiver::ResetTables()
{
	IT_IT("Archiver::ResetTables");
	
	//GetCurrentDb()->DoExec(0,"delete from ALM_GRP;",0);
	//GetCurrentDb()->DoExec(0,"delete from ALM_GRP_STATE;",0);

	//GetConfigureDb()->DoExec(this,"select * from ALARMGROUP;",tAlarmGroups); // build the alarm group
	//GetConfigureDb()->DoExec(this,"select * from PROPS where SKEY='System';",tReceipe);
	
	GetConfigureDb()->DoExec(this,"select UNITTYPE from UNITS;",tUnitTypes); 
	
	//broadcast
	//GetDispatcher()->DoExec(NotificationEvent::CURRENT_NOTIFY);
	//GetDispatcher()->DoExec(NotificationEvent::ALARMGROUP_NOTIFY);
};
//
/*
*Function:Trace
*Inputs:message source, message text
*Outputs:none
*Returns:none
*/
void Archiver::Trace(const QString &src,const QString &msg)
{   
	IT_IT("Archiver::Trace");

	// now forward to attached interfaces
	QSTrace( src + ": " + msg);
	
	emit TraceOut(src,msg); // forward it
};
//
//
//
//
// ********************** STARTUP **********************************************************
//
#ifdef UNIX
void SigTermHandler(int)
{
	IT_IT("SigTermHandler");

	Archiver::Instance->fHalt = true;
};
#endif

#define ARCHIVER_CAN_RUN_AS_ROOT

//apa NOTE: 20-04-2011 Archiver is in pre-alpha development level 
//
int main(int argc, char **argv)
{
	IT_IT("main - ARCHIVER");

	int stat = -1;

	char version[100];
	//version control///////////////////////////////////////////////////////////////
    sprintf(version, "archiver.exe - Build: %s %s at enscada.com",__DATE__,__TIME__);
    fprintf(stdout, "%s\n", version);
	SYSTEMTIME oT;
	::GetLocalTime(&oT);
	fprintf(stdout,"%02d/%02d/%04d, %02d:%02d:%02d Starting ... %s\n",oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,SYSTEM_NAME_ARCHIVER); 
	fflush(stdout);
		
	if(!IsSingleInstance(""SYSTEM_NAME_ARCHIVER""))
	{
		IT_COMMENT("Another instance of the archiver is already running!");//error message
		return stat;
	}
	
		
	SetScadaHomeDirectory(argv[0]);
	// 
	// if the DISPLAY variable is empty or not set then we go into none-GUI mode
	// this application can run GUI mode or non-GUI mode
	// it is expected that the GUI mode is started when we want to do debugging
	//   
	#ifdef UNIX
	bool useGUI = (getenv( "DISPLAY" ) != 0) && (strlen(getenv( "DISPLAY" )) > 0);
	//
	//
	if(!useGUI)
	{
		setenv("DISPLAY","localhost:0",1); 
	};
	//
	QApplication a(argc, argv,useGUI);
	#else
	QApplication a(argc, argv);
	#endif
	//
	if(!chdir(QSFilename(""))) // change directory   
	{
	#ifndef ARCHIVER_CAN_RUN_AS_ROOT
		#ifdef UNIX
		// uid = 0 for root 
		if(getuid() > QS_MIN_UID)
		#else
		if(!RunningAsAdministrator())
		#endif
	#endif
		{
			// 
			if(OpenRealTimeConnections() &&
			   OpenDispatcherConnection() )
			{
				//
				RealTimeDbDict realtime_databases = GetRealTimeDbDict();
				RealTimeDbDict spare_realtime_databases;

				Archiver *pArchiver = NULL;
				Archiver *pSpareArchiver = NULL;

				pArchiver = new Archiver;  // create the archiver engine interface

				//pArchiver = new Archiver(NULL, &realtime_databases, GetDispatcher());  // create the monitoring engine interface

				if(OpenSpareDispatcherConnection() && OpenSpareRealTimeConnections())
				{
					//Se esiste un server Spare, allora si crea un nuovo oggetto archiver
					//con lo spare dispatcher e con lo spare database realtime
					
					spare_realtime_databases = GetSpareRealTimeDbDict();
					//pSpareArchiver = new Archiver(NULL, &spare_realtime_databases, GetSpareDispatcher());
				}

				OpenHistoricConnections();

				#ifdef UNIX
				signal(SIGTERM,SigTermHandler);						
				#endif

				stat = a.exec();

				if(pArchiver)
					delete pArchiver;

				if(pSpareArchiver)
					delete pSpareArchiver;
				//
				//
				//
				#ifdef STL_BUG_FIXED
				CloseRealTimeConnections();
				#endif

				#ifdef STL_BUG_FIXED
				CloseDispatcherConnection();
				#endif

				if(GetHistoricResultDb() != NULL)
				{
					#ifdef STL_BUG_FIXED
					CloseHistoricConnections();
					#endif
				}

				if(GetSpareDispatcher() != NULL)
				{
					#ifdef STL_BUG_FIXED
					CloseSpareDispatcherConnection();
					#endif
				}

				if((GetSpareConfigureDb() != NULL) && (GetSpareCurrentDb() != NULL)&&(GetSpareResultDb() != NULL))
				{
					#ifdef STL_BUG_FIXED
					CloseSpareRealTimeConnections();
					#endif
				}

				UnloadAllDlls();
				//
				return stat;
				//
			}
			else
			{
				//cerr << "Failed to connect to database and (or) dispatcher" << endl;
				IT_COMMENT("Failed to connect to database(s) and (or) dispatcher");//error messag
				MessageBox(NULL,"Failed to connect to database(s) and (or) dispatcher","Archiver", MB_OK|MB_ICONSTOP);
			}
		}
	#ifndef ARCHIVER_CAN_RUN_AS_ROOT
		else
		{
			//cerr << "Must Not Run As Root" << endl;
			IT_COMMENT("Must Not Run As Root");//error messag
			MessageBox(NULL,"Must Not Run As Root","Archiver", MB_OK|MB_ICONSTOP);
		}
	#endif
		
	}
	else
	{
		//cerr << "User Directory Not Accessible:" << (const char *) QSFilename("") << endl;
		
		QString err_msg;
		err_msg = "User Directory Not Accessible:" + QSFilename("")+ "\n";
		IT_COMMENT((const char *)err_msg);
	}

	return stat;
}



