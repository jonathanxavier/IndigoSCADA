//
// $Id: factory.cpp 23 2007-04-16 18:46:37Z sparky1194 $
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

// Some portions are Copyright (C) 2012 Enscada Limited http://www.enscada.com

//////////////////////////apa+++ 12-06-2012///////////
#include "stdint.h"
#include "iec104types.h"
#include "iec_item.h"
#include "clear_crc_eight.h"
#include "iec_item_type.h" //Middleware
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>  //For sleep
#undef WIN32_LEAN_AND_MEAN
#endif
//////////////////////////////////////////////////////
#include "assert.h"
#include "stdio.h"
#include "common.hpp"
#include "stats.hpp"
#include "factory.hpp"

const uint8_t ObjectHeader::PACKED_WITHOUT_A_PREFIX = 0;
const uint8_t ObjectHeader::ONE_OCTET_INDEX         = 1;
const uint8_t ObjectHeader::TWO_OCTET_INDEX         = 2;
const uint8_t ObjectHeader::FOUR_OCTET_INDEX        = 3;
const uint8_t ObjectHeader::ONE_OCTET_SIZE          = 4;
const uint8_t ObjectHeader::TWO_OCTET_SIZE          = 5;
const uint8_t ObjectHeader::FOUR_OCTET_SIZE         = 6;

// range specifier codes
const uint8_t ObjectHeader::ONE_OCTET_START_STOP_INDEXES               = 0;
const uint8_t ObjectHeader::TWO_OCTET_START_STOP_INDEXES               = 1;
const uint8_t ObjectHeader::FOUR_OCTET_START_STOP_INDEXES              = 2;
const uint8_t ObjectHeader::ONE_OCTET_START_STOP_VIRTUAL_ADDRESSES     = 3;
const uint8_t ObjectHeader::TWO_OCTET_START_STOP_VIRTUAL_ADDRESSES     = 4;
const uint8_t ObjectHeader::FOUR_OCTET_START_STOP_VIRTUAL_ADDRESSES    = 5;
const uint8_t ObjectHeader::NO_RANGE_FIELD                             = 6;
const uint8_t ObjectHeader::ONE_OCTET_COUNT_OF_OBJECTS                 = 7;
const uint8_t ObjectHeader::TWO_OCTET_COUNT_OF_OBJECTS                 = 8;
const uint8_t ObjectHeader::FOUR_OCTET_COUNT_OF_OBJECTS                = 9;
const uint8_t ObjectHeader::ONE_OCTET_COUNT_OF_OBJECTS_VARIABLE_FORMAT =0xb;

ObjectHeader::ObjectHeader(uint8_t group,
			   uint8_t variation,
			   uint8_t qualifier,
			   uint32_t countOfObjects,
			   uint32_t startIndex,
			   uint32_t stopIndex ) :
  grp(group), var(variation), qual(qualifier), count(countOfObjects),
  start(startIndex), stop(stopIndex)
{
    indexSize = (qual & 0x70) >> 4;
    rangeSpecifier = qual & 0x0f;
}

char* ObjectHeader::str( char* buf, int len) const
{
    sprintf( buf, "Oh: Grp:%d,Var=%d",grp,var);
    return buf;
}

void ObjectHeader::encode( Bytes& data) const
{
    data.push_back( grp);
    data.push_back( var);
    data.push_back( qual);

    if (rangeSpecifier == ONE_OCTET_START_STOP_INDEXES)
    {
		// range size is two bytes
		data.push_back( start);
		data.push_back( stop);
    }
    else if (rangeSpecifier == TWO_OCTET_START_STOP_INDEXES)
    {
		// range size is four bytes
		appendUINT16(data, start);
		appendUINT16(data, stop);
    }
    else if (rangeSpecifier == ONE_OCTET_COUNT_OF_OBJECTS)
    {
		// range size is one byte
		data.push_back(  count);
    }
    else if (rangeSpecifier == TWO_OCTET_COUNT_OF_OBJECTS)
    {
		// range size is two bytes
		appendUINT16(data, count);
    }
    else if (rangeSpecifier == ONE_OCTET_COUNT_OF_OBJECTS_VARIABLE_FORMAT)
    {
		data.push_back( count);
    }
    else if (rangeSpecifier == NO_RANGE_FIELD)
    {
    }
    else
    {
		Stats::log(0, 0, "Unsupported Qualifier code 0x%x", qual);
		assert(0);
    }
}

