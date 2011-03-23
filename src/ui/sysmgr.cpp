/*
*Header For: System Manager's window
*Purpose: The system manager application window allows configuration of the system 
*- only one system manager can be active at any one time
*/

#include "sysmgr.h"
// get the digital clock widget
#include "dclock.h"
#include "IndentedTrace.h"
#include "main.h"
#include "quit.xpm"
//
// ***************** dialogs *************************
// 
#include "AlarmGroupCfg.h"
#include "BatchCfg.h"
#include "ReceipeCfg.h"
#include "ReportCfg.h"
#include "SampleCfg.h"
#include "ScheduleCfg.h"
#include "SerialCfg.h"
#include "SystemCfg.h"
#include "TagCfg.h"
#include "UnitCfg.h"
#include "UserCfgDlg.h"
#include "driver.h"
//xpm
#include "fileopen.xpm"
#include "filesave.xpm"
#include "sysmgr.xpm"
#include "receipe.xpm"
#include "datasourcesuser.xpm"
#include "driver.xpm"
#include "computergreen.xpm"
#include "alarmsreport.xpm"
#include "batches.xpm"
#include "configreport.xpm"
#include "eventsreport.xpm"
#include "column.xpm"
#include "connecttool.xpm"
// 
// ***************************************************
//
bool SysMgrFrameWork::Active = 0;
/*-Function:SysMgrFrameWork
*Inputs:none
*Outputs:none
*Returns:none
*/
SysMgrFrameWork::SysMgrFrameWork(QWidget *parent, const char* name) : MapApp(parent)
{
	IT_IT("SysMgrFrameWork::SysMgrFrameWork");
	
	Active = true;
	menuBar()->clear();
	//
	// Construct the menu and tool bar
	//
	QPopupMenu *file = new QPopupMenu(this);
	file->insertItem(QPixmap((const char **)fileopen), tr("Open..."),this,SLOT(FileOpen()));
	file->insertItem(QPixmap((const char **)filesave),tr("Save..."),this,SLOT(FileSave()));
	file->insertItem(QPixmap((const char **)quit_xpm), tr("Close"),this,SLOT(CloseSysMgr()));
	menuBar()->insertItem(tr("&File"),file);
	//
	// add the various configuration options
	QPopupMenu *configMenu = new QPopupMenu(this);
	configMenu->insertItem(QPixmap((const char **)computergreen_xpm),tr("&Configure System..."),this,SLOT(configureSystem()));
	configMenu->insertItem(QPixmap((const char **)datasourcesuser_xpm),tr("&Configure Users..."),this,SLOT(configureUser()));
	configMenu->insertItem(QPixmap((const char **)driver_xpm),tr("&Configure Units..."),this,SLOT(configureUnits()));
	configMenu->insertItem(QPixmap((const char **)column_xpm),tr("&Configure Sample Points..."),this,SLOT(configureSamplePoints()));
	configMenu->insertItem(QPixmap((const char **)receipe_xpm),tr("&Configure Receipes..."),this,SLOT(configureReceipes()));
	configMenu->insertItem(QPixmap((const char **)eventsreport),tr("&Configure Scheduled Events..."),this,SLOT(configureSchedule()));
	configMenu->insertItem(QPixmap((const char **)magick),tr("&Configure Serial Ports..."),this,SLOT(configurePorts()));
	configMenu->insertItem(QPixmap((const char **)alarmsreport),tr("&Configure Alarm Groups..."),this,SLOT(configureAlarmGroups()));
	configMenu->insertItem(QPixmap((const char **)configreport),tr("&Configure Report..."),this,SLOT(configureReport()));
	menuBar()->insertItem(tr("&Configure..."),configMenu);	
	//
	// add help option
	// 
	QPopupMenu *help = new QPopupMenu(this);
	help->insertItem(tr("Help..."),this,SLOT(Help()));
	menuBar()->insertItem(tr("&Help"),help); 
	//
	//
	QToolBar *pToolBar = new QToolBar("System",this,QMainWindow::Bottom); // add a tool bar to the bottom
	//
	//
	DigitalClock *pL = new DigitalClock(pToolBar); // Add the digital Clock to the tool bar
	//pToolBar->setStretchableWidget(pL);
	//
	move(20,20);
	resize(700,500); // start with a sensible size
	//
	setCaption(tr(SYSTEM_NAME_SYSTEM_ADMINISTRATOR));
	QPixmap icon((const char **)sysmgr);
	setIcon(icon);
	//
	// Load up the DLLs
	//
	//il nome della dll (es. Simulator.dll --> Simulator) corrisponde alla colonna UNITTYPE
	//della tabella UNITS del database di configurazione: configdb.fdb
	//
	// Fill the unit type list
	QDir d (QSDRIVER_DIR, "*" DLL_EXT, QDir::Name, QDir::Files);	// get the directory listing
	
	if (d.count ())
	{
		const QFileInfoList *pD = d.entryInfoList ();
		QFileInfoListIterator it (*pD);
		QFileInfo *fi;
		//
		while ((fi = it.current ()))
		{
			FindDriver(fi->baseName());
			++it;
		}
	}
};
/*
*Function:~SysMgrFrameWork
*Inputs:none
*Outputs:none
*Returns:none
*/
SysMgrFrameWork::~SysMgrFrameWork()
{
	IT_IT("SysMgrFrameWork::~SysMgrFrameWork");

	Active = false;
	GetCurrentDb()->DoExec(0,"update PROPS set DVAL='0' where SKEY='System' and IKEY='Lock';",0);
	//
	//
};
/*
*Function: configureSystem
*system configuration dialog
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::configureSystem()
{
	IT_IT("SysMgrFrameWork::configureSystem");
	
	SystemCfg dlg(this);
	dlg.exec();
};
/*
*Function: configureUnits
*configure units
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::configureUnits()
{
	IT_IT("SysMgrFrameWork::configureUnits");
	
	UnitCfg dlg(this);
	dlg.exec();
};
/*
*Function: configure sample points
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::configureSamplePoints()
{
	IT_IT("SysMgrFrameWork::configureSamplePoints");
	
	SampleCfg dlg(this);
	dlg.exec();
};
/*
*Function: configureReceipes
*Inputs:none
*Outputs:none
*Returns:none
*/

