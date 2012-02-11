//
// $Id: dummy.hpp 4 2007-04-10 22:55:27Z sparky1194 $
//
// Copyright (C) 2007 Turner Technolgoies Inc. http://www.turner.ca
//
// Permission is hereby granted, free of charge, to any person 
// obtaining a copy of this software and associated documentation 
// files (the "Software"), to deal in the Software without 
// restriction, including without limitation the rights to use, 
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following 
// conditions:
//      
// The above copyright notice and this permission notice shall be 
// included in all copies or substantial portions of the Software. 
//      
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
// OTHER DEALINGS IN THE SOFTWARE.

// Modified by Enscada limited http://www.enscada.com

#ifndef DUMMY_H
#define DUMMY_H

#include <vector>
#include "event_interface.hpp"
#include "transmit_interface.hpp"
#include "timer_interface.hpp"

#include <winsock2.h>

class DummyDb : public EventInterface
{
public:
    DummyDb();
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

class DummyTx : public TransmitInterface
{
public:
    DummyTx(int* debugLevel_p, char name1, char name2, int sck);
    virtual Uptime_t transmit( const Lpdu& lpdu);
	int read(SOCKET s, void* buf, size_t min_size, size_t max_size, time_t timeout);
	bool write(SOCKET s, void const* buf, size_t size, time_t timeout);
    Bytes lastTxBytes;
    int numTxs;
    char n[3]; // two char id
    int* debug_p;
	int socket;

};

class DummyTimer : public TimerInterface
{
public:
    DummyTimer();
    void activate( TimerId timerId);
    void cancel( TimerId timerId);
    bool isActive( TimerId timerId);
    std::vector<bool> timerActive;

};


#endif
