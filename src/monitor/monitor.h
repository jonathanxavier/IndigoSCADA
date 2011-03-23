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
*Header For: monitoring engine
*Purpose:
*/
#ifndef include_monitor_hpp 
#define include_monitor_hpp

#include "realtimedb.h"
#include "historicdb.h"

#include <map>
class SamplePoint;
class Schedule;
class MessageDisplay;
class Driver;
class Dispatcher;
struct NotificationData;

class Monitor : public QObject
{
	Q_OBJECT
	//
	bool fStarted;               // started flag
	int MidnightReset;
	int lastHour; // used to track midnight resets
	int MaxRetryReconnectToDispatcher;
	int MaxRetryReconnectToRealTimeDb;
	int MaxRetryReconnectToHistoricDb;
	int MaxRetryReconnectToSpareDispatcher;
	int MaxRetryReconnectToSpareRealTimeDb;
	//
	public:
	//
	static Monitor * Instance; // instance of monitor object 
	Monitor(QObject *parent = NULL, RealTimeDbDict *db_dct = NULL, Dispatcher *dsp = NULL);
	~Monitor();
	//
	enum 
	{	
		tUnitTypes = 1,tAlarmGroups,tReceipe,tReceipeRecord,tUnits,
		tSamples,tSerialPorts,tTags,tTable,tUpdateDone,tAllUpdated,
		tTagsCurrent,tGet,tSamplesCurrent
	};

	QTranslator translation;
	//
	typedef std::map<QString, Driver *, std::less<QString> > DDict; // the driver dictionary
	DDict drivers;
	//
	Schedule *pSchedule; // the event schedular object
	//
	unsigned long SequenceNumber; // monitor's idea of a sort of time 
	bool fHalt; // halt flag
	//
	void UpdateCurrentValue( const QString &name, SamplePoint &sp);

	void Command(const QString & str, BYTE cmd, LPVOID lpPa, DWORD pa_length, DWORD ipindex)
	{
		emit DoCommand(str, cmd, lpPa, pa_length, ipindex);
	};

	void ResetStatistics(); // reset the stats
	void ResetTables(); // reset the tables
	//
	Dispatcher *dispatcher;
	RealTimeDbDict db_dictionary;
	QSDatabase *CfgDb;
	QSDatabase *CurDb;
	QSDatabase *ResDb;
	//
	public slots:
	//
	
	//Real time database on Host A 	(Primary)
	void ConfigQueryResponse (QObject *,const QString &, int, QObject*);  // handles configuration responses
	void CurrentQueryResponse (QObject *,const QString &, int, QObject*); // current value responses
	void ResultsQueryResponse (QObject *,const QString &, int, QObject*); // results responses

	//Real time database on Host B (Spare)
	//void SpareConfigQueryResponse (QObject *,const QString &, int, QObject*);  // handles configuration responses
	//void SpareCurrentQueryResponse (QObject *,const QString &, int, QObject*); // current value responses
	//void SpareResultsQueryResponse (QObject *,const QString &, int, QObject*); // results responses

	//Historic database
	void HistoricResultsQueryResponse (QObject *,const QString &, int, QObject*); // results responses

	void ReceivedNotify(int, const char *);
	//void SpareReceivedNotify(int, const char *);

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

