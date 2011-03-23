/**********************************************************************
--- Qt Architect generated file ---
File: Opc_client_comConfiguration.h
Last generated: Mon May 22 17:14:04 2000
*********************************************************************/
#ifndef Opc_client_comConfiguration_included
#define Opc_client_comConfiguration_included
#include "opc_client_comConfigurationData.h"

class Opc_client_comConfiguration : public Opc_client_comConfigurationData
{
	Q_OBJECT
	QString Receipe; // receipe name
	enum {tItem = 1, tSerial}; // transaction codes
	public:
	Opc_client_comConfiguration(
	QWidget *parent = NULL,
	const char *name = NULL,
	const char *receipe = NULL );
	virtual ~Opc_client_comConfiguration();
	protected slots:
	virtual void Help();
	virtual void OkClicked();
	void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
};
#endif // Opc_client_comConfiguration_included

