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

#include <stdio.h>
#include "assert.h"
#include "custom.hpp"


// Event Interface -------------------

CustomDb::CustomDb() :
  numInits(0),
  numChanges(0)
{
}


void CustomDb::changePoint(   DnpAddr_t      addr,
			     DnpIndex_t     index,
		             PointType_t    pointType,
			     int            value,
			     DnpTime_t      timestamp)
{
    assert(addr != 0);
    numChanges++;
}

void  CustomDb::registerName( DnpAddr_t      addr,
			     DnpIndex_t     index,
			     PointType_t    pointType,
			     char*          name,
			     int            initialValue )
{
    assert(addr != 0);
}

// Transmit Interface -----------------

CustomInter::CustomInter(int* debugLevel_p, char name1, char name2, int sck) :
numTxs(0), debug_p(debugLevel_p), socket(NULL)
{
    n[0] = name1;
    n[1] = name2;
    n[2] = 0;
	socket = sck;
}

Uptime_t CustomInter::transmit( const Lpdu& lpdu)
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

int CustomInter::read(SOCKET s, void* buf, size_t min_size, size_t max_size, time_t timeout)
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

bool CustomInter::write(SOCKET s, void const* buf, size_t size, time_t timeout)
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

CustomTimer::CustomTimer() : 
  timerActive(TimerInterface::NUM_TIMERS, false)
{
}

void CustomTimer::activate( TimerId timerId)
{
    timerActive[timerId] = true;
}

void CustomTimer::cancel( TimerId timerId)
{
    timerActive[timerId] = false;
}

bool CustomTimer::isActive( TimerId timerId)
{
    return timerActive[timerId];
}

