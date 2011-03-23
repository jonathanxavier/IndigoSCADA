/**********************************************************************
--- Qt Architect generated file ---
File: SelectActiveDlg.cpp
Last generated: Tue Apr 18 16:01:10 2000
*********************************************************************/
#include <qt.h>
#include "SelectActiveDlg.h"
#include "main.h"
#include "plancanvas.h"
#define Inherited SelectActiveDlgData

SelectActiveDlg::SelectActiveDlg
(
QWidget* parent,
PlanWindow *mappa,
const QString &iname,
const QString &itag,
const char* name
)
:
Inherited( parent, name ),pMap(mappa),iName(iname),iTag(itag)
{
	setCaption(tr("Select Active Object"));
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
			IconList->insertItem (fi->baseName ());
			++it;
		};
		IconList->setCurrentItem(0);
	};
	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the databas
	GetConfigureDb()->DoExec(this,"select NAME from SAMPLE order by NAME asc;",tList); // list the object names
}
//
SelectActiveDlg::~SelectActiveDlg()
{
}
void SelectActiveDlg::NameChanged(int)
{
	if(Name->count() > 0)
	{
		GetConfigureDb()->DoExec(this,"select TAG from TAGS where NAME='"
		+ Name->currentText() +"' and RECEIPE ='(default)' order by TAG asc;",tTags); // list the tags
	};
};
// database response
void SelectActiveDlg::QueryResponse (QObject *p, const QString &c, int id, QObject*caller)
{
	if(p != this) return;
	switch(id)
	{
		case tList:
		{
			unsigned n = GetConfigureDb()->GetNumberResults ();
			if (n)
			{
				for (unsigned i = 0; i < n; i++,GetConfigureDb()->FetchNext())
				{
					if(!pMap-> FindActiveObject(GetConfigureDb()->GetString ("NAME"))) // only put in the list if not on map
					{
						Name->insertItem (GetConfigureDb()->GetString ("NAME"));
					};
				};
				if(!iName.isEmpty())
				{
					Name->insertItem(iName); // add the existing name
				};
				SetComboItem(Name,iName);
				NameChanged(0);
			};
		};
		break;
		case tTags:
		{
			TagList->clear();  
			unsigned n = GetConfigureDb()->GetNumberResults ();
			if (n)
			{
				for (unsigned i = 0; i < n; i++,GetConfigureDb()->FetchNext())
				{
					TagList->insertItem (GetConfigureDb()->GetString ("TAG"));
				};
				SetComboItem(TagList,iTag); // try and find the inital tag item - might be helpful - maybe not
			};
		};
		break;
		default:
		break;
	};
};

