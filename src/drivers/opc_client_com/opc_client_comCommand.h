/**********************************************************************
--- Qt Architect generated file ---
File: Opc_client_comCommand.h
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#ifndef Opc_client_comCommand_included
#define Opc_client_comCommand_included
#include "opc_client_comCommandData.h"

//NOTA BENE: questa dialog viene caricata
//dalla dll nel client di IndigoSCADA (ist.exe)

class Opc_client_comCommand : public Opc_client_comCommandData
{
	Q_OBJECT
	QString Unit_type; // receipe name
	QString samplePointName;
	enum {tUnit = 1
	//	, tSerial
	}; // transaction codes
	public:
	Opc_client_comCommand(
	QWidget *parent = NULL,
	const char *name = NULL,
	const char *unit_type = NULL );
	virtual ~Opc_client_comCommand();
	protected slots:
	virtual void Help();
	virtual void OkClicked();
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
};
#endif // Opc_client_comCommand_included

