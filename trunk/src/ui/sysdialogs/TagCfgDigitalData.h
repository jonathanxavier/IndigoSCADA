/**********************************************************************
--- Qt Architect generated file ---
File: TagCfgData.h
Last generated: Thu Jan 4 16:04:27 2001
DO NOT EDIT!!!  This file will be automatically
regenerated by qtarch.  All changes will be lost.
*********************************************************************/
#ifndef TagCfgDigitalData_included
#define TagCfgDigitalData_included
#include <qt.h>

class TagCfgDigitalData : public QDialog
{
	Q_OBJECT
	public:
	TagCfgDigitalData(QWidget *parent = NULL, const char *name = NULL);
	virtual ~TagCfgDigitalData();
	protected slots:
	virtual void Help() =0;
	virtual void SelChanged(int) =0;
	virtual void Apply() =0;
	protected:
	QComboBox *Name;
	QCheckBox *UAEnabled;
	QLineEdit *UpperAlarm;
	QPushButton *ApplyButton;
};
#endif // TagCfgDigitalData_included
