/**********************************************************************
--- Qt Architect generated file ---
File: Opcua_driverInput.cpp
Last generated: Mon May 22 17:08:25 2000
*********************************************************************/
#include <qt.h>
#include "opcua_driverInput.h"
#include "opcua_driver.h"
#include "opcua_driver_instance.h"

#define Inherited Opcua_driverInputData

Opcua_driverInput::Opcua_driverInput
(
QWidget* parent,
const char* name
)
:
Inherited( parent, name ),Name(name)
{
	// add FP validators to Low and High
	//Low->setValidator(new QDoubleValidator(-100000.0,100000.0,3,Low));
	//High->setValidator(new QDoubleValidator(-100000.0,100000.0,3,High));
	IOA->setValidator(new QIntValidator(0,16777215,NULL,NULL));
}

Opcua_driverInput::~Opcua_driverInput()
{
}

void Opcua_driverInput::Load(const QString &s)// load the configuration
{
	IT_IT("Opcua_driverInput::Load");

	if(GetConfigureDb()->GetNumberResults())
	{ 
		QString s = UndoEscapeSQLText(GetConfigureDb()->GetString("IOA"));
		QTextIStream is (&s);
		//
		QString a;
		is >> a;
		IOA->setText(a.stripWhiteSpace());
	}
	else
	{
		IOA->setText("0");
	}
};

void Opcua_driverInput::Save(const QString &s)// save the configuration
{
	//
	// save from fields to properties
	// 
	QString cmd = "delete from PROPS where SKEY ='SAMPLEPROPS' and IKEY='"+s+"';";
	GetConfigureDb()->DoExec(0,cmd,0);
	//
	QString t;
	t =  IOA->text();
	//
	cmd = "insert into PROPS values('SAMPLEPROPS','"+s+"','"+t.stripWhiteSpace()+"');";
	GetConfigureDb()->DoExec(0,cmd,0);
};
