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

#include <qt.h>
#include "hmi_mng.h"
#include "qwt_thermo.h"
#include "single_point_led.h"
#include "double_point_led.h"
#include "realtimedb.h"
#include "dispatch.h"
#include "helper_functions.h"
#include "pswitch.h"
#include "ptank.h"
#include "pthermometer.h"
#include "inspect.h"
#include "plcdnumber.h"
#include "psinglepointled.h"
#include "pdoublepointled.h"

void HMI_manager::setParent( QDialog *parent )
{
    p = parent;

    setInitialValues();

	connect(qApp->mainWidget(),SIGNAL(UpdateTags()),this,SLOT(UpdateTags()));
	connect(qApp->mainWidget(),SIGNAL(UpdateSamplePoint()),this,SLOT(UpdateSamplePoint()));

	// connect to the database

	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));

	connect (GetDispatcher (),
	SIGNAL (ReceivedNotify(int, const char *)), this,
	SLOT (ReceivedNotify(int, const char *)));	

}

//	QColor white(0xff, 0xff, 0xff);
//	QColor red(0xff, 0x00, 0x00);
//	QColor yellow(0xff, 0xff, 0x00);
//	QColor green(0x00, 0xff, 0x00);
//	QColor blue(0x00, 0x00, 0xff);

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

			((QwtThermo*)obj)->setMinValue(-500.0);

			((QwtThermo*)obj)->setMaxValue(1000.0);

			((QwtThermo*)obj)->setValue(val);
		}

		delete l; // delete the list, not the objects
	}
	
	{
		QObjectList *l = p->queryList( "PLCDNumber" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			double val = 0.0;

			((PLCDNumber*)obj)->display(val);
		}

		delete l; // delete the list, not the objects
	}

	{
		QObjectList *l = p->queryList( "SinglePointLed" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			((SinglePointLed*)obj)->setColor(Qt::white);
			((SinglePointLed*)obj)->on();
		}

		delete l; // delete the list, not the objects
	}

    	{
		QObjectList *l = p->queryList( "DoublePointLed" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			((DoublePointLed*)obj)->setColor(Qt::white);
			((DoublePointLed*)obj)->on();
		}

		delete l; // delete the list, not the objects
	}

	{
		QObjectList *l = p->queryList( "QButton" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			//By default commands are not enabled
			((QButton*)obj)->setEnabled( FALSE );
			
		}

		delete l; // delete the list, not the objects
	}

	{
		QObjectList *l = p->queryList( "PTank" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			((PTank*)obj)->setMinValue(-500.0);
			((PTank*)obj)->setMaxValue(1000.0);
			((PTank*)obj)->setValue(0);
		}

		delete l; // delete the list, not the objects
	}

	{
		QObjectList *l = p->queryList( "PThermometer" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			((PThermometer*)obj)->setMinValue(-500.0);
			((PThermometer*)obj)->setMaxValue(1000.0);
			((PThermometer*)obj)->setValue(0);
		}

		delete l; // delete the list, not the objects
	}

	{
		QObjectList *l = p->queryList( "PSinglePointLed" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			((PSinglePointLed*)obj)->on();
			((PSinglePointLed*)obj)->setOnColor(white);
		}

		delete l; // delete the list, not the objects
	}

	{
		QObjectList *l = p->queryList( "PDoublePointLed" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			((PDoublePointLed*)obj)->on();
			((PDoublePointLed*)obj)->setOnColor(white);
		}

		delete l; // delete the list, not the objects
	}

	{
		QObjectList *l = p->queryList( "PSwitch" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			QString name = obj->name();

			((PSwitch*)obj)->setPSwitchValue(false);
		}

		delete l; // delete the list, not the objects
	}
}

//TODO: put at startup the content of QObjectList in a Map and make tree search

