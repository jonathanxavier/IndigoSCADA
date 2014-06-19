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

#ifndef OPC_CLIENT_AE__INSTANCE
#define OPC_CLIENT_AE__INSTANCE

#include "opc_client_ae.h"
#include "IndentedTrace.h"
#include "clear_crc_eight.h"
#include "iec104types.h"
#include "iec_item.h"
////////////////////////////Middleware/////////////////////////////////////////////////////////////
#include "iec_item_type.h"
extern void onRegFail(void *param);
extern Boolean  quite;
extern void recvCallBack(const ORTERecvInfo *info,void *vinstance, void *recvCallBackParam); 
/////////////////////////////////////////////////////////////////////////////////////////////

class Opc_client_ae_DriverThread;

class OPC_CLIENT_AEDRV Opc_client_ae_Instance : public DriverInstance 
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
	int OpcItems;
	
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
	Opc_client_ae_DriverThread *pConnect;
	unsigned int msg_sent_in_control_direction;

	//
	Opc_client_ae_Instance(Driver *parent, const QString &name, int instance_id) : 
	DriverInstance(parent,name),fFail(0), Countdown(1), pConnect(NULL),
	State(STATE_RESET),InTick(0),Retry(0),Sp(0),OpcItems(1), Values(NULL),
	ParentDriver(parent),msg_sent_in_control_direction(0), instanceID(instance_id)
	{
		IT_IT("Opc_client_ae_Instance::Opc_client_ae_Instance");
		connect (GetConfigureDb (),
		SIGNAL (TransactionDone (QObject *, const QString &, int, QObject*)), this,
		SLOT (QueryResponse (QObject *, const QString &, int, QObject*)));	// connect to the database

		pTimer = new QTimer(this);
		connect(pTimer,SIGNAL(timeout()),this,SLOT(Tick()));
		pTimer->start(1000); // start with a 1 second timer

		/////////////////////Middleware/////////////////////////////////////////////////////////////////
		char fifo_control_name[150];
		char str_instance_id[20];
        itoa(instance_id + 1, str_instance_id, 10);
		strcpy(fifo_control_name,"fifo_control_direction");
        strcat(fifo_control_name, str_instance_id);
        strcat(fifo_control_name, "ae");

		ORTEDomainProp          dp; 
		ORTESubscription        *s = NULL;
		int32_t                 strength=1;
		NtpTime                 persistence,deadline,minimumSeparation,delay;
		Boolean                 havePublisher = ORTE_FALSE;
		Boolean                 haveSubscriber = ORTE_FALSE;
		IPAddress				smIPAddress = IPADDRESS_INVALID;
		ORTEDomainAppEvents     events;

		ORTEInit();
		ORTEDomainPropDefaultGet(&dp);
		NTPTIME_BUILD(minimumSeparation,0); 
		NTPTIME_BUILD(delay,1); //1s

		//initiate event system
		ORTEDomainInitEvents(&events);

		events.onRegFail = onRegFail;

		//Create application     
		domain = ORTEDomainAppCreate(ORTE_DEFAULT_DOMAIN,&dp,&events,ORTE_FALSE);

		iec_item_type_type_register(domain);

		//Create publisher
		NTPTIME_BUILD(persistence,5);
		
		publisher = ORTEPublicationCreate(
		domain,
		fifo_control_name,
		"iec_item_type",
		&instanceSend,
		&persistence,
		strength,
		NULL,
		NULL,
		NULL);
		
		char fifo_monitor_name[150];
		itoa(instance_id + 1, str_instance_id, 10);
		strcpy(fifo_monitor_name,"fifo_monitor_direction");
        strcat(fifo_monitor_name, str_instance_id);
        strcat(fifo_monitor_name, "ae");

		//Create subscriber
		NTPTIME_BUILD(deadline,3);

		subscriber = ORTESubscriptionCreate(
		domain,
		IMMEDIATE,
		BEST_EFFORTS,
		fifo_monitor_name,
		"iec_item_type",
		&instanceRecv,
		&deadline,
		&minimumSeparation,
		recvCallBack,
		this,
		smIPAddress);
		///////////////////////////////////Middleware//////////////////////////////////////////////////
	};

	~Opc_client_ae_Instance()
	{    
		IT_IT("Opc_client_ae_Instance::~Opc_client_ae_Instance");

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
	int instanceID; //Equals to "line concept" of a SCADA driver
	//////Middleware/////////////////////
    ORTEDomain *domain;
	ORTEPublication *publisher;
	ORTESubscription *subscriber;
	iec_item_type    instanceSend;
	iec_item_type    instanceRecv;
	///////////////////////////////
	
	void driverEvent(DriverEvent *); // message from thread to parent
	bool event(QEvent *e);
	bool Connect();					//connect to the DriverThread
	bool Disconnect();              //disconnect from the DriverThread
	bool DoExec(SendRecePacket *t);
	bool expect(unsigned int cmd);
	void removeTransaction();
	//////Middleware//////////////////////////////////////
	void get_utc_host_time(struct cp56time2a* time);
	void epoch_to_cp56time2a(cp56time2a *time, signed __int64 epoch_in_millisec);
	void get_items(struct iec_item* p_item);
	////////////////////////////////////////////////
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