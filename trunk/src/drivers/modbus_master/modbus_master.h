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
*Header For:Modbus RTU ASCII
*
*Purpose:
*/

#ifndef include_modbus_master_h 
#define include_modbus_master_h

#include "driver.h"
#include "modbus_masterConfiguration.h"
#include "modbus_masterInput.h"
#include "modbus_masterCommand.h"
#include "sptypes.h"
#include "smplstat.h"
#include "common.h"

#ifdef MODBUS_MASTER_EXPORTS
#define MODBUS_MASTERDRV __declspec(dllexport)
#else
#define MODBUS_MASTERDRV //__declspec(dllimport)
#endif

class modbus_master_Instance;

struct InstanceCfg // friend to anyone
{
	QString MODBUServerIPAddress;  // Modbus IP address (slave)

	unsigned SampleTime; // sampleing time 

	InstanceCfg() : 
	SampleTime(1000),MODBUServerIPAddress("")
	{
	};

	InstanceCfg(const InstanceCfg &m) : 

	SampleTime(m.SampleTime),MODBUServerIPAddress(m.MODBUServerIPAddress)
	{
	};
};


class MODBUS_MASTERDRV modbus_master : public Driver // all this is private to this one file
{
	Q_OBJECT
	public:
	typedef std::map<QString,modbus_master_Instance *, std::less<QString> > IDict;
	IDict Instances;
	enum
	{
		tListUnits = 1, tcreateNewUnit
	};


	modbus_master(QObject *parent,const QString &name);
	~modbus_master();
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
	static modbus_master *pDriver; // only one instance should be created

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

