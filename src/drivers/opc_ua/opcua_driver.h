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
*Header For:Opcua
*
*Purpose:
*/

#ifndef include_opcua_driver_h 
#define include_opcua_driver_h

#include "driver.h"
#include "opcua_driverConfiguration.h"
#include "opcua_driverInput.h"
#include "opcua_driverCommand.h"
#include "sptypes.h"
#include "smplstat.h"
#include "common.h"

#ifdef OPCUA_DRIVER_EXPORTS
#define OPCUA_DRIVERDRV __declspec(dllexport)
#else
#define OPCUA_DRIVERDRV //__declspec(dllimport)
#endif

class Opcua_driver_Instance;

struct InstanceCfg // friend to anyone
{
	int context; //TCP or RTU
	//OPCUA TCP
	QString OPCUAServerIPAddress;  // OPCUA server IP address (slave)
	QString OPCUAServerTCPPort;  // OPCUA server TCP port
	//OPCUA RTU
	QString SerialDevice;
	QString Baud;
	QString DataBits;
	QString StopBit;
	QString Parity;
	QString RTSOnTime;
	QString RTSOffTime;

	unsigned SampleTime; // sampleing time 

	enum {
		TCP = 0,
		RTU = 1
	};

	InstanceCfg() : 
	SampleTime(1000),OPCUAServerIPAddress(""),OPCUAServerTCPPort(""),
	context(TCP),SerialDevice(""), Baud(""), DataBits(""), StopBit(""), Parity("")
	{
	};

	InstanceCfg(const InstanceCfg &m) :
		SampleTime(m.SampleTime),OPCUAServerIPAddress(m.OPCUAServerIPAddress),
		OPCUAServerTCPPort(m.OPCUAServerTCPPort),
		context(m.context),SerialDevice(m.SerialDevice), 
		Baud(m.Baud), DataBits(m.DataBits), StopBit(m.StopBit), Parity(m.Parity)
		{
		};
};


class OPCUA_DRIVERDRV Opcua_driver : public Driver // all this is private to this one file
{
	Q_OBJECT
	public:
	typedef std::map<QString,Opcua_driver_Instance *, std::less<QString> > IDict;
	IDict Instances;
	enum
	{
		tListUnits = 1, tcreateNewUnit
	};


	Opcua_driver(QObject *parent,const QString &name);
	~Opcua_driver();
	//
	// user interface stuff
	// 
	void UnitConfigure(QWidget *parent, const QString &name, const QString &receipe="(default)"); // configure a unit
	void SetTypeList(QComboBox *pCombo, const QString &unitname); // set the type list for unit type
	void GetInputList(const QString &type, QStringList &,const QString &unit, const QString &name); // set the permitted input IDs
	QWidget * GetSpecificConfig(QWidget *, const QString &spname, const QString &sptype); //specific config for sample point of type
	void GetTagList(const QString &type, QStringList &,const QString &,const QString &); // returns the permitted tags for a given type for this unit
	void CreateNewUnit(QWidget *parent, const QString &, int); // create a new unit - quick configure
	void CommandDlg(QWidget *parent, const QString &name); // command dialog

	//
	// 
	static Opcua_driver *pDriver; // only one instance should be created

	int n_iec_items;
	QString iec_unit_name;
	//
	// Actual driver stuff
	//
	// these are slots - the declarations are virtual in Driver
	//  
	public slots:
	void Start(); // start everything under this driver's control
	void Stop(); // stop everything under this driver's control
	void Command(const QString &, BYTE, LPVOID, DWORD, DWORD); // process a command for a named unit 
	void QueryResponse (QObject *, const QString &, int, QObject*);
};
#endif

