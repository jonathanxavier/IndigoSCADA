/**********************************************************************
--- Qt Architect generated file ---
File: modbus_masterInput.h
Last generated: Mon May 22 17:08:25 2000
*********************************************************************/
#ifndef modbus_masterInput_included
#define modbus_masterInput_included
#include "modbus_masterInputData.h"

class modbus_masterInput : public modbus_masterInputData
{
	Q_OBJECT
	public:
	enum  {  tConfigLoad = 1};
	modbus_masterInput(
	QWidget *parent = NULL,
	const char *name = NULL);
	virtual ~modbus_masterInput();

	QString Name;
	public slots:
	void Load(const QString &s);// load the configuration
	void Save(const QString &s);// save the configuration
};
#endif // modbus_masterInput_included

