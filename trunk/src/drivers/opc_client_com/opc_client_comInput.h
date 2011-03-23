/**********************************************************************
--- Qt Architect generated file ---
File: Opc_client_comInput.h
Last generated: Mon May 22 17:08:25 2000
*********************************************************************/
#ifndef Opc_client_comInput_included
#define Opc_client_comInput_included
#include "opc_client_comInputData.h"

class Opc_client_com_DriverThread;

class Opc_client_comInput : public Opc_client_comInputData
{
	Q_OBJECT
	public:
	enum  {  tConfigLoad = 1};
	Opc_client_comInput(
	QWidget *parent = NULL,
	const char *name = NULL);
	virtual ~Opc_client_comInput();

	QString Name;
	public slots:
	void Load(const QString &s);// load the configuration
	void Save(const QString &s);// save the configuration
};
#endif // Opc_client_comInput_included

