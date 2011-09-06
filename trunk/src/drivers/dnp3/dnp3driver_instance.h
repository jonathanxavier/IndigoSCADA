/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2011 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef DNP_3_DRIVER__INSTANCE
#define DNP_3_DRIVER__INSTANCE

#include "dnp3driver.h"
#include "IndentedTrace.h"
#include "fifo.h"
#include "fifoc.h"

#define MAX_FIFO_SIZE 33554432UL//2^25

/* Information object */
struct iec_object {
	u_int		ioa;	/* information object address */
	union {
		 iec_type1	type1;
		 iec_type3	type3;
		 iec_type7	type7;
		 iec_type9	type9;
		 iec_type11	type11;
		 iec_type13	type13;
		 iec_type15	type15;
		 iec_type30	type30;
		 iec_type31	type31;
		 iec_type33	type33;
		 iec_type34	type34;
		 iec_type35	type35;
		 iec_type36	type36;
		 iec_type37	type37;
		 iec_type45	type45;
		 iec_type46	type46;
		 iec_type51	type51;
		 iec_type58	type58;
		 iec_type64	type64;
		 iec_type70	type70;
		 iec_type100	type100;
		 iec_type103	type103;
		 iec_type104	type104;
		 iec_type105	type105;
		 iec_type106	type106;
		 iec_type107	type107;
		 iec_type120	type120;
		 iec_type121	type121;
		 iec_type122	type122;
		 iec_type123	type123;
		 iec_type124	type124;
		 iec_type125	type125;
		 iec_type126	type126;
		 is_type150	type150;
		 is_type151	type151;
		 is_type152	type152;
		 is_type153	type153;
		 is_type154	type154;
		 is_type155	type155;
		 is_type156	type156;
	} o;	
};

#define ITEMID_LENGTH 99

//Record format of configuration database inside iec 104 master and slave
//the collection of all records form a table, which is the configuration database

struct iec_item
{
	char opc_server_item_id[ITEMID_LENGTH]; //Item ID of opc server
	u_char iec_type;
	struct iec_object iec_obj;
	DWORD hClient; 
	unsigned char cause; //Per distingure tra spontanee e general interrogation
	u_int   msg_id; //ID of the message
	u_int	checksum; //Checksum of the message
};

#define	C_SC_NA_1	45
#define	C_EX_IT_1 	200		//custom type
#define	C_IC_NA_1	100     //general interrogation

class DNP_3_DRIVERDRV Dnp3driver_Instance : public DriverInstance 
{
	Q_OBJECT
	//
	//
	enum
	{
		tUnitProperties = 1,tList, tSamplePointProperties, tListUnits, 
		tGetSamplePointNamefromIOA, tGetIOAfromSamplePointName
	};
	//
	//
	
//	QStringList SampleList; // list of sample points
	bool fFail;
	QTimer *pTimer; // timer object for driving state machine
	int Retry; // the retry count
	int Countdown; // the countdown track
	int State; // the state machine's state 
	
	//  
	int Sp; //Current sample point index under measurement
	bool InTick; //tick sentinal
	int IecItems;
	
	struct  Track
	{
		QString Name;           // name of sample point
		SampleStatistic Stats;  // we track the stats  
		double LastValue;       // the last value
		bool   fSpotValue;        // do we report the last value or the mean of the values over the sample period
		unsigned  SamplePeriod; // how often we sample 
		QDateTime NextSample;
		bool fFailed; // flag if the sample point is in a failed state
		void clear()
		{
			LastValue = 0.0; fSpotValue = false;
			NextSample = QDateTime::currentDateTime();
			fFailed = false;
			Stats.reset();
		}; 
	};
	//
	Track* Values;

	enum // states for the state machine
	{
		STATE_IDLE = 0,
		STATE_READ,
		STATE_WRITE,
		STATE_RESET,
		STATE_FAIL,
		STATE_DONE
	};

	public:
//	Dnp3driver_DriverThread *pConnect;
	fifo_h fifo_control_direction;
	unsigned int msg_sent_in_control_direction;
	fifo_h fifo_monitor_direction;

	//
	Dnp3driver_Instance(Driver *parent, const QString &name) : 
	DriverInstance(parent,name),fFail(0), Countdown(1),
	State(STATE_RESET),InTick(0),Retry(0),Sp(0),IecItems(1), Values(NULL),
	ParentDriver(parent),msg_sent_in_control_direction(0)
	{
		IT_IT("Dnp3driver_Instance::Dnp3driver_Instance");
		connect (GetConfigureDb (),
		SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
		SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database

		pTimer = new QTimer(this);
		connect(pTimer,SIGNAL(timeout()),this,SLOT(Tick()));
		pTimer->start(1000); // start with a 1 second timer

		/////////////////////////////////////////////////////////////////////////////
		const size_t max_fifo_queue_size = MAX_FIFO_SIZE;
		//Init thread shared fifos
		fifo_control_direction = fifo_open("fifo_control_direction1", max_fifo_queue_size);
		fifo_monitor_direction = fifo_open("fifo_monitor_direction1", max_fifo_queue_size);
	};

	~Dnp3driver_Instance()
	{    
		IT_IT("Dnp3driver_Instance::~Dnp3driver_Instance");

		if(Values)
		{
			delete[] Values;
			Values = NULL;
		}
	};
	//
	void Fail(const QString &s)
	{
		FailUnit(s);
		fFail = true;
	};

	InstanceCfg Cfg; // the cacheable stuff
	Driver* ParentDriver;
	QString unit_name;
	
	void driverEvent(DriverEvent *); // message from thread to parent
	bool event(QEvent *e);
	bool Connect();					//connect to the DriverThread
	bool Disconnect();              //disconnect from the DriverThread
	bool DoExec(SendRecePacket *t);
	bool expect(unsigned int cmd);
	void removeTransaction();


	//
	public slots:
	//
	virtual void Start(); // start everything under this driver's control
	virtual void Stop(); // stop everything under this driver's control
	virtual void Command(const QString & name, BYTE cmd, LPVOID lpPa, DWORD pa_length, DWORD ipindex); // process a command for a named unit 
	virtual void QueryResponse (QObject *, const QString &, int, QObject*); // handles database responses
	virtual void Tick();
	//
};

#endif