void ObjectHeader::decode( Bytes& data, Stats& stats) throw(int)
{
    grp = data[0]; data.pop_front();
    var = data[0]; data.pop_front();

    // this next byte contains both the index size and the range specifier
    qual = data[0]; data.pop_front();
    indexSize = (qual & 0x70) >> 4;
    rangeSpecifier = qual & 0x0f;

    if (rangeSpecifier == ONE_OCTET_START_STOP_INDEXES)
    {
		// range size is two bytes
		start = data[0]; data.pop_front();
		stop = data[0]; data.pop_front();
    }
    else if (rangeSpecifier == TWO_OCTET_START_STOP_INDEXES)
    {
		// range size is four bytes
		start = data[0] + (data[1] << 8);
		data.pop_front();
		data.pop_front();

		stop = data[0] + (data[1] << 8);
		data.pop_front();
		data.pop_front();
    }
    else if (rangeSpecifier == NO_RANGE_FIELD)
    {
		// range size is zero bytes
    }
    else if (rangeSpecifier == ONE_OCTET_COUNT_OF_OBJECTS)
    {
		// range size is one byte
		count = data[0]; data.pop_front();
		}
		else if (rangeSpecifier == TWO_OCTET_COUNT_OF_OBJECTS)
		{
		// range size is two bytes
		count = data[0] + (data[1] << 8);
		data.pop_front();
		data.pop_front();
    }
    else if (rangeSpecifier == ONE_OCTET_COUNT_OF_OBJECTS_VARIABLE_FORMAT)
    {
		count = data[0]; data.pop_front();
    }
    else
    {
		stats.logAbnormal(0, "Unsupported Qualifier code 0x%x", qual);
		throw(__LINE__);
    }
}

Factory::DnpObjectMap Factory::objectMap;

Factory::Factory(EventInterface* eventInterface_p) :
  db_p(eventInterface_p),
  cto(0)
{
    // 1,2 is used when decoding 1,1 and 3,1
    // this is because the object is less than one byte
    objectMap[ key(  1,  2)] = new BinaryInputWithStatus();
    objectMap[ key(  2,  1)] = new BinaryInputEventNoTime();
    objectMap[ key(  2,  2)] = new BinaryInputEvent();
    objectMap[ key(  2,  3)] = new BinaryInputEventRelativeTime();
    objectMap[ key( 10,  2)] = new BinaryOutputStatus();
    objectMap[ key( 12,  1)] = new ControlOutputRelayBlock();
    objectMap[ key( 20,  1)] = new Bit32BinaryCounter();
    objectMap[ key( 20,  2)] = new Bit16BinaryCounter();
    objectMap[ key( 20,  3)] = new Bit32DeltaCounter();
    objectMap[ key( 20,  4)] = new Bit16DeltaCounter();
    objectMap[ key( 20,  5)] = new Bit32BinaryCounterNoFlag();
    objectMap[ key( 20,  6)] = new Bit16BinaryCounterNoFlag();
    objectMap[ key( 20,  7)] = new Bit32DeltaCounterNoFlag();
    objectMap[ key( 20,  8)] = new Bit16DeltaCounterNoFlag();
    objectMap[ key( 22,  1)] = new Bit32CounterEventNoTime();
    objectMap[ key( 22,  2)] = new Bit16CounterEventNoTime();
    objectMap[ key( 22,  3)] = new Bit32DeltaCounterEventNoTime();
    objectMap[ key( 22,  4)] = new Bit16DeltaCounterEventNoTime();
    objectMap[ key( 30,  1)] = new Bit32AnalogInput();
    objectMap[ key( 30,  2)] = new Bit16AnalogInput();
    objectMap[ key( 30,  3)] = new Bit32AnalogInputNoFlag();       
    objectMap[ key( 30,  4)] = new Bit16AnalogInputNoFlag();
    objectMap[ key( 32,  1)] = new Bit32AnalogEventNoTime();
    objectMap[ key( 32,  2)] = new Bit16AnalogEventNoTime();
    objectMap[ key( 40,  2)] = new Bit16AnalogOutputStatus();
    objectMap[ key( 41,  2)] = new Bit16AnalogOutput();
    objectMap[ key( 50,  1)] = new TimeAndDate();
    objectMap[ key( 51,  1)] = new TimeAndDateCTO();
    objectMap[ key( 51,  2)] = new UnsyncronizedTimeAndDateCTO();
    objectMap[ key( 52,  1)] = new TimeDelayCoarse();
    objectMap[ key( 52,  2)] = new TimeDelayFine();
    objectMap[ key(120,  1)] = new Challenge();
    objectMap[ key(120,  2)] = new Reply();
    objectMap[ key(120,  3)] = new AggressiveModeRequest();
    objectMap[ key(120,  4)] = new SessionKeyStatusReq();
    objectMap[ key(120,  5)] = new SessionKeyStatus();
    objectMap[ key(120,  6)] = new SessionKeyChange();
    objectMap[ key(120,  7)] = new AuthenticationError();

    selfTest();
}

