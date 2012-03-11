/**********************************************************************
--- Qt Architect generated file ---
File: Mpi_driverCommand.h
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#ifndef Mpi_driverCommand_included
#define Mpi_driverCommand_included
#include "mpi_driverCommandData.h"

//NOTA BENE: questa dialog viene caricata
//dalla dll nel client di IndigoSCADA (ist.exe)

class Mpi_driverCommand : public Mpi_driverCommandData
{
	Q_OBJECT
	QString Unit_type; // receipe name
	QString samplePointName;
	enum {tUnit = 1
	//	, tSerial
	}; // transaction codes
	public:
	Mpi_driverCommand(
	QWidget *parent = NULL,
	const char *name = NULL,
	const char *unit_type = NULL );
	virtual ~Mpi_driverCommand();
	protected slots:
	virtual void Help();
	virtual void OkClicked();
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
};
#endif // Mpi_driverCommand_included

