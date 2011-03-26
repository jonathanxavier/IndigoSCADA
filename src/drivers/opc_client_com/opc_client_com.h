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
*Header For:OPC client COM
*
*Purpose:Client OPC 2.0
*/

#ifndef include_opc_client_com_h 
#define include_opc_client_com_h

#include "driver.h"
#include "opc_client_comConfiguration.h"
#include "opc_client_comInput.h"
#include "opc_client_comCommand.h"
#include "sptypes.h"
#include "smplstat.h"
#include "common.h"

#ifdef OPC_CLIENT_COM_EXPORTS
#define OPC_CLIENT_COMDRV __declspec(dllexport)
#else
#define OPC_CLIENT_COMDRV //__declspec(dllimport)
#endif

//#define OPC_CLIENT_COM_DRIVER_NAME "OPC_Client_COM"

class Opc_client_com_Instance;

struct InstanceCfg // friend to anyone
{
	QString OpcServerProgID;  // opc server ProgID
	QString OpcServerIPAddress;  // opc server IP address

	unsigned SampleTime; // sampleing time 

	InstanceCfg() : 
	OpcServerProgID(""),SampleTime(1000),OpcServerIPAddress("")
	{
	};

	InstanceCfg(const InstanceCfg &m) : 

	OpcServerProgID(m.OpcServerProgID),SampleTime(m.SampleTime),OpcServerIPAddress(m.OpcServerIPAddress)
	{
	};
};


class OPC_CLIENT_COMDRV Opc_client_com : public Driver // all this is private to this one file
{
	Q_OBJECT
	public:
	typedef std::map<QString,Opc_client_com_Instance *, std::less<QString> > IDict;
	IDict Instances;
	enum
	{
		tListUnits = 1, tcreateNewUnit
	};


	Opc_client_com(QObject *parent,const QString &name);
	~Opc_client_com();
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
	static Opc_client_com *pDriver; // only one instance should be created

	int n_opc_items;
	QString opc_unit_name;
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

