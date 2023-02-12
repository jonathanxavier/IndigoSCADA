/**********************************************************************
--- Qt Architect generated file ---
File: Opcua_driverConfiguration.cpp
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#include "Opcua_driverConfiguration.h"
#include <qt.h>
#include "opcua_driver.h"
#define Inherited Opcua_driverConfigurationData
Opcua_driverConfiguration::Opcua_driverConfiguration
(
QWidget* parent,
const char* name,
const char *receipe
)
:
Inherited( parent, name ),Receipe(receipe)
{
	setCaption(tr("Opcua_driver Configuration"));

	Name->setText(name);

	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database

	//GetConfigureDb()->DoExec(this,"select * from SERIAL where ENABLED=1 order by NAME;",tSerial); // get the list of enabled serial ports 
	// 
	// get the properties for this unit and receipe
	if(Receipe == "(default)")
	{ 
		QString pc = 
		"select * from PROPS where SKEY='" + QString(name) + 
		"' and IKEY='(default)';"; 
		//
		// get the properties SKEY = unit name IKEY = receipe name
		GetConfigureDb()->DoExec(this,pc,tItem);
	}
	else
	{
		QString pc = 
		"select * from PROPS where SKEY='" + QString(name) + 
		"' and (IKEY='(default)' or IKEY='"+ Receipe + "') order by IKEY desc;"; 
		//
		// get the properties SKEY = unit name IKEY = receipe name
		GetConfigureDb()->DoExec(this,pc,tItem);
	};
	if(Receipe != "(default)")
	{
		NItems->setEnabled(false);
		OPCUAServerIPAddressText->setEnabled(false);
	};
}
Opcua_driverConfiguration::~Opcua_driverConfiguration()
{
}
void Opcua_driverConfiguration::Help()
{
	//
	// invoke the help viewer for the Opcua_driver
	// 
	QSHelp("Opcua_driver");
}
void Opcua_driverConfiguration::OkClicked()
{
	//
	QString cmd = QString("delete from PROPS where SKEY='")+QString(Name->text()) + QString("' and IKEY='") + Receipe + "';";
	GetConfigureDb()->DoExec(0,cmd,0); // delete the old value
	//
	cmd = "insert into PROPS values('"+Name->text() +"','" + Receipe + "','" + 
	NItems->text() + " " + PollInterval->text() + " " + OPCUAServerIPAddressText->text() +"');";

	GetConfigureDb()->DoExec(0,cmd,0);
	QSAuditTrail(this,caption(), tr("Edited"));

	Opcua_driver::pDriver->CreateNewUnit(this,Name->text(), (NItems->text()).toInt() );  

	accept();
		
}
void Opcua_driverConfiguration::QueryResponse (QObject *p, const QString &c, int id, QObject*caller) // handles database responses
{
	if(p != this) return;
	switch(id)
	{
		case tItem:
		{
			if(GetConfigureDb()->GetNumberResults() > 0)
			{
				//
				// we have the properties response
				// fill out the fields
				// 
				QString s = UndoEscapeSQLText(GetConfigureDb()->GetString("DVAL")); // the top one is either the receipe or (default)
				QTextIStream is(&s); // extract the values
				//
				QString OPCUAServerIPAddress;
				int n;
				is >> n;
				NItems->setValue(n);
				is >> n;
				PollInterval->setValue(n);
				is >> OPCUAServerIPAddress;

				OPCUAServerIPAddressText->setText(OPCUAServerIPAddress);
			}
			else
			{
				// just generate the default properties
				QString cmd = "insert into PROPS values('"+Name->text()+"','" + Receipe + "','');";
				GetConfigureDb()->DoExec(0,cmd,0);
				cmd = "insert into PROPS values('"+Name->text()+"','(default)','');"; // create default
				GetConfigureDb()->DoExec(0,cmd,0);
				NItems->setValue(8);
				PollInterval->setValue(1000);
				OPCUAServerIPAddressText->setText("");
			}
		} 
		break;
		default:
		break;
	};
};

