#ifndef MAPAPP_HPP
#define MAPAPP_HPP
/*
Header For: 
Map application class window
History:	
*/
#include <qt.h>
#include "plancanvas.h"
class MapApp : public QMainWindow
{
	Q_OBJECT
	enum
	{
		tLoad = 1,tSave,tNew
	};
	PlanWindow *pMap;
	public:
	MapApp(QWidget *parent, const char *name = 0);
	public slots:
	void FileOpen();
	void FileSave();
	void EditActiveObject(PlanActive *);
	void EditLinkObject(PlanLink *pA);
};
#endif

