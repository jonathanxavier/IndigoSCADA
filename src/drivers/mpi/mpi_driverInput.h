/**********************************************************************
--- Qt Architect generated file ---
File: Mpi_driverInput.h
Last generated: Mon May 22 17:08:25 2000
*********************************************************************/
#ifndef Mpi_driverInput_included
#define Mpi_driverInput_included
#include "mpi_driverInputData.h"

class Mpi_driverInput : public Mpi_driverInputData
{
	Q_OBJECT
	public:
	enum  {  tConfigLoad = 1};
	Mpi_driverInput(
	QWidget *parent = NULL,
	const char *name = NULL);
	virtual ~Mpi_driverInput();

	QString Name;
	public slots:
	void Load(const QString &s);// load the configuration
	void Save(const QString &s);// save the configuration
};
#endif // Mpi_driverInput_included

