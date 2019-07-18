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

	Type->insertItem ("C_IC_NA_1");	// add to the list box

	Type->insertItem ("C_SC_NA_1");	// add to the list box
	Type->insertItem ("C_DC_NA_1");	// add to the list box

	Type->insertItem ("C_SC_TA_1");	// add to the list box
	Type->insertItem ("C_DC_TA_1");	// add to the list box

	Type->insertItem ("C_SE_NA_1");	// add to the list box
	Type->insertItem ("C_SE_NB_1");	// add to the list box
	Type->insertItem ("C_SE_NC_1");	// add to the list box

	Type->insertItem ("C_SE_TA_1");	// add to the list box
	Type->insertItem ("C_SE_TB_1");	// add to the list box
	Type->insertItem ("C_SE_TC_1");	// add to the list box

	Type->insertItem ("C_CS_NA_1");	// add to the list box
	
	Type->setCurrentItem (0);

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
							
				params->value = atof((const char*)Value->text());
				
				QString unit_name = GetConfigureDb()->GetString("UNIT");
				strcpy(params->string1, (const char *)unit_name); //driver instance
				strcpy(params->string2, (const char *)samplePointName);
				strcpy(params->string3, (const char *)Value->text()); //For writing the string

				struct cp56time2a actual_time;
				get_utc_host_time(&actual_time);
				params->time_stamp = actual_time;

				char IECcommandtype[20];
				strcpy(IECcommandtype, (const char *)Type->currentText());
								
				if(strcmp(IECcommandtype, "C_IC_NA_1") == 0)
					params->iec_command_type = 100;
				else if(strcmp(IECcommandtype, "C_SC_NA_1") == 0)
					params->iec_command_type = 45;
				else if(strcmp(IECcommandtype, "C_DC_NA_1") == 0)
					params->iec_command_type = 46;
				else if(strcmp(IECcommandtype, "C_SC_TA_1") == 0)
					params->iec_command_type = 58;
				else if(strcmp(IECcommandtype, "C_DC_TA_1") == 0)
					params->iec_command_type = 59;
				else if(strcmp(IECcommandtype, "C_SE_NA_1") == 0)
					params->iec_command_type = 48;
				else if(strcmp(IECcommandtype, "C_SE_NB_1") == 0)
					params->iec_command_type = 49;
				else if(strcmp(IECcommandtype, "C_SE_NC_1") == 0)
					params->iec_command_type = 50;
				else if(strcmp(IECcommandtype, "C_SE_TA_1") == 0)
					params->iec_command_type = 61;
				else if(strcmp(IECcommandtype, "C_SE_TB_1") == 0)
					params->iec_command_type = 62;
				else if(strcmp(IECcommandtype, "C_SE_TC_1") == 0)
					params->iec_command_type = 63;
				else if(strcmp(IECcommandtype, "C_CS_NA_1") == 0)
					params->iec_command_type = 103;
				
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

#include <time.h>
#include <sys/timeb.h>

void Iec104driverCommand::get_utc_host_time(struct cp56time2a* time)
{
	struct timeb tb;
	struct tm	*ptm;

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

    return;
}
