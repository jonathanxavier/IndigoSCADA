/**********************************************************************
--- Qt Architect generated file ---
File: Rfc1006driverCommand.cpp
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#include "Rfc1006driverCommand.h"
#include <qt.h>
#include "Rfc1006driver.h"
#define Inherited Rfc1006driverCommandData
Rfc1006driverCommand::Rfc1006driverCommand
(
QWidget* parent,
const char* name,
const char *unit_type
)
:
Inherited( parent, name ),Unit_type(unit_type),samplePointName(name)
{
	setCaption(tr("Rfc1006driver Command"));

	Name->setText(name);

	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database
}

Rfc1006driverCommand::~Rfc1006driverCommand()
{
}

void Rfc1006driverCommand::Help()
{
	//
	// invoke the help viewer for the Rfc1006driver
	// 
	QSHelp("Rfc1006driver_command");
}

void Rfc1006driverCommand::OkClicked()
{
	if(!YESNO(tr("Command confirmation"),tr("Send the command - Are You Sure?")))
	{
		QString cmd = "select UNIT from SAMPLE where NAME='"+ QString(samplePointName) +"';";
		GetConfigureDb()->DoExec(this,cmd,tUnit); // kick it off
	}
}

void Rfc1006driverCommand::QueryResponse (QObject *p, const QString &c, int id, QObject*caller) // handles database responses
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
				
				//Generate IEC command
				
				GetDispatcher()->DoExec(NotificationEvent::CMD_SEND_COMMAND_TO_UNIT, (char *)parametri, sizeof(dispatcher_extra_params));  //broadcast to all tcp clients

				accept();
			}
		} 
		break;
		default:
		break;
	};
};

