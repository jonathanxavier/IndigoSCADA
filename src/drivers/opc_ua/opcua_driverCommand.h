/**********************************************************************
--- Qt Architect generated file ---
File: Opcua_driverCommand.h
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#ifndef Opcua_driverCommand_included
#define Opcua_driverCommand_included
#include "opcua_driverCommandData.h"

//NOTA BENE: questa dialog viene caricata
//dalla dll nel client di IndigoSCADA (ui.exe)

class Opcua_driverCommand : public Opcua_driverCommandData
{
	Q_OBJECT
	QString Unit_type; // receipe name
	QString samplePointName;
	enum {tUnit = 1
	//	, tSerial
	}; // transaction codes
	public:
	Opcua_driverCommand(
	QWidget *parent = NULL,
	const char *name = NULL,
	const char *unit_type = NULL );
	virtual ~Opcua_driverCommand();
	protected slots:
	virtual void Help();
	virtual void OkClicked();
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
	void get_utc_host_time(struct cp56time2a* time);
};
#endif // Opcua_driverCommand_included

