/**********************************************************************
--- Qt Architect generated file ---
File: Iec104driverCommand.cpp
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#include "Iec104driverCommand.h"
#include <qt.h>
#include "Iec104driver.h"
#define Inherited Iec104driverCommandData
Iec104driverCommand::Iec104driverCommand
(
QWidget* parent,
const char* name,
const char *unit_type
)
:
Inherited( parent, name ),Unit_type(unit_type),samplePointName(name)
{
	setCaption(tr("Iec104driver Command"));

	Name->setText(name);

	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database
}

Iec104driverCommand::~Iec104driverCommand()
{
}

void Iec104driverCommand::Help()
{
	//
	// invoke the help viewer for the Iec104driver
	// 
	QSHelp("Iec104driver_command");
}

void Iec104driverCommand::OkClicked()
{
	if(!YESNO(tr("Command confirmation"),tr("Send the command - Are You Sure?")))
	{
		QString cmd = "select UNIT from SAMPLE where NAME='"+ QString(samplePointName) +"';";
		GetConfigureDb()->DoExec(this,cmd,tUnit); // kick it off
	}
}

void Iec104driverCommand::QueryResponse (QObject *p, const QString &c, int id, QObject*caller) // handles database responses
{
	if(p != this) return;
	switch(id)
	{
		case tUnit:
		{
			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				unsigned char parametri[sizeof(dispatcher_extra_params)];
				dispatcher_extra_params* params = (dispatcher_extra_params *) parametri;

				memset(parametri, 0, sizeof(dispatcher_extra_params));
							
				(params->res[0]).value = atof((const char*)Value->text());
				
				QString unit_name = GetConfigureDb()->GetString("UNIT");
				strcpy(params->string1, (const char *)unit_name); //driver instance
				strcpy(params->string2, (const char *)samplePointName);
				strcpy(params->string3, (const char *)Value->text()); //For writing the string
				
				//Generazione del COMANDO OPC
				
				GetDispatcher()->DoExec(NotificationEvent::CMD_SEND_COMMAND_TO_UNIT, (char *)parametri, sizeof(dispatcher_extra_params));  //broadcast to all tcp clients

				accept();
			}
		} 
		break;
		default:
		break;
	};
};

