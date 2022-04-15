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
*Header For: global databases
*Purpose:
*/

#ifndef include_realtime_db_hpp
#define include_realtime_db_hpp

#ifdef USING_GARRET
// we are using the GigaBASE SQL server and FastDB SQL server
// this runs on Win32 and Linux and other Unices
#include "qsfastdb.h"
#define REALTIME_DB QSFastdb
#endif

#include "general_defines.h"

#ifdef USE_STD_MAP
typedef std::map<QString, REALTIME_DB*, std::less<QString> > RealTimeDbDict; // the database dictionary
#else
#include "ds.h"
typedef SET RealTimeDbDict;
#endif

/////////////////////primary host real time databases/////////////////////////////
//
// open the primary databases
QSEXPORT bool OpenRealTimeConnections ();	
QSEXPORT bool ConnectToRealTimeDatabases();
// close them
QSEXPORT void CloseRealTimeConnections ();
// disconnect them
QSEXPORT void DisconnectFromRealTimeDatabases();
// acces the database channels
QSEXPORT QSDatabase *GetConfigureDb (); 
QSEXPORT QSDatabase *GetResultDb ();
QSEXPORT QSDatabase *GetCurrentDb ();

#ifdef USE_STD_MAP
QSEXPORT RealTimeDbDict& GetRealTimeDbDict();
#else
QSEXPORT RealTimeDbDict GetRealTimeDbDict();
#endif
//
#endif