// this method could be expanded to include tests for the variable
// sized and other more complex objects
void Factory::selfTest()
{
    DnpObjectMap::iterator iter;

    // the simple encode and decode test only works for objects that
    // have implemented these methods.

    ObjectKey lastObjectToTest =  key(52,2);

    for(iter = objectMap.begin(); iter->first != lastObjectToTest; iter++)
    {
		assert (iter != objectMap.end());

		Bytes data;
		DnpObject* o = iter->second;

		try
		{
			o->encode( data);
			o->decode( data);
		}
		catch (int e)
		{
			printf ("Testing grp=%d var=%d\n", ((iter->first & 0xff00) >> 8),
				iter->first & 0xff);
			printf ("Exception line# %d\n", e);
			assert(0);
		}
    }
}

Factory::ObjectKey Factory::key(uint8_t group, uint8_t variation)
{
    return ( (group<<8) + variation);
}


DnpObject* Factory::decode(const ObjectHeader& oh, Bytes& data,
			     DnpAddr_t addr,
			     Stats& stats) throw(int)
{
    uint32_t num;

    if (oh.rangeSpecifier == ObjectHeader::TWO_OCTET_START_STOP_INDEXES ||
	oh.rangeSpecifier == ObjectHeader::ONE_OCTET_START_STOP_INDEXES    )
    {
		if (oh.indexSize == ObjectHeader::PACKED_WITHOUT_A_PREFIX)
		{
			num = oh.stop - oh.start + 1;
			stats.logNormal( "Decoding: %d object(s)", num);
			createObjects(oh.grp, oh.var, data, oh.start, oh.stop,addr,stats);
		}
		else
		{
			stats.logAbnormal(0, "Rx Unsupported Qualifier %d", oh.qual);
			throw(__LINE__);
		}
    }
    else if (oh.rangeSpecifier == ObjectHeader::TWO_OCTET_COUNT_OF_OBJECTS ||
	     oh.rangeSpecifier == ObjectHeader::ONE_OCTET_COUNT_OF_OBJECTS )
    {
		uint32_t index;
		stats.logNormal("Decoding: %d object(s)", oh.count);
		for (unsigned int i=0; i<oh.count; i++)
		{
			if (oh.indexSize == 2)
			{
			index = removeUINT16(data);
			}
			else if (oh.indexSize == 1)
			{
			index = removeUINT8(data);
			}
			else if (oh.indexSize == 0)
			index = NO_INDEX;
			else
			{
			// something has gone wrong
			stats.logAbnormal(0,
					  "Rx Unsupported Index Size %d",oh.indexSize);
			throw(__LINE__);
			}

			createObjects(oh.grp, oh.var, data, index, index, addr, stats);
		}
    }
    else if (oh.rangeSpecifier ==
	ObjectHeader::ONE_OCTET_COUNT_OF_OBJECTS_VARIABLE_FORMAT )
    {
		uint32_t objectSize;
		for (unsigned int i=0; i<oh.count; i++)
		{
			// index size here is really used for the size of the object size
			if (oh.indexSize == ObjectHeader::ONE_OCTET_SIZE)
			objectSize = removeUINT8(data);
			else if (oh.indexSize == ObjectHeader::TWO_OCTET_SIZE)
			objectSize = removeUINT16(data);
			else if (oh.indexSize == ObjectHeader::FOUR_OCTET_SIZE)
			objectSize = removeUINT32(data);
			else
			{
			stats.logAbnormal(0,"Unsupported Qualifier code 0x%x",oh.qual);
			throw(__LINE__);
			}

			stats.logNormal( "Decoding: 1 object of size %d", objectSize);
			createObjects(oh.grp, oh.var, data,
				  NO_INDEX,
				  NO_INDEX,
				  addr,
				  stats,
				  objectSize );
		}
    }
    else
    {	
		stats.logAbnormal(0, "Rx Unsupported Qualifier %d", oh.qual);
		throw(__LINE__);
    }

    return lastObjectParsed;
}

