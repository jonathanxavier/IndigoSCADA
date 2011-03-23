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
*Header For: archiving engine
*Purpose:
*/
#ifndef include_archiver_hpp 
#define include_archiver_hpp

#include "realtimedb.h"
#include "historicdb.h"
#include <map>
class SamplePoint;
class MessageDisplay;
struct NotificationData;

class Archiver : public QObject
{
	Q_OBJECT
	//
	bool fStarted;               // started flag
	//int MidnightReset;
	int lastHour; // used to track midnight resets
	int MaxRetryReconnectToDispatcher;
	int MaxRetryReconnectToRealTimeDb;
	int MaxRetryReconnectToHistoricDb;
	int MaxRetryReconnectToSpareDispatcher;
	int MaxRetryReconnectToSpareRealTimeDb;
	//
	public:
	//
	static Archiver * Instance; // instance of archiver object 
	Archiver(QObject *parent = 0);
	~Archiver();
	//
	enum 
	{	
		tUnitTypes = 1,tAlarmGroups,tReceipe,tReceipeRecord,tUnits,
		tSamples,tSerialPorts,tTags,tTable,tUpdateDone,tAllUpdated,
		tTagsCurrent,tGet,tSamplesCurrent
	};

	QTranslator translation;
	//
	//
	//Schedule *pSchedule; // the event schedular object
	//
	unsigned long SequenceNumber; // archiver's idea of a sort of time 
	bool fHalt; // halt flag
	//
	//void UpdateCurrentValue( const QString &name, SamplePoint &sp);
	//void ResetStatistics(); // reset the stats
	void ResetTables(); // reset the tables
	//
	public slots:
	//
	//
	void ConfigQueryResponse (QObject *,const QString &, int, QObject*);  // handles real time configuration responses
	void CurrentQueryResponse (QObject *,const QString &, int, QObject*); // current real time responses
	void ResultsQueryResponse (QObject *,const QString &, int, QObject*); // real time results responses
	void HistoricResultsQueryResponse (QObject *,const QString &, int, QObject*); // hisotric results responses
	void ReceivedNotify(int, const char *);
//	void SpareReceivedNotify(int, const char *);
	//
	void Start(); // start it all off
	void Stop(); // stop everything
	void Tick();
	void Trace(const QString &, const QString &);
	//
	signals:
	void DoCommand(const QString &, BYTE cmd, LPVOID lpPa, DWORD pa_length, DWORD ipindex);
	void TraceOut(const  QString &, const QString &); // trace out to listeners
};

#endif

