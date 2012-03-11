/**********************************************************************
--- Qt Architect generated file ---
File: Mpi_driverConfiguration.h
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#ifndef Mpi_driverConfiguration_included
#define Mpi_driverConfiguration_included
#include "mpi_driverConfigurationData.h"

class Mpi_driverConfiguration : public Mpi_driverConfigurationData
{
	Q_OBJECT
	QString Receipe; // receipe name
	enum {tItem = 1, tSerial}; // transaction codes
	public:
	Mpi_driverConfiguration(
	QWidget *parent = NULL,
	const char *name = NULL,
	const char *receipe = NULL );
	virtual ~Mpi_driverConfiguration();
	protected slots:
	virtual void Help();
	virtual void OkClicked();
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
};
#endif // Mpi_driverConfiguration_included

