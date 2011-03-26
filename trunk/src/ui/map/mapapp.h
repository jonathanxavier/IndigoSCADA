/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

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

