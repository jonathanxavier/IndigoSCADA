//
// $Id: dummy.cpp 4 2007-04-10 22:55:27Z sparky1194 $
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

#include <stdio.h>
#include "assert.h"
#include "dummy.hpp"


// Event Interface -------------------

DummyDb::DummyDb() :
  numInits(0),
  numChanges(0)
{
}


void DummyDb::changePoint(   DnpAddr_t      addr,
			     DnpIndex_t     index,
		             PointType_t    pointType,
			     int            value,
			     DnpTime_t      timestamp)
{
    assert(addr != 0);
    numChanges++;
}

void  DummyDb::registerName( DnpAddr_t      addr,
			     DnpIndex_t     index,
			     PointType_t    pointType,
			     char*          name,
			     int            initialValue )
{

}

// Transmit Interface -----------------

DummyTx::DummyTx(int* debugLevel_p, char name1, char name2, int sck) :
numTxs(0), debug_p(debugLevel_p), socket(NULL)
{
    n[0] = name1;
    n[1] = name2;
    n[2] = 0;
	socket = sck;
}

Uptime_t DummyTx::transmit( const Lpdu& lpdu)
{
    char buf[MAX_LEN*3+1];
    assert(lpdu.ab.size() >= 10);
    lastTxBytes = lpdu.ab;
    if (*debug_p > 0)
	printf( "%s Tx %s\n", n, hex_repr(lpdu.ab, buf,sizeof(buf)));

	char buf_to_send[MAX_LEN*3+1];

	for(unsigned int i = 0; i < lpdu.ab.size(); i++)
    {
        buf_to_send[i] = lpdu.ab[i];
    }
	
	//if(send(socket, (const char*)buf_to_send, lpdu.ab.size(),  0) == SOCKET_ERROR)   
    //{   
      //Send Error
    //}

	if(!write(socket, (const char*)buf_to_send, lpdu.ab.size(), 15))
	{
		//Error
		return 1;
	}

    numTxs++;
    return 0;
}

#define WAIT_FOREVER ((time_t)-1)

enum error_codes {
    ok = 0,
    not_opened = -1,
    broken_pipe = -2,
    timeout_expired = -3
};

int DummyTx::read(SOCKET s, void* buf, size_t min_size, size_t max_size, time_t timeout)
{ 
	int errcode = 0;
    size_t size = 0;
    time_t start = 0;

    if (timeout != WAIT_FOREVER) 
	{ 
        start = time(NULL); 
    }

    do{ 
        int rc;

        if (timeout != WAIT_FOREVER)
		{ 
            fd_set events;
            struct timeval tm;
            FD_ZERO(&events);
            FD_SET(s, &events);
            tm.tv_sec = (long)timeout;
            tm.tv_usec = 0;
            rc = select((int)s+1, &events, NULL, NULL, &tm);
            if (rc < 0) 
			{ 
                errcode = WSAGetLastError();
                fprintf(stderr, "Socket select is failed: %d\n", errcode);
			    fflush(stderr);

                return -1;
            }

            if (rc == 0) 
			{
                return size;
            }

            time_t now = time(NULL);
            timeout = start + timeout >= now ? timeout + start - now : 0;  
        }

        rc = recv(s, (char*)buf + size, max_size - size, 0);

        if (rc < 0) 
		{ 
            errcode = WSAGetLastError();
            fprintf(stderr,"Socket read is failed: %d\n", errcode);
			fflush(stderr);

            return -1;
        } 
		else if (rc == 0) 
		{
            errcode = broken_pipe;
            fprintf(stderr,"Socket is disconnected\n");
			fflush(stderr);
            return -1; 
        }
		else 
		{
            size += rc; 
        }

    }while (size < min_size); 

    return (int)size;
}

bool DummyTx::write(SOCKET s, void const* buf, size_t size, time_t timeout)
{ 
	int errcode = 0;
    time_t start = 0;

    if (timeout != WAIT_FOREVER) { 
        start = time(NULL); 
    }
    
    do { 
        int rc;
        if (timeout != WAIT_FOREVER) { 
            fd_set events;
            struct timeval tm;
            FD_ZERO(&events);
            FD_SET(s, &events);
            tm.tv_sec = (long)timeout;
            tm.tv_usec = 0;
            rc = select((int)s+1, NULL, &events, NULL, &tm);
            if (rc <= 0) { 
                errcode = WSAGetLastError();
                fprintf(stderr,"Socket select is failed: %d\n", errcode);
                return false;
            }
            time_t now = time(NULL);
            timeout = start + timeout >= now ? timeout + start - now : 0;  
        }
        rc = send(s, (char*)buf, size, 0);
        if (rc < 0) { 
            errcode = WSAGetLastError();
            fprintf(stderr,"Socket write is failed: %d\n", errcode);
            return false;
        } else if (rc == 0) {
            errcode = broken_pipe;
            fprintf(stderr,"Socket is disconnected\n");
            return false; 
        } else { 
            buf = (char*)buf + rc; 
            size -= rc; 
        }
    } while (size != 0); 

    return true;
}

// Timer Interface --------------------

DummyTimer::DummyTimer() : 
  timerActive(TimerInterface::NUM_TIMERS, false)
{
}

void DummyTimer::activate( TimerId timerId)
{
    timerActive[timerId] = true;
}

void DummyTimer::cancel( TimerId timerId)
{
    timerActive[timerId] = false;
}

bool DummyTimer::isActive( TimerId timerId)
{
    return timerActive[timerId];
}

