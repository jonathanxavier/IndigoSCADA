/**********************************************************************
--- Qt Architect generated file ---
File: modbus_masterCommand.h
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#ifndef modbus_masterCommand_included
#define modbus_masterCommand_included
#include "modbus_masterCommandData.h"

//NOTA BENE: questa dialog viene caricata
//dalla dll nel client di IndigoSCADA (ist.exe)

class modbus_masterCommand : public modbus_masterCommandData
{
	Q_OBJECT
	QString Unit_type; // receipe name
	QString samplePointName;
	enum {tUnit = 1
	//	, tSerial
	}; // transaction codes
	public:
	modbus_masterCommand(
	QWidget *parent = NULL,
	const char *name = NULL,
	const char *unit_type = NULL );
	virtual ~modbus_masterCommand();
	protected slots:
	virtual void Help();
	virtual void OkClicked();
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
};
#endif // modbus_masterCommand_included

