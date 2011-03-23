/**********************************************************************
--- Qt Architect generated file ---
File: IconSelectDlg.h
Last generated: Tue Apr 18 15:52:48 2000
*********************************************************************/
#ifndef IconSelectDlg_included
#define IconSelectDlg_included
#include "IconSelectDlgData.h"
class IconSelectDlg : public IconSelectDlgData
{
	Q_OBJECT
	public:
	IconSelectDlg(
	QWidget *parent = NULL,
	const char *name = NULL );
	virtual ~IconSelectDlg();
	QString GetName() { return  List->text(List->currentItem());}; // place holder
};
#endif // IconSelectDlg_included