void Factory::createObjects(uint8_t grp, uint8_t var, Bytes& data,
			    uint32_t startIndex, uint32_t stopIndex,
			    DnpAddr_t addr, Stats& stats,
			    uint32_t objectSize) throw(int)
{
    uint32_t  i;
    DnpObject* obj_p = NULL; // we should never return NULL
    // if we do have toubl parseing with should throw an exception

    // Binary Input is a special case because it is packed 
    if (grp == 1 && var == 1)
    {
 		uint8_t bitMask = 0x01;
		uint8_t flag;
 		for (i=startIndex; i<stopIndex+1; i++)
		{
			BinaryInputWithStatus bi;
			if (data[0] & bitMask)
			flag = 0x81;
			else
			flag = 0x01;

			bi = BinaryInputWithStatus( flag);
			db_p->changePoint( addr, i,
					   bi.pointType,
					   bi.value,
					   bi.timestamp);

			if ((bitMask == 0x80) || (i == stopIndex))
			data.pop_front();

			if (bitMask == 0x80)
			bitMask = 0x01;
			else
			bitMask = bitMask << 1;
		}
    }
    // Double bit Binary Input is a special case because it is packed 
    else if (grp == 3 && var == 1)
    {
 		uint8_t bitMask = 0x03;
		uint8_t shift   = 0x00;
		uint8_t flag;
 		for (i=startIndex; i<stopIndex+1; i++)
		{
			BinaryInputWithStatus bi;
			if ( ((data[0] & bitMask) >> shift) == 0x02)
			flag = 0x81;
			else
			flag = 0x01;

			bi = BinaryInputWithStatus( flag);
			db_p->changePoint( addr, i,
					   bi.pointType,
					   bi.value,
					   bi.timestamp);

			if ((bitMask == 0xC0) || (i == stopIndex))
			data.pop_front();

			if (bitMask == 0xC0)
			{
			bitMask = 0x03;
			shift = 0;
			}
			else
			{
			bitMask = bitMask << 2;
			shift += 2;
			}

		}
    }
    else
    {
		if (objectMap.count( key( grp, var)) == 0)
		{
			stats.logAbnormal(0,"Rx Unsupported Object grp=%d var=%d",grp,var);
			throw(__LINE__);
		}

		obj_p = objectMap[ key( grp, var)];

		if (startIndex == NO_INDEX)
		{
			if (objectSize > 0)
			{
			// this is a variable sized object and the object size
			// was specified in the header
			obj_p->decode(data, objectSize);
			}
			else
			{
			obj_p->decode(data);    // init instance
			}
		}
		else
		{
			for (i=startIndex; i<stopIndex+1; i++)
			{
			obj_p->decode(data);    // init instance

			if ((grp == 2) && (var == 2))
				// handle another special case
				// we need to add the CTO to get a dnp time
				obj_p->timestamp += cto;
			
			db_p->changePoint(addr, i,
					  obj_p->pointType,
					  obj_p->value,
					  obj_p->timestamp);
			}
		}
    }

    lastObjectParsed = obj_p;

	sendObjectInMonitorDirection(obj_p);
}