void HMI_manager::UpdateTags()
{
	//IT_IT("HMI_manager::UpdateTags");

	//Here we have set of records from TAGS_DB
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

					// e.g. OPCPoint09QwtThermo

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));

						((QwtThermo*)obj)->setValue(v);
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "PLCDNumber" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. OPCPoint09

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));

						((PLCDNumber*)obj)->display(v);
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "SinglePointLed" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. OPCPoint09

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));
						
						int i = (int)v;

						switch(i)
						{
							case 0:
							{
								//Green means state off
								((SinglePointLed*)obj)->setColor(Qt::green);
								((SinglePointLed*)obj)->on();
							}
							break;
							case 1:
							{
								//	Red means state on
								((SinglePointLed*)obj)->setColor(Qt::red);
								((SinglePointLed*)obj)->on();
							}
							break;
							case 2:
							{
								//Yellow is not used for Single point
								((SinglePointLed*)obj)->setColor(Qt::yellow);
								((SinglePointLed*)obj)->on();
							}
							break;
							case 3:
							{
								//Yellow is not used for Single point
								((SinglePointLed*)obj)->setColor(Qt::yellow);
								((SinglePointLed*)obj)->on();
							}
							break;
							default:
								//White means HMI state none or Invalid
								((SinglePointLed*)obj)->setColor(Qt::white);
								((SinglePointLed*)obj)->on();
							break;
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

            		{
				QObjectList *l = p->queryList( "DoublePointLed" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. IEC104Point

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));
						
						int i = (int)v;

						switch(i)
						{
							case 0:
							{
								//Yellow means Indeterminate or Intermediate state
								((DoublePointLed*)obj)->setColor(Qt::yellow);
								((DoublePointLed*)obj)->on();
							}
							break;
							case 1:
							{
								//Green means Determinate state off
								((DoublePointLed*)obj)->setColor(Qt::green);
								((DoublePointLed*)obj)->on();
							}
							break;
							case 2:
							{
								//Red means Determinate state on
								((DoublePointLed*)obj)->setColor(Qt::red);
								((DoublePointLed*)obj)->on();
							}
							break;
							case 3:
							{
								//Yellow means Indeterminate state
								((DoublePointLed*)obj)->setColor(Qt::yellow);
								((DoublePointLed*)obj)->on();
							}
							break;
							default:
								//White means HMI state none or Invalid
								((DoublePointLed*)obj)->setColor(Qt::white);
								((DoublePointLed*)obj)->on();
							break;
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "PSinglePointLed" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. OPCPoint09

					int idx = name.find('_');

					name.truncate(idx);

					///////////////////////////////////////////////////////

					//if(name == s + "PSinglePointLed")
					if(name == s)
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));
						
						int i = (int)v;

						switch(i)
						{
							case 0:
							{
								//Green means state off
								((PSinglePointLed*)obj)->on();
								((PSinglePointLed*)obj)->setOnColor(green);
							}
							break;
							case 1:
							{
								//	Red means state on
								((PSinglePointLed*)obj)->on();
								((PSinglePointLed*)obj)->setOnColor(red);
							}
							break;
							case 2:
							{
								//Yellow is not used for Single point
								((PSinglePointLed*)obj)->on();
								((PSinglePointLed*)obj)->setOnColor(yellow);
							}
							break;
							case 3:
							{
								//Yellow is not used for Single point
								((PSinglePointLed*)obj)->on();
								((PSinglePointLed*)obj)->setOnColor(yellow);
							}
							break;
							default:
								//White means HMI state none or Invalid
								((PSinglePointLed*)obj)->on();
								((PSinglePointLed*)obj)->setOnColor(white);
							break;
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "PDoublePointLed" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. OPCPoint09

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));
						
						int i = (int)v;

						switch(i)
						{
							case 0:
							{
								//Yellow means Indeterminate or Intermediate state
								((PDoublePointLed*)obj)->on();
								((PDoublePointLed*)obj)->setOnColor(yellow);
							}
							break;
							case 1:
							{
								//Green means Determinate state off

								((PDoublePointLed*)obj)->on();
								((PDoublePointLed*)obj)->setOnColor(green);
							}
							break;
							case 2:
							{
								//Red means Determinate state on
								((PDoublePointLed*)obj)->on();
								((PDoublePointLed*)obj)->setOnColor(red);
							}
							break;
							case 3:
							{
								//Yellow means Indeterminate state								
								((PDoublePointLed*)obj)->on();
								((PDoublePointLed*)obj)->setOnColor(yellow);
							}
							break;
							default:
								//White means HMI state none or Invalid
								((PDoublePointLed*)obj)->on();
								((PDoublePointLed*)obj)->setOnColor(white);
							break;
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}
			
			{
				QObjectList *l = p->queryList( "PSwitch" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. OPCPoint09

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));
						
						int i = (int)v;

						switch(i)
						{
							case 0:
							{
								//Green means state off
								((PSwitch*)obj)->setPSwitchValue(false);
							}
							break;
							case 1:
							{
								//	Red means state on
								((PSwitch*)obj)->setPSwitchValue(true);
							}
							break;
							default:
								//White means HMI state none or Invalid
								((PSwitch*)obj)->setPSwitchValueInvalid(true);
							break;
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "PTank" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. OPCPoint09

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));

						((PTank*)obj)->setValue(v);
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "PThermometer" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. OPCPoint09

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));

						((PThermometer*)obj)->setValue(v);
						
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
*Inputs:
*Outputs:none
*Returns:none
*/
void HMI_manager::UpdateSamplePoint() // handle updated sample points
{
//	IT_IT("HMI_manager::UpdateSamplePoint");

	//Here we have a set of records from CVAL_DB
	
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
				QObjectList *l = p->queryList( "SinglePointLed" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. OPCPoint09

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						int state = GetCurrentDb()->GetInt("STATE");

						int ack_flag = GetCurrentDb()->GetInt("ACKFLAG");

						if(ack_flag)
						{
							((SinglePointLed*)obj)->startFlash();
						}
						else
						{
							((SinglePointLed*)obj)->stopFlash();
						}

						if(state == NoLevel)
						{
							//White means HMI state none or NO or Invalid
							((SinglePointLed*)obj)->setColor(Qt::white);
							((SinglePointLed*)obj)->on(); 
						}

						if(state == FailureLevel)
						{ //Blue means Communication driver error state or Invalid
								((SinglePointLed*)obj)->setColor(Qt::blue);
								((SinglePointLed*)obj)->on(); 
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

            		{
				QObjectList *l = p->queryList( "DoublePointLed" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. IEC104Point

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						int state = GetCurrentDb()->GetInt("STATE");

						int ack_flag = GetCurrentDb()->GetInt("ACKFLAG");

						if(ack_flag)
						{
							((DoublePointLed*)obj)->startFlash();
						}
						else
						{
							((DoublePointLed*)obj)->stopFlash();
						}

						if(state == NoLevel)
						{
							//White means HMI state none or NO or Invalid
							((DoublePointLed*)obj)->setColor(Qt::white);
							((DoublePointLed*)obj)->on(); 
						}

						if(state == FailureLevel)
						{	    //Blue means Communication driver error state or Invalid
								((DoublePointLed*)obj)->setColor(Qt::blue);
								((DoublePointLed*)obj)->on(); 
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "PSinglePointLed" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. OPCPoint09

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						int state = GetCurrentDb()->GetInt("STATE");

						int ack_flag = GetCurrentDb()->GetInt("ACKFLAG");
						
						if(ack_flag)
						{
							((PSinglePointLed*)obj)->startFlash();
						}
						else
						{
							((PSinglePointLed*)obj)->stopFlash();
						}

						if(state == NoLevel)
						{
							//White means HMI state none or NO or Invalid
							((PSinglePointLed*)obj)->on();
							((PSinglePointLed*)obj)->setOnColor(white);
						}

						if(state == FailureLevel)
						{   //Blue means Communication driver error state or Invalid
							((PSinglePointLed*)obj)->on();
							((PSinglePointLed*)obj)->setOnColor(blue);
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "PDoublePointLed" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					// e.g. IEC104Point

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						int state = GetCurrentDb()->GetInt("STATE");

						int ack_flag = GetCurrentDb()->GetInt("ACKFLAG");
						
						if(ack_flag)
						{
							((PDoublePointLed*)obj)->startFlash();
						}
						else
						{
							((PDoublePointLed*)obj)->stopFlash();
						}

						if(state == NoLevel)
						{
							//White means HMI state none or NO or Invalid
							((PDoublePointLed*)obj)->on();
							((PDoublePointLed*)obj)->setOnColor(white);
						}

						if(state == FailureLevel)
						{   //Blue means Communication driver error state or Invalid
							((PDoublePointLed*)obj)->on();
							((PDoublePointLed*)obj)->setOnColor(blue);
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			{
				QObjectList *l = p->queryList( "PSwitch" );

				QObjectListIt it( *l ); // iterate over the buttons

				QObject *obj;

				while((obj = it.current()) != 0) 
				{
					// for each found object...
					++it;

					QString name = obj->name();

					int idx = name.find('_');
					name.truncate(idx);

					if(name == s)
					{
						int state = GetCurrentDb()->GetInt("STATE");

						int ack_flag = GetCurrentDb()->GetInt("ACKFLAG");

						/*
						if(ack_flag)
						{
							((PSwitch*)obj)->startFlash();
						}
						else
						{
							((PSwitch*)obj)->stopFlash();
						}
						*/

						if(state == NoLevel)
						{
							//White means HMI state none or NO or Invalid
							((PSwitch*)obj)->setPSwitchValueInvalid(false);
						}

						if(state == FailureLevel)
						{   //Blue means Communication driver error state or Invalid
							((PSwitch*)obj)->setPSwitchValueInvalid(false);
						}
						
						break; // handle the next record
					}
				}

				delete l; // delete the list, not the objects
			}

			lastName = s;
		}
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

			int idx = name.find('_');
			name.truncate(idx);
			sample_point_name = name;
			
			QTimer::singleShot(10,this,SLOT(DoButtonCommand()));
	
			break;
		}
	}

	delete l; // delete the list, not the objects
}


void HMI_manager::pSwitchToggledSendCommand()
{
	QObjectList *l = p->queryList("PSwitch");

	QObjectListIt it( *l ); // iterate over the switches

	QObject *obj;

	while((obj = it.current()) != 0) 
	{
		// for each found object...
		++it;

		if(((PSwitch*)obj)->hasFocus())
		{
			if(((PSwitch*)obj)->getPSwitchValue())
			{
				value_for_command.sprintf("1");
			}
			else
			{
				value_for_command.sprintf("0");
			}
		
			Sleep(100);

			QString name = obj->name();

			int idx = name.find('_');
			name.truncate(idx);
			sample_point_name = name;

			QTimer::singleShot(10,this,SLOT(Do_pSwitchCommand()));

			break;
		}
	}

	delete l; // delete the list, not the objects
}


/*
*Function:RightClicked(const QString &name) 
show the active object menu
*Inputs:select object
*Outputs:none
*Returns:none
*/
void HMI_manager::RightClicked(QString &class_name, QString &widget_name) // show the inspection window
{
	IT_IT("HMI_manager::RightClicked");

	//  
	// create the inspection window
	//

	bool ack = false;
	bool found = false;

	if(class_name == QString("SinglePointLed"))
	{
		QObjectList *l = p->queryList("SinglePointLed");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(widget_name == obj->name())
			{
				int idx = widget_name.find('_');
				widget_name.truncate(idx);
				sample_point_name = widget_name;
				
				found = true;

				break;
			}
		}

		delete l; // delete the list, not the objects

		if(found)
		{
			ack = ((SinglePointLed*)obj)->GetFlash(); // need an ack or wot ?

			InspectMenu((SinglePointLed*)obj, sample_point_name, ack);

			return;
		}
	}
	else if(class_name == QString("DoublePointLed"))
	{
		QObjectList *l = p->queryList("DoublePointLed");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(widget_name == obj->name())
			{
				int idx = widget_name.find('_');
				widget_name.truncate(idx);
				sample_point_name = widget_name;
				
				found = true;

				break;
			}
		}

		delete l; // delete the list, not the objects

		if(found)
		{
			ack = ((DoublePointLed*)obj)->GetFlash(); // need an ack or wot ?

			InspectMenu((DoublePointLed*)obj, sample_point_name, ack);

			return;
		}
	}
	if(class_name == QString("PSwitch"))
	{
		QObjectList *l = p->queryList("PSwitch");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(widget_name == obj->name())
			{
				int idx = widget_name.find('_');
				widget_name.truncate(idx);
				sample_point_name = widget_name;
				
				found = true;

				break;
			}
		}

		delete l; // delete the list, not the objects

		if(found)
		{
			InspectMenu((PSwitch*)obj, sample_point_name, ack);

			return;
		}
	}
	else if(class_name == QString("PSinglePointLed"))
	{
		QObjectList *l = p->queryList("PSinglePointLed");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(widget_name == obj->name())
			{
				int idx = widget_name.find('_');
				widget_name.truncate(idx);
				sample_point_name = widget_name;
				
				found = true;

				break;
			}
		}

		delete l; // delete the list, not the objects

		if(found)
		{
			ack = ((PSinglePointLed*)obj)->GetFlash(); // need an ack or wot ?

			InspectMenu((PSinglePointLed*)obj, sample_point_name, ack);

			return;
		}
	}
	else if(class_name == QString("PDoublePointLed"))
	{
		QObjectList *l = p->queryList("PDoublePointLed");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(widget_name == obj->name())
			{
				int idx = widget_name.find('_');
				widget_name.truncate(idx);
				sample_point_name = widget_name;
				
				found = true;

				break;
			}
		}

		delete l; // delete the list, not the objects

		if(found)
		{
			ack = ((PDoublePointLed*)obj)->GetFlash(); // need an ack or wot ?

			InspectMenu((PDoublePointLed*)obj, sample_point_name, ack);

			return;
		}
	}
	else if(class_name == QString("QwtThermo"))
	{
		QObjectList *l = p->queryList("QwtThermo");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(widget_name == obj->name())
			{
				int idx = widget_name.find('_');
				widget_name.truncate(idx);
				sample_point_name = widget_name;
				
				found = true;

				break;
			}
		}

		delete l; // delete the list, not the objects

		if(found)
		{
			InspectMenu((QwtThermo*)obj, sample_point_name, ack);

			return;
		}
	}
	else if(class_name == QString("PTank"))
	{
		QObjectList *l = p->queryList("PTank");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(widget_name == obj->name())
			{
				int idx = widget_name.find('_');
				widget_name.truncate(idx);
				sample_point_name = widget_name;
				
				found = true;

				break;
			}
		}

		delete l; // delete the list, not the objects

		if(found)
		{
			InspectMenu((PTank*)obj, sample_point_name, ack);

			return;
		}
	}
	else if(class_name == QString("PThermometer"))
	{
		QObjectList *l = p->queryList("PThermometer");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(widget_name == obj->name())
			{
				int idx = widget_name.find('_');
				widget_name.truncate(idx);
				sample_point_name = widget_name;
				
				found = true;

				break;
			}
		}

		delete l; // delete the list, not the objects

		if(found)
		{
			InspectMenu((PThermometer*)obj, sample_point_name, ack);

			return;
		}
	}
	else if(class_name == QString("PLCDNumber"))
	{
		QObjectList *l = p->queryList("PLCDNumber");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(widget_name == obj->name())
			{
				int idx = widget_name.find('_');
				widget_name.truncate(idx);
				sample_point_name = widget_name;
				
				found = true;

				break;
			}
		}

		delete l; // delete the list, not the objects

		if(found)
		{
			InspectMenu((PLCDNumber*)obj, sample_point_name, ack);

			return;
		}
	}
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

