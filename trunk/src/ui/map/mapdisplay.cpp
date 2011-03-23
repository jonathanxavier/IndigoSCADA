/*h
*Header For: Map Display class
*Purpose:
*/
#include <qt.h>
#include "mapdisplay.h"        
#include "fileopen.xpm"
#include "printicon.xpm"
#include "main.h"
#include "dbase.h"
#include "common.h"
#include "inspect.h"
#include "AckAlarmDlg.h"
#include "inifile.h"
/*
*Function: MapDisplay
*Inputs:none
*Outputs:none
*Returns:none
*/
MapDisplay::MapDisplay(QWidget *parent) : QMainWindow(parent,"MapDisplayWindow") // construct 
{
	IT_IT("MapDisplay::MapDisplay");

	QSplitter * pS = new QSplitter (this);
	pMap = new PlanWindow(pS,"map",false);
	Status.Create(pS); // create the status inspector panes
	//
	QValueList<int> l;
	l.append(width() * 3 / 4);
	l.append(width() / 4);
	//
	pS->setSizes(l);
	// 
	connect (GetCurrentDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database
	// 
	// connect to the configuration database 
	// 
	connect (GetConfigureDb (),
	SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
	SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database
	//
	// catch active object selects 
	//  
	connect(pMap,SIGNAL(SelectActiveObject(const QString &)),this,SLOT( ActiveSelected(const QString &)));
	connect(pMap,SIGNAL(LinkSelected(PlanLink *)),this,SLOT(LinkSelected(PlanLink *))); // link handler
	//
	// right click opens inspector
	connect(pMap,SIGNAL(RightClicked(const QString &)),this,SLOT(RightClicked(const QString &)));
	//
	setCentralWidget(pS);
	//
	statusBar()->message(tr("Map Now Ready"));
	QPixmap openicon,saveicon,printicon; // icons
	//
	// Tool Bar
	setDockEnabled(QMainWindow::Bottom,TRUE);
	//
	pToolBar = new QToolBar("Map Display Tools",this,QMainWindow::Bottom);
	addToolBar(pToolBar,"Map Display Tools",QMainWindow::Bottom);
	openicon = QPixmap((const char **)fileopen);
	(void) new QToolButton(openicon,tr("Map Open"),0,this,SLOT(load()),pToolBar,"open the map");
	printicon = QPixmap((const char **)fileprint);
	(void) new QToolButton(printicon,tr("Map Print"),0,this,SLOT(print()),pToolBar,"print the map");
	//     
	// load the default map - if it exists 
	//
	QString ini_file = GetScadaHomeDirectory() + "\\bin\\scada.ini";
	Inifile iniFile((const char*)ini_file);
	//
	QString default_map;
	if( iniFile.find("map","intial_map"))
	{
		default_map = iniFile.find("map","intial_map");
	}

	QFile fs(QSMAPS_DIR + QString("/") + default_map);

	if(fs.open(IO_ReadOnly))
	{
		QDataStream is(&fs);
		pMap->Load(is);
		GetCurrentDb()->DoExec(this,"select * from TAGS_DB;",tTags); // refresh sample point states and values
		GetCurrentDb()->DoExec(this,"select * from CVAL_DB;",tCval); // refresh sample point states
	};
	connect(qApp->mainWidget(),SIGNAL(UpdateSamplePoint()),this,SLOT(UpdateSamplePoint()));
	connect(qApp->mainWidget(),SIGNAL(UpdateTags()),this,SLOT(UpdateTags()));
	connect(qApp->mainWidget(),SIGNAL(Restart()),this,SLOT(Restart()));
	//
	//
	setDockEnabled(pToolBar,Left,TRUE);
	setDockEnabled(pToolBar,Right,TRUE);
	setDockEnabled(pToolBar,Top,TRUE);
	setDockEnabled(pToolBar,Bottom,TRUE);
	setToolBarsMovable(TRUE);
	//
	//
};
/*-Function: ~MapDisplay
*Inputs:none
*Outputs:none
*Returns:none
*/
MapDisplay::~MapDisplay() // nothing to do
{
	IT_IT("MapDisplay::~MapDisplay");
};
/*-Function: load the map
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapDisplay::load()
{
	IT_IT("MapDisplay::load");

	QString f = QFileDialog::getOpenFileName(QSMAPS_DIR,"*.qsc");

	if(!f.isEmpty())
	{
		QFile fs(f);
		if(fs.open(IO_ReadOnly))
		{
			QDataStream is(&fs);
			pMap->Load(is);
			//
			GetCurrentDb()->DoExec(this,"select * from TAGS_DB;",tTags); // refresh sample point states and values
			GetCurrentDb()->DoExec(this,"select * from CVAL_DB;",tCval); // refresh sample point states
		};
	};
};
/*-Function: print
print a map
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapDisplay::print()
{
	IT_IT("MapDisplay::print");
	pMap->Print();
};
/*
*Function: GetPixmap
*fetch a pixmap
*Inputs:name of pixmap
*Outputs:none
*Returns:pixmap
*/
QPixmap *GetPixmap(const QString &s)
{
	IT_IT("GetPixmap");
	QString f = QSBMP_DIR + "/" + s + ".xpm"; // later extend to fetch from DB - for now cache
	return new QPixmap(f);
};
/*
*Function: QueryResponse
*thransactions fromthe configuration database
*Inputs:none
*Outputs:none
*Returns:none
*/
void  MapDisplay::QueryResponse(QObject *p, const QString &, int id, QObject*caller)
{
	IT_IT("MapDisplay::QueryResponse");

	if(p != this) return;

	switch(id)
	{
		case tCval:
		{
			UpdateSamplePoint();
			Status.UpdateSamplePoint();
		};
		break;
		case tTags:
		{
			Status.UpdateTagsPane();
			UpdateTags();
		};
		break;
		case tItem: // selected item
		{
			//
			// get the details - name, comment, units
			// this is the header
			// 
			Status.UpdateHeader();
		};
		break;
		case tTagLimits:
		{
			Status.UpdateTagsLimits();
		};
		break;
		default:
		break;
	};
};
/*
*Function:ActiveSelected
*handle active object being selected
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapDisplay::ActiveSelected(const QString &objname)
{
	IT_IT("MapDisplay::ActiveSelected");
	//
	// fetch the current state
	// save the current value name
	//
	CurrentSp = objname;
	Status.SetName(CurrentSp);
	//
	QString cmd = "select * from TAGS_DB where NAME = '" + objname + "';";
	GetCurrentDb()->DoExec(this,cmd,tTags); 
	//
	// get the sample point data 
	cmd = "select * from CVAL_DB where NAME='"+objname+"';";
	GetCurrentDb()->DoExec(this,cmd,tCval); 
	//
	// get the units , comment et c
	// 
	cmd = "select * from SAMPLE where NAME='"+objname+"';";
	GetConfigureDb()->DoExec(this,cmd,tItem);
	//    
	cmd = "select * from TAGS where NAME='"+Status.GetName()+"' and RECEIPE='"+GetReceipeName()+"';";
	GetConfigureDb()->DoExec(this,cmd,tTagLimits);
};
/*
*Function:ActiveMenu(const QString &name) 
show the active object menu
*Inputs:select object
*Outputs:none
*Returns:none
*/
void MapDisplay::RightClicked(const QString &name) // show the inspection window
{
	IT_IT("MapDisplay::RightClicked");
	//  
	// create the inspection window
	bool f = 0;
	PlanActive *p = pMap->FindActiveObject(name);
	if(p)
	{
		f = p->GetFlash(); // need an ack or wot ?
	};
	InspectMenu(pMap,name,f);
};
/*
*Function: UpdateSamplePoints
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapDisplay::UpdateSamplePoint()
{
	IT_IT("MapDisplay::UpdateSamplePoint");

	int n = GetCurrentDb()->GetNumberResults();

	GetCurrentDb()->GotoBegin();

	for(int i = 0; i < n; i++, GetCurrentDb()->FetchNext())
	{
		pMap->UpdateActiveColour(
		GetCurrentDb()->GetString("NAME"),
		GetAlarmStateBkColour(GetCurrentDb()->GetInt("STATE")));
		//
		// set the flash state based onthe acknowldege state
		pMap->UpdateActiveFlash(
		GetCurrentDb()->GetString("NAME"),
		GetCurrentDb()->GetInt("ACKFLAG"));
		//
	};

	pMap->repaint(true); // redraw the lot
};
/*
*Function:UpdateTags()
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapDisplay::UpdateTags() // handle tag updates - this updates the value string on the map
{
	IT_IT("MapDisplay::UpdateTags");

	int n = GetCurrentDb()->GetNumberResults();

	GetCurrentDb()->GotoBegin();

	for(int i = 0; i < n; i++, GetCurrentDb()->FetchNext())
	{
		QString name = GetCurrentDb()->GetString("NAME");
		//IT_COMMENT1("NAME = %s", (const char*)name);

		double v = atof((const char*)(GetCurrentDb()->GetString("VAL")));
		QString val = QString::number(v,'f',2); //two decimal points of precision

		//IT_COMMENT1("TAGNAME = %s", (const char*)GetCurrentDb()->GetString("TAGNAME"));

		pMap->UpdateActiveValue(name,
			GetCurrentDb()->GetString("TAGNAME"),
			val);
	};

	pMap->repaint(true); // redraw the lot
};
/*
*Function: LinkSelected
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapDisplay::LinkSelected(PlanLink *pL)
{
	IT_IT("MapDisplay::LinkSelected");

	QFile file(pL->GetName());

	if(file.exists())
	{
		if(file.open(IO_ReadOnly))
		{
			QDataStream is(&file);
			pMap->Load(is);
			//
			GetCurrentDb()->DoExec(this,"select * from TAGS_DB;",tTags); // refresh sample point states and values
			GetCurrentDb()->DoExec(this,"select * from CVAL_DB;",tCval); // refresh sample point states
			//
		};
	};                    
};
/*
*Function: Restart
*Inputs:none
*Outputs:none
*Returns:none
*/
void MapDisplay::Restart()
{
	IT_IT("MapDisplay::Restart");
	
	Status.Clear();

	QString ini_file = GetScadaHomeDirectory() + "\\bin\\scada.ini";
	Inifile iniFile((const char*)ini_file);

	QString default_map;
	if( iniFile.find("map","intial_map"))
	{
		default_map = iniFile.find("map","intial_map");
	}

	QFile fs(QSMAPS_DIR + QString("/") + default_map);

	if(fs.open(IO_ReadOnly))
	{
		QDataStream is(&fs);
		pMap->Load(is);
		GetCurrentDb()->DoExec(this,"select * from TAGS_DB;",tTags); // refresh sample point states and values
		GetCurrentDb()->DoExec(this,"select * from CVAL_DB;",tCval); // refresh sample point states
	};
};