void Factory::setCTO( DnpTime_t newCTO)
{
    cto = newCTO;
}

////////////////////////////////////////////apa+++////////////////////////////////////////////
#define ABS(x) ((x) >= 0 ? (x) : -(x))
static unsigned int n_msg_sent = 0;

/////////////////////////////////Globals remove ASAP/////////////
extern iec_item_type* global_instanceSend;
extern ORTEPublication* global_publisher;
/////////////////////////////////////////////////////////////////

#define QUALITY_GOOD 0x01

void Factory::sendObjectInMonitorDirection(DnpObject* obj_p)
{
	cp56time2a time;
	struct iec_item item_to_send;
	//double delta = 0.0;

	if(obj_p == NULL)
	{
		//print error message
		return;
	}
    	
	memset(&item_to_send,0x00, sizeof(struct iec_item));
		
	item_to_send.iec_obj.ioa = obj_p->index;

	//TODO on 13-06-2012: Map the object to the corresponding IOA

//	printf("ioa = %d\n", obj_p->index);

	//item_to_send.cause = cot;
		
	switch(obj_p->pointType)
	{
		case EventInterface::AI:
		{
			printf("AI\n");

			item_to_send.iec_type = M_ME_TE_1;

			//DNP time (DnpTime_t) is a six byte unsigned int representing the number of milli-seconds
			// since midnight UTC Jan 1, 1970 (does not include leap seconds)
			
			epoch_to_cp56time2a(&time, obj_p->timestamp);
			item_to_send.iec_obj.o.type35.mv = obj_p->value;
			item_to_send.iec_obj.o.type35.time = time;

			if(obj_p->flag != QUALITY_GOOD)
				item_to_send.iec_obj.o.type35.iv = 1;
		}
		break;
		case EventInterface::BI:
		{
			printf("BI\n");

			item_to_send.iec_type = M_SP_TB_1;
			epoch_to_cp56time2a(&time, obj_p->timestamp);
			item_to_send.iec_obj.o.type30.sp = obj_p->value;
			item_to_send.iec_obj.o.type30.time = time;

			if(obj_p->flag != QUALITY_GOOD)
				item_to_send.iec_obj.o.type30.iv = 1;
		}
		break;
		case EventInterface::CI:
		{
			item_to_send.iec_type = M_IT_TB_1;
			epoch_to_cp56time2a(&time, obj_p->timestamp);
			item_to_send.iec_obj.o.type37.counter = obj_p->value;
			item_to_send.iec_obj.o.type37.time = time;
				
			if(obj_p->flag != QUALITY_GOOD)
				item_to_send.iec_obj.o.type37.iv = 1;

			printf("CI\n");
		}
		break;
		case EventInterface::AO:
		{
			item_to_send.iec_type = M_ME_TE_1;
			epoch_to_cp56time2a(&time, obj_p->timestamp);
			item_to_send.iec_obj.o.type35.mv = obj_p->value;
			item_to_send.iec_obj.o.type35.time = time;

			if(obj_p->flag != QUALITY_GOOD)
				item_to_send.iec_obj.o.type35.iv = 1;

			printf("AO\n");
		}
		break;
		case EventInterface::BO:
		{
			item_to_send.iec_type = M_SP_TB_1;
			epoch_to_cp56time2a(&time, obj_p->timestamp);
			item_to_send.iec_obj.o.type30.sp = obj_p->value;
			item_to_send.iec_obj.o.type30.time = time;

			if(obj_p->flag != QUALITY_GOOD)
				item_to_send.iec_obj.o.type30.iv = 1;

			printf("BO\n");
		}
		break;
		case EventInterface::NONE:
		{
			printf("NONE\n");
		}
		break;
		case EventInterface::ST:
		{
			printf("ST\n");
		}
		break;
		case EventInterface::AP_AB_ST: // app abnormal stat
		{
			printf("AP_AB_ST\n");
		}
		break;
		case EventInterface::AP_NM_ST: // app normal stat
		{
			printf("AP_NM_ST\n");
		}
		break;
		case EventInterface::DL_AB_ST: // datalink abnormal stat
		{
			printf("DL_AB_ST\n");
		}
		break;
		case EventInterface::DL_NM_ST: // datalink normal stat
		{
			printf("DL_NM_ST\n");
		}
		break;
		case EventInterface::SA_AB_ST: // secure auth abnormal stat
		{
			printf("SA_AB_ST\n");
		}
		break;
		case EventInterface::SA_NM_ST: // secure auth normal stat
		{
			printf("SA_NM_ST\n");
		}
		break;
		case EventInterface::EP_AB_ST: // end point abnormal stat
		{
			printf("EP_AB_ST\n");
		}
		break;
		case EventInterface::EP_NM_ST: // end point normal stat
		{
			printf("EP_NM_ST\n");
		}
		break;
		case EventInterface::NUM_POINT_TYPES:
		{
			printf("NUM_POINT_TYPES\n");
		}
		break;
		default:
			printf("UNSUPPORTED TYPE\n");
		break;
	}

	//IT_COMMENT6("at time: %d_%d_%d_%d_%d_%d", time.hour, time.min, time.msec, time.mday, time.month, time.year);

	item_to_send.msg_id = n_msg_sent;
	item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));

	//unsigned char buf[sizeof(struct iec_item)];
	//int len = sizeof(struct iec_item);
	//memcpy(buf, &item_to_send, len);
	//	for(j = 0;j < len; j++)
	//	{
	//	  unsigned char c = *(buf + j);
		//fprintf(stderr,"tx ---> 0x%02x\n", c);
		//fflush(stderr);
		//IT_COMMENT1("tx ---> 0x%02x\n", c);
	//	}

	Sleep(10); //Without delay there is missing of messages in the loading

	//Send in monitor direction
	fprintf(stderr,"Sending message %u th\n", n_msg_sent);
	fflush(stderr);

	//prepare published data
	memset(global_instanceSend,0x00, sizeof(iec_item_type));
	
	global_instanceSend->iec_type = item_to_send.iec_type;
	memcpy(&(global_instanceSend->iec_obj), &(item_to_send.iec_obj), sizeof(struct iec_object));
	global_instanceSend->cause = item_to_send.cause;
	global_instanceSend->msg_id = item_to_send.msg_id;
	global_instanceSend->ioa_control_center = item_to_send.ioa_control_center;
	global_instanceSend->casdu = item_to_send.casdu;
	global_instanceSend->is_neg = item_to_send.is_neg;
	global_instanceSend->checksum = item_to_send.checksum;

	ORTEPublicationSend(global_publisher);

	n_msg_sent++;
}

#include <time.h>
#include <sys/timeb.h>

void Factory::epoch_to_cp56time2a(cp56time2a *time, signed __int64 epoch_in_millisec)
{
	struct tm	*ptm;
	int ms = (int)(epoch_in_millisec%1000);
	time_t seconds;
	
	memset(time, 0x00,sizeof(cp56time2a));
	seconds = (long)(epoch_in_millisec/1000);
	ptm = localtime(&seconds);
		
    if(ptm)
	{
		time->hour = ptm->tm_hour;					//<0.23>
		time->min = ptm->tm_min;					//<0..59>
		time->msec = ptm->tm_sec*1000 + ms; //<0.. 59999>
		time->mday = ptm->tm_mday; //<1..31>
		time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
		time->month = ptm->tm_mon + 1; //<1..12>
		time->year = ptm->tm_year - 100; //<0.99>
		time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
		time->su = (u_char)ptm->tm_isdst; //<0..1> SUmmer time: <0> is standard time, <1> is summer time
	}

    return;
}

