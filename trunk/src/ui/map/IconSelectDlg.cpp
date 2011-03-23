/**********************************************************************
--- Qt Architect generated file ---
File: IconSelectDlg.cpp
Last generated: Tue Apr 18 15:52:48 2000
*********************************************************************/
#include <qt.h>

#include "IconSelectDlg.h"
#define Inherited IconSelectDlgData
#include "common.h"

IconSelectDlg::IconSelectDlg
(
QWidget* parent,
const char* name
)
:
Inherited( parent, name )
{
	setCaption(tr("Select Icon"));
	// put a list of icons up from the bitmap directory
	// we support xpms - assume small 
	QDir d (QSBMP_DIR, "*.def", QDir::Name, QDir::Files);	// get the directory listing
	// 
	if (d.count ())
	{
		const QFileInfoList *pD = d.entryInfoList ();
		QFileInfoListIterator it (*pD);
		QFileInfo *fi;
		//
		while ((fi = it.current ()))
		{
			List->insertItem (fi->baseName ());
			++it;
		};
		List->setCurrentItem(0);
	};
}
IconSelectDlg::~IconSelectDlg()
{
}

