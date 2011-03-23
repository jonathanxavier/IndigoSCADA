/**********************************************************************
--- Qt Architect generated file ---
File: InputDlg.h
Last generated: Tue Apr 18 15:50:33 2000
*********************************************************************/
#ifndef InputDlg_included
#define InputDlg_included
#include "InputDlgData.h"
class InputDlg : public InputDlgData
{
	Q_OBJECT
	public:
	InputDlg(
	QWidget *parent = NULL,
	const char *name = NULL );
	virtual ~InputDlg();
	QString GetText() { return Name->text();};
};
#endif // InputDlg_included

