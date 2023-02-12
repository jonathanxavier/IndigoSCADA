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
		OPCUAServerIPPortText->setEnabled(false);
	};

    static const char* items[] = { "N", "E", "O", 0 };
    Parity->insertStrList( items );
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
	if(context == RTU)
	{
		OPCUAServerIPAddressText->setText("xxx.xxx.xxx.xxx");
		OPCUAServerIPPortText->setText("502");

		cmd = "insert into PROPS values('"+Name->text() +"','" + Receipe + "','" + 
		NItems->text() + " " + PollInterval->text() + " " + OPCUAServerIPAddressText->text() + " " + OPCUAServerIPPortText->text() +
		" " + SerialDevice->text() + " " + Baud->text() + " " + DataBits->text() +" "+ StopBit->text() +" "+ Parity->currentText() +" "+ RTSOnTime->text() +" "+ RTSOffTime->text() + "');";
	}
	else if(context == TCP)
	{
		cmd = "insert into PROPS values('"+Name->text() +"','" + Receipe + "','" + 
		NItems->text() + " " + PollInterval->text() + " " + OPCUAServerIPAddressText->text() + " " + OPCUAServerIPPortText->text() +"');";
	}

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
				QString t;
				QString OPCUAServerIPAddress;
				QString OPCUAServerIPPort;
				int n;
				is >> n;
				NItems->setValue(n);
				is >> n;
				PollInterval->setValue(n);
				is >> OPCUAServerIPAddress;
				is >> OPCUAServerIPPort;
				is >> t;
				SerialDevice->setText(t);
				is >> n;
				Baud->setValue(n);
				is >> n;
				DataBits->setValue(n);
				is >> n;
				StopBit->setValue(n);
				is >> t;
				Parity->setCurrentText(t);
				is >> n;
				RTSOnTime->setValue(n);
				is >> n;
				RTSOffTime->setValue(n);

				if(strlen((const char*)(SerialDevice->text())) == 0)
				{
					TCPButton->toggle();
	                context = TCP;

					OPCUAServerIPAddressText->setText(OPCUAServerIPAddress);
					OPCUAServerIPPortText->setText(OPCUAServerIPPort);

				}
				else
				{
					RTUButton->toggle();
	                context = RTU;

					OPCUAServerIPAddressText->setText("");
					OPCUAServerIPPortText->setText("");
				}
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
				OPCUAServerIPPortText->setText("");
				SerialDevice->setText("COM1");
				Baud->setValue(9600);
				DataBits->setValue(8);
				StopBit->setValue(1);
				Parity->setCurrentText("N");
				RTSOnTime->setValue(0);
				RTSOffTime->setValue(0);
				RTUButton->toggle();
			}
		} 
		break;
		default:
		break;
	};
};

void Opcua_driverConfiguration::RTUContextActive(bool)
{
	context = RTU;
}

void Opcua_driverConfiguration::TCPContextActive(bool)
{
    context = TCP;
}

