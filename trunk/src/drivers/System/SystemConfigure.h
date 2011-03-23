/**********************************************************************
--- Qt Architect generated file ---
File: SystemConfigure.h
Last generated: Fri Sep 8 16:30:38 2000
*********************************************************************/
#ifndef SystemConfigure_included
#define SystemConfigure_included
#include "SystemConfigureData.h"
class SystemConfigure : public SystemConfigureData
{
	Q_OBJECT
	enum{tItem = 1};

	QString Name;
	public:
	SystemConfigure(
	QWidget *parent = NULL,
	const char *name = NULL );
	virtual ~SystemConfigure();
	protected slots:
	virtual void OkClicked();
	void QueryResponse (QObject *p, const QString &c, int id, QObject*);
	void Help();
};
#endif // SystemConfigure_included

