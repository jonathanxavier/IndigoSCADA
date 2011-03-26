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

/*h
*Header For: Map display window
*Purpose:
*/
#ifndef include_map_display_hpp 
#define include_map_display_hpp
#include <qt.h>
#include "plancanvas.h"
#include "dbase.h"
#include "realtimedb.h"
#include "statuspane.h"

class MapDisplay : public QMainWindow // the application frame
{
	Q_OBJECT 
	enum {tList = 1,tItem,tTagValue,tTags,tCval,tTagLimits};
	QString CurrentSp;
	QDateTime LastFetchTime; // when the last dat fetch was done
	QString Name;            // name of map to load or loaded
	PlanWindow  *pMap;           // the map display object
	QToolBar *pToolBar;      // the tool bar
	// 
	StatusPane Status;
	//  
	public:
	MapDisplay(QWidget *parent);
	~MapDisplay();
	//
	public slots:
	//
	void load(); // load the map
	void print(); // print a map
	//
	public slots:
	void QueryResponse (QObject *, const QString &, int, QObject*); // handle DB transactions
	void ActiveSelected(const QString &); // an active object was selected
	void UpdateSamplePoint();
	void UpdateTags(); // update the tags
	void LinkSelected(PlanLink *pL); // link handler
	void RightClicked(const QString &);
	void Restart();
};
#endif

