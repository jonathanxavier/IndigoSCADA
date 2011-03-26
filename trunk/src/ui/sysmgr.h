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

/*
*Header For: System Manager Window
*Purpose:
*/

#ifndef include_sysmgr_hpp 
#define include_sysmgr_hpp 
#include "mapapp.h"
#include "qt.h"
class MapApp;
class MessageDisplay;
class SysMgrFrameWork : public MapApp
{
	Q_OBJECT
	public:
	SysMgrFrameWork(QWidget *, const char*);
	~SysMgrFrameWork();
	public slots:
	void configureSystem();
	void configureUnits();
	void configureSamplePoints();
	void configureReceipes();
	void configureSchedule();
	void configurePorts();
	void configureAlarmGroups();
	void configureReport();
	void configureUser();
	void CloseSysMgr();
	void Help();
	public:
	static bool Active;
}; 
#endif

