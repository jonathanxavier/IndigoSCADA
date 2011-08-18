/**********************************************************************
--- Qt Architect generated file ---
File: modbus_masterConfiguration.h
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#ifndef modbus_masterConfiguration_included
#define modbus_masterConfiguration_included
#include "modbus_masterConfigurationData.h"

class modbus_masterConfiguration : public modbus_masterConfigurationData
{
	Q_OBJECT
	QString Receipe; // receipe name
	enum {tItem = 1, tSerial}; // transaction codes
	public:
	modbus_masterConfiguration(
	QWidget *parent = NULL,
	const char *name = NULL,
	const char *receipe = NULL );
	virtual ~modbus_masterConfiguration();
	protected slots:
	virtual void Help();
	virtual void OkClicked();
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
};
#endif // modbus_masterConfiguration_included

