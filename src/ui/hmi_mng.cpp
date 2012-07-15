#include <qt.h>
#include "hmi_mng.h"
#include "qwt_thermo.h"
#include "led.h"
#include <qlcdnumber.h>
#include "realtimedb.h"
#include "dispatch.h"

void HMI_manager::setParent( QDialog *parent )
{
    p = parent;

    setInitialValues();

	connect(qApp->mainWidget(),SIGNAL(UpdateTags()),this,SLOT(UpdateTags()));
	connect(qApp->mainWidget(),SIGNAL(UpdateSamplePoint()),this,SLOT(UpdateSamplePoint()));

	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database
}

void HMI_manager::setInitialValues() 
{
	{
		QObjectList *l = p->queryList( "QwtThermo" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			double val = 0.0;

			((QwtThermo*)obj)->setMinValue(0.0);

			((QwtThermo*)obj)->setMaxValue(500.0);

			((QwtThermo*)obj)->setValue(val);
		}

		delete l; // delete the list, not the objects
	}

	{
		QObjectList *l = p->queryList( "QLCDNumber" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			double val = 0.0;

			((QLCDNumber*)obj)->display(val);
		}

		delete l; // delete the list, not the objects
	}

	{
		QObjectList *l = p->queryList( "Led" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			((Led*)obj)->setColor("0,255,0");
			((Led*)obj)->on();
		}

		delete l; // delete the list, not the objects
	}
/*
	{
		QObjectList *l = p->queryList( "QButton" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			//((QButton*)obj)->setEnabled( FALSE );
		}

		delete l; // delete the list, not the objects
	}

	//////////////////////////
	{
		QObjectList *l = p->queryList( "QRadioButton" );

		QObjectListIt it( *l ); // iterate over the radio buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = (obj)->name();
			//((QButton*)obj)->setEnabled( FALSE );
		}

		delete l; // delete the list, not the objects
	}
*/
}

void HMI_manager::UpdateTags()
{
	//IT_IT("HMI_manager::UpdateTags");

	//Here we have set of record from TAGS_DB
	//
	int n = GetCurrentDb()->GetNumberResults();

	GetCurrentDb()->GotoBegin();

	//
	QString lastName = "";

	for(int i = 0; i < n ; i++,GetCurrentDb()->FetchNext())
	{
		QString s = GetCurrentDb()->GetString("NAME");

		if(s != lastName)
		{
			{
				QObjectList *l = p->queryList( "QwtThermo" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					if(name == s + "thermo") // do we want this cell
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));

						((QwtThermo*)obj)->setValue(v);
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "QLCDNumber" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					if(name == s + "lcd") // do we want this cell
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));

						((QLCDNumber*)obj)->display(v);
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			lastName = s;
		}
	}

};

/*
*Function: Query Response - handle transactions with the current values database
*Inputs:object,command, transaction code
*Outputs:none
*Returns:none
*/
void HMI_manager::UpdateSamplePoint() // handle updated sample points
{
//	IT_IT("HMI_manager::UpdateSamplePoint");
	
	int n = GetCurrentDb()->GetNumberResults();

	GetCurrentDb()->GotoBegin();

	for(int i = 0; i < n ; i++,GetCurrentDb()->FetchNext())
	{
		//
		//get the record
		//
		QString ns = GetCurrentDb()->GetString("NAME");
		//
		// Find the entry in the table
		//
		/*
		for(unsigned j = 0; j < pStatus->count(); j++)
		{
			TableItem *st = pStatus->getCell(j);

			if(st->text() == ns)
			{
				int s = GetCurrentDb()->GetInt("STATE");
				st->setBackColour(GetAlarmStateBkColour(s));    
				st->setTextColour(GetAlarmStateFgColour(s));
				st->SetState(s);
				st->SetFlash(GetCurrentDb()->GetInt("ACKFLAG"));
				break;
			}
		}
		*/
	}
};


void HMI_manager::sendCommand() 
{
	QObjectList *l = p->queryList("QwtPushButton");

	QObjectListIt it( *l ); // iterate over the buttons

	QObject *obj;

	while((obj = it.current()) != 0) 
	{
		// for each found object...
		++it;

		if(((QButton*)obj)->isDown())
		{
			Sleep(100);
			QString name = obj->name();
			//((QButton*)obj)->setEnabled( FALSE );

			char str[100];

			//sample_point_name = name.remove(QString("command_button"));
			strcpy(str, (const char*)name.replace("command_button", ""));

			sample_point_name = QString(str);

			QTimer::singleShot(10,this,SLOT(DoCommand()));
	
			break;
		}
	}

	delete l; // delete the list, not the objects
}

void HMI_manager::QueryResponse (QObject *p, const QString &c, int id, QObject*caller) // handles database responses
{
	if(p != this) return;
	switch(id)
	{
		case tUnit:
		{
			QSTransaction &t = GetConfigureDb()->CurrentTransaction();

			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				unsigned char parametri[sizeof(dispatcher_extra_params)];
				dispatcher_extra_params* params = (dispatcher_extra_params *) parametri;

				memset(parametri, 0, sizeof(dispatcher_extra_params));

				QString command_value = t.Data1;
				
				QString samplePointName = t.Data2;
							
				if(command_value.length() > 0)
				{
					(params->res[0]).value = atof((const char*)command_value);
				}
				else
				{
					break;
				}
				
				QString unit_name = GetConfigureDb()->GetString("UNIT");

				strcpy(params->string1, (const char *)unit_name); //driver instance
				strcpy(params->string2, (const char *)samplePointName);
				strcpy(params->string3, (const char *)command_value); //For writing the string
				
				//Generate IEC command
				
				GetDispatcher()->DoExec(NotificationEvent::CMD_SEND_COMMAND_TO_UNIT, (char *)parametri, sizeof(dispatcher_extra_params));  //broadcast to all tcp clients
			}
		} 
		break;
		default:
		break;
	};
};

//void HMI_manager::DoCommand(const QString & sample_point_name)
void HMI_manager::DoCommand()
{
	IT_IT("HMI_manager::DoCommand");

	if(QMessageBox::information(NULL,tr("Command confirmation"),tr("Send the command - Are You Sure?"),QObject::tr("Yes"),QObject::tr("No"),0,1) == 0)
	{
		QString edit_child_name = sample_point_name + QString("command_value");
		QLineEdit* edit = (QLineEdit *) p->child(edit_child_name, "QLineEdit");

		QString value_for_command;

		if(edit)
		{
			QString v = edit->text();

			value_for_command.sprintf("%s", (const char*)v);
		}
		else
		{
			return;
		}

		QString cmd = "select UNIT from SAMPLE where NAME='"+ sample_point_name +"';";

		GetConfigureDb()->DoExec(this, cmd, tUnit, value_for_command, sample_point_name); // kick it off
	}
};