void SysMgrFrameWork::configureReceipes()
{
	IT_IT("SysMgrFrameWork::configureReceipes");
	
	ReceipeCfg dlg(this);
	dlg.exec();
};
/*
*Function: configureSchedule
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::configureSchedule()
{
	IT_IT("SysMgrFrameWork::configureSchedule");
	
	ScheduleCfg dlg(this);
	dlg.exec();
};
/*
*Function: configurePorts
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::configurePorts()
{
	IT_IT("SysMgrFrameWork::configurePorts");
	
	SerialCfg dlg(this);
	dlg.exec();
};
/*
*Function:configureAlarmGroups
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::configureAlarmGroups()
{
	IT_IT("SysMgrFrameWork::configureAlarmGroups");
	
	AlarmGroupCfg dlg(this);
	dlg.exec();
};
/*
*Function: configureReport
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::configureReport()
{
	IT_IT("SysMgrFrameWork::configureReport");
	
	ReportCfg dlg(this);
	dlg.exec();
};
/*
*Function: CloseSysMgr
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::CloseSysMgr()
{
	IT_IT("SysMgrFrameWork::CloseSysMgr");
	
	if(Active)
	{
		// When we start we do a version control snap shot on the configuration database
		// this should go most of the way towards keeping CFR 21 Part 11 users happy
		// This catches configuration changes that we made but failed to run version control
		// (eg user interface crashed)
		// 
		// CFR 21 Part 11 is an FDA document 2 pages long with about 30 pages of explination
		// 
		QString cmd = QSBIN_DIR + "/version.sh"; // this is the script for doing this
		//
		// under Linux we can invoke cvs
		// We assume the postgres structures have been imported into CVS at this point
		// this assumes (oh dear !) that the system has been configured ready for use 
		// and then put under version control prior to validation (we are talking Pharamceutical parinoia here).
		// With postgres we can use pg_dump to produce a report file. It is this we hold under version control
		// as it is both human readable and can be used to regenerate any configuration
		// 
		// The number of tables (files) should not change but it might be necessary 
		// following upgrades or bug fixes to add new tables to CVS.
		// 
		QFile f(cmd);
		if(f.exists())
		{
			cmd += " \"Configuration Changes By " + GetUserDetails().Name + "\"";
			cmd += "&";
			system((const char *)cmd); 
		};
		close(true); // close and delete the window
	};
};
/*
*Function:configureUser
*configure users
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::configureUser()
{
	IT_IT("SysMgrFrameWork::configureUser");
	
	UserCfgDlg dlg(this);
	dlg.exec();
};
/*
*Function: Help
*Inputs:none
*Outputs:none
*Returns:none
*/
void SysMgrFrameWork::Help()
{
	IT_IT("SysMgrFrameWork::Help");

	QSHelp("SystemAdminMode");
};