void HMI_manager::DoButtonCommand()
{
	IT_IT("HMI_manager::DoButtonCommand");

	if(QMessageBox::information(NULL,tr("Command confirmation"),tr("Send the command - Are You Sure?"),QObject::tr("Yes"),QObject::tr("No"),0,1) == 0)
	{
		bool found = false;
		QString edit_child_name;
		
		QObjectList *l = p->queryList( "QLineEdit" );

		QObjectListIt it( *l ); // iterate over the buttons

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			edit_child_name = obj->name();

			QString truncated_edit_child_name = edit_child_name;

			int idx = truncated_edit_child_name.find('_');
			truncated_edit_child_name.truncate(idx);

			if(sample_point_name == truncated_edit_child_name)
			{
				found = true;
				break;
			}
		}

		delete l; // delete the list, not the objects
		
		if(found)
		{
			QLineEdit* edit = (QLineEdit *) p->child(edit_child_name, "QLineEdit");

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
	}
};

void HMI_manager::Do_pSwitchCommand()
{
	IT_IT("HMI_manager::Do_pSwitchCommand");

	if(QMessageBox::information(NULL,tr("Command confirmation"),tr("Send the command - Are You Sure?"),QObject::tr("Yes"),QObject::tr("No"),0,1) == 0)
	{
		QString cmd = "select UNIT from SAMPLE where NAME='"+ sample_point_name +"';";

		GetConfigureDb()->DoExec(this, cmd, tUnit, value_for_command, sample_point_name); // kick it off
	}
	else
	{
		QObjectList *l = p->queryList("PSwitch");

		QObjectListIt it( *l ); // iterate over the switches

		QObject *obj;

		while((obj = it.current()) != 0) 
		{
			// for each found object...
			++it;

			if(((PSwitch*)obj)->hasFocus())
			{
				((PSwitch*)obj)->undoToggle();

				break;
			}
		}

		delete l; // delete the list, not the objects
	}
};


