/**********************************************************************
--- Qt Architect generated file ---
File: Opcua_driverInput.h
Last generated: Mon May 22 17:08:25 2000
*********************************************************************/
#ifndef Opcua_driverInput_included
#define Opcua_driverInput_included
#include "opcua_driverInputData.h"

class Opcua_driverInput : public Opcua_driverInputData
{
	Q_OBJECT
	public:
	enum  {  tConfigLoad = 1};
	Opcua_driverInput(
	QWidget *parent = NULL,
	const char *name = NULL);
	virtual ~Opcua_driverInput();

	QString Name;
	public slots:
	void Load(const QString &s);// load the configuration
	void Save(const QString &s);// save the configuration
};
#endif // Opcua_driverInput_included

