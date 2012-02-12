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

#ifndef CUSTOM_H
#define CUSTOM_H

#include <vector>
#include "event_interface.hpp"
#include "transmit_interface.hpp"
#include "timer_interface.hpp"
#include <time.h>
#include <winsock2.h>

class CustomDb : public EventInterface
{
public:
    CustomDb();
    virtual void changePoint(   DnpAddr_t      addr,
				DnpIndex_t     index,
				PointType_t    pointType,
				int            value,
				DnpTime_t      timestamp=0);

    virtual void registerName(  DnpAddr_t      addr,
				DnpIndex_t     index,
				PointType_t    pointType,
				char*          name,
				int            initialValue=0);
    int numInits;
    int numChanges;
};

class CustomInter : public TransmitInterface
{
public:
    CustomInter(int* debugLevel_p, char name1, char name2, int sck);
    virtual Uptime_t transmit( const Lpdu& lpdu);
	int read(SOCKET s, void* buf, size_t min_size, size_t max_size, time_t timeout);
	bool write(SOCKET s, void const* buf, size_t size, time_t timeout);
    Bytes lastTxBytes;
    int numTxs;
    char n[3]; // two char id
    int* debug_p;
	int socket;

};

class CustomTimer : public TimerInterface
{
public:
    CustomTimer();
    void activate( TimerId timerId);
    void cancel( TimerId timerId);
    bool isActive( TimerId timerId);
    std::vector<bool> timerActive;

};


#endif