/*
*Function:CurrentNotify
*Inputs:notification code
*Outputs:none
*Returns:none
*/
void HMI_manager::ReceivedNotify(int ntf, const char * data)
{
	IT_IT("HMI_manager::ReceivedNotify");
	
	switch(ntf)
	{
		case NotificationEvent::CMD_LOGOUT:
		{
			QObjectList *l = p->queryList( "QButton" );

			QObjectListIt it( *l ); // iterate over the buttons

			QObject *obj;

			while((obj = it.current()) != 0) 
			{
				// for each found object...
				++it;

				QString name = obj->name();

				((QButton*)obj)->setEnabled( FALSE );
			}

			delete l; // delete the list, not the objects
		}
		break;
		case NotificationEvent::CMD_LOGON:
		{
			QObjectList *l = p->queryList( "QButton" );

			QObjectListIt it( *l ); // iterate over the buttons

			QObject *obj;

			while((obj = it.current()) != 0) 
			{
				// for each found object...
				++it;

				QString name = obj->name();

				if(GetUserDetails().privs & PRIVS_ACK_ALARMS)
				{
					((QButton*)obj)->setEnabled(TRUE);
				}
				else
				{
					((QButton*)obj)->setEnabled( FALSE );
				}
			}

			delete l; // delete the list, not the objects
		}
		break;
		default:
		break;
	}
};
