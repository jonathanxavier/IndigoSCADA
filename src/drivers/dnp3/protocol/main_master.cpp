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
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "dummy.hpp"
#include "station.hpp"
#include "master.hpp"
#include "datalink.hpp"

int main( int argc, char **argv )
{
    Master* master_p;
    DummyDb db;
    DummyTimer timer;
    int debugLevel = 1;
    DummyTx tx(&debugLevel, 'M', 'S');
    int integrityPollInterval = 10;

    Master::MasterConfig          masterConfig;
    Datalink::DatalinkConfig      datalinkConfig;
    Station::StationConfig        stationConfig;

    masterConfig.addr = 1;
    masterConfig.consecutiveTimeoutsForCommsFail = 3;
    masterConfig.integrityPollInterval_p = &integrityPollInterval;
    masterConfig.debugLevel_p = &debugLevel;

    stationConfig.addr = 2;
    stationConfig.debugLevel_p = &debugLevel;

    datalinkConfig.addr                  = masterConfig.addr;
    datalinkConfig.isMaster              = 1;
    datalinkConfig.keepAliveInterval_ms  = 10000;
    datalinkConfig.tx_p                  = &tx;
    datalinkConfig.debugLevel_p          = &debugLevel;

    master_p = new Master (masterConfig, datalinkConfig, &stationConfig, 1,
			   &db, &timer);

    // full resp to an integrity poll
    unsigned char r[] ={ 0x05, 0x64, 0x10, 0x44, 0x01, 0x00, 0x02, 0x00,
			 0x50, 0x8e, 0xc0, 0xc5, 0x81, 0x80, 0x00, 0x01,
			 0x02, 0x00, 0x00, 0x00, 0xff, 0xde, 0xa7 };

    while(1)
    {
        Sleep(1000);
        
        Bytes bytes1(r, r+sizeof(r));
        master_p->rxData( &bytes1, 0);
        //master_p->poll( Master::INTEGRITY);
        master_p->startNewTransaction();
        Bytes bytes2(r, r+sizeof(r));
        master_p->rxData( &bytes2, 0);
    }

	return 0;
}
