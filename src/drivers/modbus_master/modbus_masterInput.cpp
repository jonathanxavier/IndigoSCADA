/**********************************************************************
--- Qt Architect generated file ---
File: modbus_masterInput.cpp
Last generated: Mon May 22 17:08:25 2000
*********************************************************************/
#include <qt.h>
#include "modbus_masterInput.h"
#include "modbus_master.h"
#include "modbus_master_instance.h"

#define Inherited modbus_masterInputData

modbus_masterInput::modbus_masterInput
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

modbus_masterInput::~modbus_masterInput()
{
}

void modbus_masterInput::Load(const QString &s)// load the configuration
{
	IT_IT("modbus_masterInput::Load");

	if(GetConfigureDb()->GetNumberResults())
	{ 
		QString s = UndoEscapeSQLText(GetConfigureDb()->GetString("DVAL"));
		QTextIStream is (&s);
		//
		QString a;
		//is >> a;
		//Time->setText(a.stripWhiteSpace());
		//
		//is >> a;
		//Mean->setText(a.stripWhiteSpace());
		//
		is >> a;
		IOA->setText(a.stripWhiteSpace());
	}
	else
	{
		//Time->setText("00:01:00");
		//Mean->setText("100");
		IOA->setText("10");
	};
};

void modbus_masterInput::Save(const QString &s)// save the configuration
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
