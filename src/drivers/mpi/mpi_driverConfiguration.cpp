/**********************************************************************
--- Qt Architect generated file ---
File: Mpi_driverConfiguration.cpp
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#include "Mpi_driverConfiguration.h"
#include <qt.h>
#include "mpi_driver.h"
#define Inherited Mpi_driverConfigurationData
Mpi_driverConfiguration::Mpi_driverConfiguration
(
QWidget* parent,
const char* name,
const char *receipe
)
:
Inherited( parent, name ),Receipe(receipe)
{
	setCaption(tr("Mpi_driver Configuration"));

	Name->setText(name);

	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database
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
	}

	if(Receipe != "(default)")
	{
		NItems->setEnabled(false);
		MPIServerIPAddressText->setEnabled(false);
		MPIServerIPPortText->setEnabled(false);
	};
}
Mpi_driverConfiguration::~Mpi_driverConfiguration()
{
}
void Mpi_driverConfiguration::Help()
{
	//
	// invoke the help viewer for the Mpi_driver
	// 
	QSHelp("Mpi_driver");
}
void Mpi_driverConfiguration::OkClicked()
{
	//
	QString cmd = QString("delete from PROPS where SKEY='")+QString(Name->text()) + QString("' and IKEY='") + Receipe + "';";
	GetConfigureDb()->DoExec(0,cmd,0); // delete the old value
	//
	cmd = "insert into PROPS values('"+Name->text() +"','" + Receipe + "','" + 
	NItems->text() + " " + MPIServerIPAddressText->text() + " " + MPIServerIPPortText->text() + "');";
	GetConfigureDb()->DoExec(0,cmd,0);
	QSAuditTrail(this,caption(), tr("Edited"));

	Mpi_driver::pDriver->CreateNewUnit(this,Name->text(), (NItems->text()).toInt() );  

	accept();
		
}
void Mpi_driverConfiguration::QueryResponse (QObject *p, const QString &c, int id, QObject*caller) // handles database responses
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
				int n;
				is >> n;
				NItems->setValue(n);
				is >> t;
				MPIServerIPAddressText->setText(t);
				is >> t;
				MPIServerIPPortText->setText(t);
			}
			else
			{
				// just generate the default properties
				QString cmd = "insert into PROPS values('"+Name->text()+"','" + Receipe + "','');";
				GetConfigureDb()->DoExec(0,cmd,0);
				cmd = "insert into PROPS values('"+Name->text()+"','(default)','');"; // create default
				GetConfigureDb()->DoExec(0,cmd,0);
				NItems->setValue(8);
				MPIServerIPAddressText->setText("");
				MPIServerIPPortText->setText("");
			}
		} 
		break;
		default:
		break;
	};
};

