/**********************************************************************
--- Qt Architect generated file ---
File: Opcua_driverConfiguration.h
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#ifndef Opcua_driverConfiguration_included
#define Opcua_driverConfiguration_included
#include "opcua_driverConfigurationData.h"

class Opcua_driverConfiguration : public Opcua_driverConfigurationData
{
	Q_OBJECT
	QString Receipe; // receipe name
	enum {tItem = 1, tSerial}; // transaction codes
	enum {TCP = 0, RTU = 1}; // context codes
	int context;
	public:
	Opcua_driverConfiguration(
	QWidget *parent = NULL,
	const char *name = NULL,
	const char *receipe = NULL );
	virtual ~Opcua_driverConfiguration();
	protected slots:
	virtual void Help();
	virtual void OkClicked();
	virtual void RTUContextActive(bool);
    virtual void TCPContextActive(bool);
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
};
#endif // Opcua_driverConfiguration_included

