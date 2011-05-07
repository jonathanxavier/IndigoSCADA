/**********************************************************************
--- Qt Architect generated file ---
File: Opc_client_daCommand.cpp
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#include "Opc_client_daCommand.h"
#include <qt.h>
#include "opc_client_da.h"
#define Inherited Opc_client_daCommandData
Opc_client_daCommand::Opc_client_daCommand
(
QWidget* parent,
const char* name,
const char *unit_type
)
:
Inherited( parent, name ),Unit_type(unit_type),samplePointName(name)
{
	setCaption(tr("Opc_client_da Command"));

	Name->setText(name);

	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database
}

Opc_client_daCommand::~Opc_client_daCommand()
{
}

void Opc_client_daCommand::Help()
{
	//
	// invoke the help viewer for the Opc_client_da
	// 
	QSHelp("Opc_client_da_command");
}

void Opc_client_daCommand::OkClicked()
{
	if(!YESNO(tr("Command confirmation"),tr("Send the command - Are You Sure?")))
	{
		QString cmd = "select UNIT from SAMPLE where NAME='"+ QString(samplePointName) +"';";
		GetConfigureDb()->DoExec(this,cmd,tUnit); // kick it off
	}
}

void Opc_client_daCommand::QueryResponse (QObject *p, const QString &c, int id, QObject*caller) // handles database responses
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

