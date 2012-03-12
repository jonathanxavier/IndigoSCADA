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
*Header For:MPI
*
*Purpose:
*/

#ifndef include_mpi_driver_h 
#define include_mpi_driver_h

#include "driver.h"
#include "mpi_driverConfiguration.h"
#include "mpi_driverInput.h"
#include "mpi_driverCommand.h"
#include "sptypes.h"
#include "smplstat.h"
#include "common.h"

#ifdef MPI_DRIVER_EXPORTS
#define MPI_DRIVERDRV __declspec(dllexport)
#else
#define MPI_DRIVERDRV //__declspec(dllimport)
#endif

class Mpi_driver_Instance;

struct InstanceCfg // friend to anyone
{
	QString MPIServerIPAddress;  // MPI server IP address (slave)
	QString MPIServerIPPort;  // MPI server IP port (102 is the default)
	QString MPIServerSlot;  // MPI server slot
	QString PLCAddress;  // PLC address

	InstanceCfg() : 
	MPIServerIPAddress(""),MPIServerIPPort(""),MPIServerSlot(""),PLCAddress("")
	{
	};

	InstanceCfg(const InstanceCfg &m) : 
	MPIServerIPAddress(m.MPIServerIPAddress), MPIServerIPPort(m.MPIServerIPPort),
	MPIServerSlot(m.MPIServerSlot), PLCAddress(m.PLCAddress)
	{
	};
};


class MPI_DRIVERDRV Mpi_driver : public Driver // all this is private to this one file
{
	Q_OBJECT
	public:
	typedef std::map<QString,Mpi_driver_Instance *, std::less<QString> > IDict;
	IDict Instances;
	enum
	{
		tListUnits = 1, tcreateNewUnit
	};


	Mpi_driver(QObject *parent,const QString &name);
	~Mpi_driver();
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
	static Mpi_driver *pDriver; // only one instance should be created

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

