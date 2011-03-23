/*
Header For: Mapeditor frame work
Map application window
History:	
*/
#include <qt.h>
#include "mapapp.h"
#include "active.xpm"
#include "background.xpm"
#include "circle.xpm"
#include "fileopen.xpm"
#include "filesave.xpm"
#include "filloff.xpm"
#include "fillon.xpm"
#include "font.xpm"
#include "foreground.xpm"
#include "icon.xpm"
#include "line.xpm"
#include "linestyle.xpm"
#include "link.xpm"
#include "newmap.xpm"
#include "printicon.xpm"
#include "rect.xpm"
#include "textlabel.xpm"
#include "common.h"
#include "SelectActiveDlg.h"
#include "realtimedb.h"
#include "helper_functions.h"
//
// Construct and set up a map application window
/*
*Function:MapApp
*Inputs:none
*Outputs:none
*Returns:none
*/
MapApp::MapApp(QWidget *parent, const char *name)
: QMainWindow(parent,name,WDestructiveClose)
{
	//
	QToolBar *pTools = new QToolBar( this, "Operations" );
	pMap = new PlanWindow(this,"Editor",true);
	//
	// the icons
	QPixmap activeIcon((const char **)active), backIcon((const char **)background), circleIcon((const char **)circle),
	fileOpenIcon((const char **)fileopen), fileSaveIcon((const char **)filesave), fillOffIcon((const char **)filloff), 
	fillOnIcon((const char **)fillon),fontIcon((const char **)fonticon), foreIcon((const char **)foreground), 
	iconIcon((const char **)iconicon), 
	lineIcon((const char **)line), styleIcon((const char **)linestyle),linkIcon((const char **)linkmap),
	newMapIcon((const char **)newmap),
	printIcon((const char **)fileprint),rectIcon((const char **)recticon),textIcon((const char **)textlabel);
	// add the buttons
	(void) new QToolButton(newMapIcon,tr("New File"),QString::null,pMap,SLOT(newPlan()),pTools,"new file");
	(void) new QToolButton(fileOpenIcon,tr("Open File"),QString::null,this,SLOT(FileOpen()),pTools,"open file");
	(void) new QToolButton(fileSaveIcon,tr("Save File"),QString::null,this,SLOT(FileSave()),pTools,"save file");
	(void) new QToolButton(printIcon,tr("Print Map"),QString::null,pMap,SLOT(Print()),pTools,"print file");
	(void) new QToolButton(backIcon,tr("Background Colour"),QString::null,pMap,SLOT(setbackColour()),pTools,"background");
	(void) new QToolButton(foreIcon,tr("Foreground Colour"),QString::null,pMap,SLOT(setforeColour()),pTools,"foreground");
	(void) new QToolButton(fontIcon,tr("Select Font"),QString::null,pMap,SLOT(setFont()),pTools,"set font");
	(void) new QToolButton(fillOnIcon,tr("Fill On"),QString::null,pMap,SLOT(setFillOn()),pTools,"fill on");
	(void) new QToolButton(fillOffIcon,tr("Fill Off"),QString::null,pMap,SLOT(setFillOff()),pTools,"fill off");
	(void) new QToolButton(styleIcon,tr("Line Style"),QString::null,pMap,SLOT(setPen()),pTools,"line style");
	(void) new QToolButton(circleIcon,tr("Add Circle"),QString::null,pMap,SLOT(addCircle()),pTools,"circle");
	(void) new QToolButton(rectIcon,tr("Add Rectangle"),QString::null,pMap,SLOT(addRectangle()),pTools,"rectangle");
	(void) new QToolButton(lineIcon,tr("Add Line"),QString::null,pMap,SLOT(addLine()),pTools,"line");
	(void) new QToolButton(linkIcon,tr("Add Link"),QString::null,pMap,SLOT(addLink()),pTools,"link");
	(void) new QToolButton(textIcon,tr("Add Text"),QString::null,pMap,SLOT(addLabel()),pTools,"Text");
	(void) new QToolButton(activeIcon,tr("Add Active"),QString::null,pMap,SLOT(addActive()),pTools,"active");
	(void) new QToolButton(iconIcon,tr("Add Icon"),QString::null,pMap,SLOT(addIcon()),pTools,"icon");
	//
	addToolBar(pTools);	
	//
	setDockEnabled(pTools,Left,TRUE);
	setDockEnabled(pTools,Right,TRUE);
	setDockEnabled(pTools,Top,TRUE);
	setDockEnabled(pTools,Bottom,TRUE);
	setToolBarsMovable(TRUE);
	//
	statusBar()->message(tr("Ready")); // make sure we have a status bar
	setCentralWidget(pMap);	// insert the application window
	//
	connect(pMap,SIGNAL(ActiveObjectSelected(PlanActive *)),this,SLOT(EditActiveObject(PlanActive *)));		
	connect(pMap,SIGNAL(LinkSelected(PlanLink *)),this,SLOT(EditLinkObject(PlanLink *)));		
	//
};
/*
*Function: FileOpen
*Inputs:none
*Outputs:none
*Returns:none
*/
// open a map file
void MapApp::FileOpen()
{
	//
	// file open dialog
	//
	QString s = QFileDialog::getOpenFileName(QSMAPS_DIR,"*.qsc",this,"Load",tr("Load Plan File"));
	if(!s.isEmpty())
	{
		QFile f(s);
		if(f.open(IO_ReadOnly))
		{
			QDataStream is(&f);
			pMap->Load(is);
		};
	};
};
/*
*Function:
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapApp::EditLinkObject(PlanLink *pA)
{
	QPopupMenu p(pMap);
	p.insertItem(tr("&Edit..."),0);
	p.insertItem(tr("&Delete..."),1);
	int n = p.exec(QCursor::pos());
	switch(n)
	{
		case 0: // open the active object editor
		{
			QString f =  QFileDialog::getOpenFileName(QSMAPS_DIR + "/*."PLAN_EXT,tr("(Plan Files) *."PLAN_EXT),
			this,tr("Open"),tr("Select Plan To Link To"));
			//
			if(!f.isEmpty())
			{
				pA->SetName(f);
			};
		};
		break;
		case 1:
		{
			pMap->deleteSelectedObject();
		};
		break;
		default:
		break;
	};
};
/*
*Function:EditActiveObject()
*handle active object edtting
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapApp::EditActiveObject(PlanActive *pA)
{
	QPopupMenu p(pMap);
	p.insertItem(tr("&Edit..."),0);
	p.insertItem(tr("&Delete..."),1);
	int n = p.exec(QCursor::pos());
	switch(n)
	{
		case 0: // open the active object editor
		{
			SelectActiveDlg dlg(this,pMap,pA->GetName(),pA->GetTagName());
			if(dlg.exec())
			{
				QString obj(dlg.GetActive());
				QString bmp(dlg.GetIcon());
				pA->SetIconName(bmp);
				pA->SetName(obj);
				pA->SetTagName(dlg.GetTagName());
				pMap->repaint();
			};
		};
		break;
		case 1:
		{
			pMap->deleteSelectedObject();
		};
		break;
		default:
		break;
	};
};
/*
*Function:FileSave
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapApp::FileSave()
{
	//
	// file save dialog
	QString s = QFileDialog::getSaveFileName(QSMAPS_DIR,"*.qsc",this,"Save",tr("Save Plan File"));
	if(!s.isEmpty())
	{
		QFile f(s);
		if(f.open(IO_WriteOnly))
		{
			QDataStream is(&f);
			pMap->Save(is);
			QSAuditTrail(this,"Map",tr("Save Map:") + s);
		};
	};
};

