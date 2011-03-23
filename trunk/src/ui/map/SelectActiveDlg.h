/**********************************************************************
--- Qt Architect generated file ---
File: SelectActiveDlg.h
Last generated: Tue Apr 18 16:01:10 2000
*********************************************************************/
#ifndef SelectActiveDlg_included
#define SelectActiveDlg_included
#include "SelectActiveDlgData.h"
class PlanWindow;
class SelectActiveDlg : public SelectActiveDlgData
{
	Q_OBJECT
	enum { tList = 1,tTags};
	PlanWindow *pMap;
	QString iName;  // inital name
	QString iTag;   // inital tag
	public:
	SelectActiveDlg(
	QWidget *parent = NULL,
	PlanWindow *mappa = 0,
	const QString &iname = "",
	const QString &itag = "",
	const char *name = NULL );
	virtual ~SelectActiveDlg();
	QString GetActive() { return Name->currentText();};
	QString GetIcon() { return IconList->text(IconList->currentItem());};
	QString GetTagName() { return TagList->currentText();};
	void NameChanged(int);
	public slots:
	void QueryResponse (QObject *, const QString &, int, QObject*);
};
#endif // SelectActiveDlg_included

