/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2014 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "iec104types.h"
#include "iec_item.h"
#include "clear_crc_eight.h"
#include "opc_ua_db.h"
#include "client_api.h"
#include "opc_ua_imp.h"
#include "stdlib.h"
#include "time64.h"

#define MAX_KEYLEN 256
#define MAX_COMMAND_SEND_TIME 60

#include <sys/stat.h>

extern int gl_timeout_connection_with_parent;

/////////////////////////////////////Middleware///////////////////////////////////////////
Boolean  quite = ORTE_FALSE;
int	regfail=0;

//event system
void onRegFail(void *param) 
{
  printf("registration to a manager failed\n");
  regfail = 1;
}

void rebuild_iec_item_message(struct iec_item *item2, iec_item_type *item1)
{
	unsigned char checksum;

	///////////////Rebuild struct iec_item//////////////////////////////////
	item2->iec_type = item1->iec_type;
	memcpy(&(item2->iec_obj), &(item1->iec_obj), sizeof(struct iec_object));
	item2->cause = item1->cause;
	item2->msg_id = item1->msg_id;
	item2->ioa_control_center = item1->ioa_control_center;
	item2->casdu = item1->casdu;
	item2->is_neg = item1->is_neg;
	item2->checksum = item1->checksum;
	///////and check the 1 byte checksum////////////////////////////////////
	checksum = clearCrc((unsigned char *)item2, sizeof(struct iec_item));

//	fprintf(stderr,"new checksum = %u\n", checksum);

	//if checksum is 0 then there are no errors
	if(checksum != 0)
	{
		//log error message
		ExitProcess(0);
	}

	/*
	fprintf(stderr,"iec_type = %u\n", item2->iec_type);
	fprintf(stderr,"iec_obj = %x\n", item2->iec_obj);
	fprintf(stderr,"cause = %u\n", item2->cause);
	fprintf(stderr,"msg_id =%u\n", item2->msg_id);
	fprintf(stderr,"ioa_control_center = %u\n", item2->ioa_control_center);
	fprintf(stderr,"casdu =%u\n", item2->casdu);
	fprintf(stderr,"is_neg = %u\n", item2->is_neg);
	fprintf(stderr,"checksum = %u\n", item2->checksum);
	*/
}

void recvCallBack(const ORTERecvInfo *info,void *vinstance, void *recvCallBackParam) 
{
	opcua_imp * cl = (opcua_imp*)recvCallBackParam;
	iec_item_type *item1 = (iec_item_type*)vinstance;

	switch (info->status) 
	{
		case NEW_DATA:
		{
		  if(!quite)
		  {
			  struct iec_item item2;
			  rebuild_iec_item_message(&item2, item1);

			  if(cl->is_connected)
			  {
				cl->check_for_commands(&item2);
			  }
		  }
		}
		break;
		case DEADLINE:
		{
			//printf("deadline occurred\n");
		}
		break;
	}
}
////////////////////////////////Middleware/////////////////////////////////////


//   
//  Class constructor.   
//   
opcua_imp::opcua_imp(char* server_url, char* line_number, int polling_time):
fExit(false),pollingTime(polling_time), general_interrogation(true), is_connected(false)
{   
	lineNumber = atoi(line_number);
	strcpy(serverURL, server_url);
	
	/////////////////////Middleware/////////////////////////////////////////////////////////////////
	int32_t                 strength = 1;
	NtpTime                 persistence, deadline, minimumSeparation, delay;
	IPAddress				smIPAddress = IPADDRESS_INVALID;
	ORTEDomainProp          dp; 
	ORTEDomainAppEvents     events;

	publisher = NULL;
	subscriber = NULL;

	ORTEInit();
	ORTEDomainPropDefaultGet(&dp);
	NTPTIME_BUILD(minimumSeparation,0); 
	NTPTIME_BUILD(delay,1); //1s

	//initiate event system
	ORTEDomainInitEvents(&events);

	events.onRegFail = onRegFail;

	//Create application     
	domain = ORTEDomainAppCreate(ORTE_DEFAULT_DOMAIN,&dp,&events,ORTE_FALSE);

	iec_item_type_type_register(domain);

	//Create publisher
	NTPTIME_BUILD(persistence,5);

	char fifo_monitor_name[150];
	strcpy(fifo_monitor_name,"fifo_monitor_direction");
	strcat(fifo_monitor_name, line_number);
	strcat(fifo_monitor_name, "opcua");

	publisher = ORTEPublicationCreate(
	domain,
	fifo_monitor_name,
	"iec_item_type",
	&instanceSend,
	&persistence,
	strength,
	NULL,
	NULL,
	NULL);

	//if(publisher == NULL){} //check this error
	
	char fifo_control_name[150];
	strcpy(fifo_control_name,"fifo_control_direction");
	strcat(fifo_control_name, line_number);
	strcat(fifo_control_name, "opcua");

	//Create subscriber
	NTPTIME_BUILD(deadline,3);

	subscriber = ORTESubscriptionCreate(
	domain,
	IMMEDIATE,
	BEST_EFFORTS,
	fifo_control_name,
	"iec_item_type",
	&instanceRecv,
	&deadline,
	&minimumSeparation,
	recvCallBack,
	this,
	smIPAddress);

	//if(subscriber == NULL){} //check this error
	///////////////////////////////////Middleware//////////////////////////////////////////////////
}   
//   
//  Class destructor.   
//   
opcua_imp::~opcua_imp()  
{   
    // free resources   
	fExit = 1;
	///////////////////////////////////Middleware//////////////////////////////////////////////////
	ORTEDomainAppDestroy(domain);
    domain = NULL;
	////////////////////////////////////Middleware//////////////////////////////////////////////////
    return;   
}   

static u_int n_msg_sent = 0;

int opcua_imp::PollServer(void)
{
	IT_IT("opcua_imp::PollServer");
	
	int rc = 0;

	////////////General interrogation condition//////////////
	general_interrogation = true;
	loops = 0;
	//////////////////////////////////////////////////////////
  
	while(true) //the polling loop
	{	
		if(is_connected)
		{
			rc = PollItems();

			loops++;

			if(loops == 4)
			{
				general_interrogation = false;
			}
		
			if(rc)
			{ 
				fprintf(stderr,"opc ua client on line %d exiting...., due to lack of connection with server\n", lineNumber);
				fflush(stderr);

				IT_COMMENT("opcua_imp exiting...., due to lack of connection with server");
				
				//Send LOST message to parent (monitor.exe)
				struct iec_item item_to_send;
				struct cp56time2a actual_time;
				get_utc_host_time(&actual_time);

				memset(&item_to_send,0x00, sizeof(struct iec_item));

				item_to_send.iec_obj.ioa = 0;

				item_to_send.cause = 0x03;
				item_to_send.iec_type = C_LO_ST_1;
				item_to_send.iec_obj.o.type30.sp = 0;
				item_to_send.iec_obj.o.type30.time = actual_time;
				item_to_send.iec_obj.o.type30.iv = 0;
				item_to_send.msg_id = n_msg_sent;
				item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_item));

				//Send in monitor direction
				////////Middleware/////////////
				//publishing data
				//queue_monitor_dir->put(&item_to_send, sizeof(struct iec_item));
				////////Middleware/////////////

				memset(&instanceSend,0x00, sizeof(iec_item_type));

				instanceSend.iec_type = item_to_send.iec_type;
				memcpy(&(instanceSend.iec_obj), &(item_to_send.iec_obj), sizeof(struct iec_object));
				instanceSend.cause = item_to_send.cause;
				instanceSend.msg_id = item_to_send.msg_id;
				instanceSend.ioa_control_center = item_to_send.ioa_control_center;
				instanceSend.casdu = item_to_send.casdu;
				instanceSend.is_neg = item_to_send.is_neg;
				instanceSend.checksum = item_to_send.checksum;

				ORTEPublicationSend(publisher);
				
				n_msg_sent++;

				Sleep(100);
			
				break; //this terminate the loop and the program

				is_connected = false;
			}
		}

		if(fExit)
		{
			IT_COMMENT("Terminate opc ua loop!");
			break;
		}

		#define USE_KEEP_ALIVE_WATCH_DOG

		#ifdef USE_KEEP_ALIVE_WATCH_DOG
		gl_timeout_connection_with_parent++;

		if(gl_timeout_connection_with_parent > 1000*60/pollingTime)
		{
			break; //exit loop for timeout of connection with parent
		}
		#endif
				
		::Sleep(pollingTime);
	}
	
	IT_EXIT;
	return 0;
}

int opcua_imp::Start(void)
{
	IT_IT("opcua_imp::Start");
	
	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA OPC UA client Start\n");
	LogMessage(NULL, show_msg);

	if(fExit == 1)
	{
		return(1); //error
	}
	
	if(AddItems())
	{
		return(1); //error
	}

	OpcUa_StatusCode uStatus = OpcUa_Good;

	Session_Initialize(&session);

	uStatus = Client_Initialize();
	OpcUa_GotoErrorIfBad(uStatus);

	printf("1\n");

	/* need to fetch the metadata from the server using an insecure channel */
	uStatus = OpcUa_Channel_Create(&session.Channel, OpcUa_Channel_SerializerType_Binary);
	OpcUa_GotoErrorIfBad(uStatus);

	printf("2\n");

	uStatus = Client_Connect(&session, OpcUa_False, serverURL);
	OpcUa_GotoErrorIfBad(uStatus);

	printf("3\n");

	uStatus = Client_GetEndpoints(&session, serverURL);
	OpcUa_GotoErrorIfBad(uStatus);

	printf("4\n");

	OpcUa_Channel_Disconnect(session.Channel);
	OpcUa_Channel_Delete(&session.Channel);

	printf("5\n");

	/* now need to connect to server using a secure channel with any access token */
	uStatus = OpcUa_Channel_Create(&session.Channel, OpcUa_Channel_SerializerType_Binary);
	OpcUa_GotoErrorIfBad(uStatus);

	printf("6\n");

	uStatus = Client_Connect(&session, OpcUa_True, serverURL);
	OpcUa_GotoErrorIfBad(uStatus);

	printf("7\n");

	/* create a normal UA session and activate it */
	uStatus = Client_CreateSession(&session);
	OpcUa_GotoErrorIfBad(uStatus);

	printf("8\n");
	
	uStatus = Client_ActivateSession(&session);
	OpcUa_GotoErrorIfBad(uStatus);

	printf("9\n");

	is_connected = true;

	printf("Server is connected\n");

Error:
/*
	OpcUa_Trace(OPCUA_TRACE_LEVEL_ERROR, "**** Error ****\n");

	if (session.Channel != OpcUa_Null)
	{
		OpcUa_Channel_Delete(&session.Channel);

		printf("14\n");
	}

	Session_Clear(&session);
	Client_Cleanup();

	printf("15\n");
*/
	IT_EXIT;
	return uStatus;
}

int opcua_imp::Stop()
{
	IT_IT("opcua_imp::Stop");

	fprintf(stderr,"Entering Stop()\n");
	fflush(stderr);
	
	// terminate server and it will clean up itself

	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA OPC UA client End\n");
	LogMessage(NULL, show_msg);

	IT_EXIT;
	return 1;
}

struct log_message{

	int ioa;
	char message[150];
};

void opcua_imp::LogMessage(int* error, const char* name)
{
	//TODO: send message to monitor.exe as a single point

	/*
	struct iec_item item_to_send;
	struct cp56time2a actual_time;
	get_utc_host_time(&actual_time);

	memset(&item_to_send,0x00, sizeof(struct iec_item));

	//item_to_send.iec_obj.ioa =  Find ioa given the message in a vector of log_message

	item_to_send.cause = 0x03;
	item_to_send.iec_type = M_SP_TB_1;
	item_to_send.iec_obj.o.type30.sp = 0;
	item_to_send.iec_obj.o.type30.time = actual_time;
	item_to_send.iec_obj.o.type30.iv = 0;
	*/
}

uint64_t getTimeInMs_from_filetime(OpcUa_DateTime* timestamp)
{
	uint64_t now;
	FILETIME ft;

	ft.dwHighDateTime = timestamp->dwHighDateTime;
	ft.dwLowDateTime = timestamp->dwLowDateTime;

	static const uint64_t DIFF_TO_UNIXTIME = 11644473600000i64;

	now = (LONGLONG)ft.dwLowDateTime + ((LONGLONG)(ft.dwHighDateTime) << 32i64);

	return (now / 10000i64) - DIFF_TO_UNIXTIME;
}

void opcua_imp::get_opc_ua_message_time(struct cp56time2a* time, OpcUa_DateTime* timestamp)
{

	struct tm	*ptm;
	int64_t epoch_in_ms;
	int64_t seconds;

	epoch_in_ms = getTimeInMs_from_filetime(timestamp);

	seconds = epoch_in_ms/1000;

	ptm = localtime64((int64_t*)(&seconds));
		
	time->hour = ptm->tm_hour;					//<0.23>
	time->min = ptm->tm_min;					//<0..59>
	time->msec = ptm->tm_sec*1000 + (unsigned short)(epoch_in_ms%1000); //<0.. 59999>
	time->mday = ptm->tm_mday; //<1..31>
	time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
	time->month = ptm->tm_mon + 1; //<1..12>
	time->year = ptm->tm_year - 100; //<0.99>
	time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
	time->su = ptm->tm_isdst; //<0..1> SUmmer time: <0> is standard time, <1> is summer time

    return;
}

uint64_t getTimeInMs()
{
	FILETIME ft;
	uint64_t now;

	static const uint64_t DIFF_TO_UNIXTIME = 11644473600000i64;

	GetSystemTimeAsFileTime(&ft);

	now = (LONGLONG)ft.dwLowDateTime + ((LONGLONG)(ft.dwHighDateTime) << 32i64);

	return (now / 10000i64) - DIFF_TO_UNIXTIME;
}

void opcua_imp::get_local_host_time(struct cp56time2a* time)
{

	struct tm	*ptm;
	int64_t epoch_in_ms;
	int64_t seconds;

	epoch_in_ms = getTimeInMs();

	seconds = epoch_in_ms/1000;

	ptm = localtime64((int64_t*)(&seconds));
		
	time->hour = ptm->tm_hour;					//<0.23>
	time->min = ptm->tm_min;					//<0..59>
	time->msec = ptm->tm_sec*1000 + (unsigned short)(epoch_in_ms%1000); //<0.. 59999>
	time->mday = ptm->tm_mday; //<1..31>
	time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
	time->month = ptm->tm_mon + 1; //<1..12>
	time->year = ptm->tm_year - 100; //<0.99>
	time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
	time->su = ptm->tm_isdst; //<0..1> SUmmer time: <0> is standard time, <1> is summer time

    return;
}

void opcua_imp::get_utc_host_time(struct cp56time2a* time)
{
	struct tm	*ptm;
	int64_t epoch_in_ms;
	int64_t seconds;

	IT_IT("get_utc_host_time");

	epoch_in_ms = getTimeInMs();

	seconds = epoch_in_ms/1000;

	ptm = gmtime64((int64_t*)(&seconds));
		
	time->hour = ptm->tm_hour;					//<0..23>
	time->min = ptm->tm_min;					//<0..59>
	time->msec = ptm->tm_sec*1000 + (unsigned short)(epoch_in_ms%1000); //<0..59999>
	time->mday = ptm->tm_mday; //<1..31>
	time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
	time->month = ptm->tm_mon + 1; //<1..12>
	time->year = ptm->tm_year - 100; //<0.99>
	time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
	time->su = ptm->tm_isdst; //<0..1> SUmmer time: <0> is standard time, <1> is summer time

	IT_EXIT;
    return;
}

int64_t opcua_imp::epoch_from_cp56time2a(const struct cp56time2a* time)
{
	struct tm	t;
	int64_t epoch = 0;
	int ms;
	
	memset(&t, 0x00, sizeof(struct tm));
	
	t.tm_hour = time->hour;
	t.tm_min = time->min;
	t.tm_sec = time->msec/1000;
	ms = time->msec%1000; //not used
	t.tm_mday = time->mday;
	t.tm_mon = time->month - 1;	  //from <1..12> to	<0..11>				
	t.tm_year = time->year + 100; //from <0..99> to <years from 1900>
	t.tm_isdst = time->su;
	
	epoch = mktime64(&t);

	if((epoch == -1) || (time->iv == 1))
	{
		epoch = 0;
	}

	return epoch;
}

#define ABS(x) ((x) >= 0 ? (x) : -(x))

//Retun 1 on error
int opcua_imp::PollItems(void)
{
	IT_IT("opcua_imp::PollItems");

	struct iec_item item_to_send;
	struct cp56time2a message_time;
		
    bool send_item = true;

	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_DataValue* value = OpcUa_Null;
	char node_id[50];
	int namespace_index;

	for(int rowNumber = 0; rowNumber < db_n_rows; rowNumber++)
	{
		value = OpcUa_Null;
		
		memset(&item_to_send,0x00, sizeof(struct iec_item));
			
		strcpy(node_id, Config_db[rowNumber].nodeid);
		namespace_index = Config_db[rowNumber].namespace_index;
				
		uStatus = Client_ReadNode(&session, node_id, namespace_index, &value);

		if(OpcUa_IsBad(uStatus))
		{
			return 1;
		}

		if(value != OpcUa_Null)
		{
			switch(value->Value.Datatype)
			{
				case OpcUaType_Int16:
				{
					item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;
					item_to_send.cause = 0x03;
					item_to_send.iec_type = M_ME_TE_1;
										
					get_opc_ua_message_time(&message_time, &(value->SourceTimestamp));

					item_to_send.iec_obj.o.type35.mv = value->Value.Value.Int16;
					item_to_send.iec_obj.o.type35.time = message_time;
					item_to_send.iec_obj.o.type35.iv = 0;

					fprintf(stderr, "%d\n", value->Value.Value.Int16);
					fflush(stderr);
				}
				break;
				case OpcUaType_Int32:
				{
					item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;
					item_to_send.cause = 0x03;
					item_to_send.iec_type = M_IT_TB_1;
										
					get_opc_ua_message_time(&message_time, &(value->SourceTimestamp));

					item_to_send.iec_obj.o.type37.counter = value->Value.Value.Int32;
					item_to_send.iec_obj.o.type37.time = message_time;
					item_to_send.iec_obj.o.type37.iv = 0;

					fprintf(stderr, "%d\n", value->Value.Value.Int32);
					fflush(stderr);
				}
				break;
				case OpcUaType_UInt16:
				{
					item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;
					item_to_send.cause = 0x03;
					item_to_send.iec_type = M_ME_TQ_1;
										
					get_opc_ua_message_time(&message_time, &(value->SourceTimestamp));

					item_to_send.iec_obj.o.type153.mv = value->Value.Value.UInt16;
					item_to_send.iec_obj.o.type153.time = message_time;
					item_to_send.iec_obj.o.type153.iv = 0;

					fprintf(stderr, "%d\n", value->Value.Value.UInt16);
					fflush(stderr);
				}
				break;
				case OpcUaType_UInt32:
				{
					item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;
					item_to_send.cause = 0x03;
					item_to_send.iec_type = M_ME_TO_1;
										
					get_opc_ua_message_time(&message_time, &(value->SourceTimestamp));

					item_to_send.iec_obj.o.type151.mv = value->Value.Value.UInt32;
					item_to_send.iec_obj.o.type151.time = message_time;
					item_to_send.iec_obj.o.type151.iv = 0;

					fprintf(stderr, "%d\n", value->Value.Value.UInt32);
					fflush(stderr);
				}
				break;
				case OpcUaType_Float:
				{
					item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;
					item_to_send.cause = 0x03;
					item_to_send.iec_type = M_ME_TF_1;
										
					get_opc_ua_message_time(&message_time, &(value->SourceTimestamp));

					item_to_send.iec_obj.o.type36.mv = value->Value.Value.Float;
					item_to_send.iec_obj.o.type36.time = message_time;
					item_to_send.iec_obj.o.type36.iv = 0;

					fprintf(stderr, "%f\n", value->Value.Value.Float);
					fflush(stderr);
				}
				break;
				case OpcUaType_Double:
				{
					item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;
					item_to_send.cause = 0x03;
					item_to_send.iec_type = M_ME_TN_1;
										
					get_opc_ua_message_time(&message_time, &(value->SourceTimestamp));

					item_to_send.iec_obj.o.type150.mv = value->Value.Value.Float;
					item_to_send.iec_obj.o.type150.time = message_time;
					item_to_send.iec_obj.o.type150.iv = 0;

					fprintf(stderr, "%lf\n", value->Value.Value.Double);
					fflush(stderr);
				}
				break;
				case OpcUaType_Boolean:
				{
					item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;
					item_to_send.cause = 0x03;
					item_to_send.iec_type = M_SP_TB_1;
										
					get_opc_ua_message_time(&message_time, &(value->SourceTimestamp));

					item_to_send.iec_obj.o.type30.sp = value->Value.Value.Boolean;
					item_to_send.iec_obj.o.type30.time = message_time;
					item_to_send.iec_obj.o.type30.iv = 0;
					
					fprintf(stderr, "%x\n", value->Value.Value.Boolean);
					fflush(stderr);
				}
				break;
				default:
				{
					fprintf(stderr, "Node ID %s type is not supported\n", node_id);
					fflush(stderr);
				}
				break;
			}
		}

		if(send_item || general_interrogation)
		{
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

			//Send in monitor direction
			fprintf(stderr,"Sending message %u th\n", n_msg_sent);
			fflush(stderr);
			IT_COMMENT1("Sending message %u th\n", n_msg_sent);

			memset(&instanceSend,0x00, sizeof(iec_item_type));

			instanceSend.iec_type = item_to_send.iec_type;
			memcpy(&(instanceSend.iec_obj), &(item_to_send.iec_obj), sizeof(struct iec_object));
			instanceSend.cause = item_to_send.cause;
			instanceSend.msg_id = item_to_send.msg_id;
			instanceSend.ioa_control_center = item_to_send.ioa_control_center;
			instanceSend.casdu = item_to_send.casdu;
			instanceSend.is_neg = item_to_send.is_neg;
			instanceSend.checksum = item_to_send.checksum;

			ORTEPublicationSend(publisher);

			n_msg_sent++;
		}
	}

	IT_EXIT;
	return 0;

}

#define _EPSILON_ ((double)(2.220446E-16))

#define DO_NOT_RESCALE

short opcua_imp::rescale_value(double V, double Vmin, double Vmax, int* error)
{
	#ifdef DO_SCALE
	double Amin;
	double Amax;
	double r;
	//double V; //Observed value in ingegneristic unit
	double A = 0.0; //Calculate scaled value between Amin = -32768 and Amax = 32767
	double denomin;

	IT_IT("rescale_value");

	*error = 0;

	Amin = -32768.0;
	Amax = 32767.0;

	if(((V - Vmin) > 0.0) && ((V - Vmax) < 0.0))
	{
		denomin = Vmax - Vmin;

		if(denomin > 0.0)
		{
			r = (Amax - Amin)/denomin;
			A = r*(V - Vmin) + Amin;
		}
		else
		{
			*error = 1;
		}
	}
	else if(((V - Vmin) < 0.0))
	{
		A = Amin;
	}
	else if(!fcmp(V, Vmin, _EPSILON_))
	{
		A = Amin;
	}
	else if(((V - Vmax) > 0.0))
	{
		A = Amax;
	}
	else if(!fcmp(V, Vmax, _EPSILON_))
	{
		A = Amax;
	}
	
	IT_COMMENT4("V = %lf, Vmin = %lf, Vmax = %lf, A = %lf", V, Vmin, Vmax, A);

	IT_EXIT;

	return (short)A;

	#endif

	#ifdef DO_NOT_RESCALE

	return (short)V;

	#endif //DO_NOT_RESCALE
}

double opcua_imp::rescale_value_inv(double A, double Vmin, double Vmax, int* error)
{
	#ifdef DO_SCALE
	double Amin;
	double Amax;
	double r;
	double V; //Calculated value in ingegneristic unit
	//double A = 0.0; //Given a scaled value between Amin = -32768 and Amax = 32767
	double denomin;

	IT_IT("rescale_value_inv");

	*error = 0;

	Amin = -32768.0;
	Amax = 32767.0;

	denomin = Vmax - Vmin;

	if(denomin > 0.0)
	{
		r = (Amax - Amin)/denomin;
		V = Vmin + (A - Amin)/r;
	}
	else
	{
		*error = 1;
	}
		
	IT_COMMENT4("V = %lf, Vmin = %lf, Vmax = %lf, A = %lf", V, Vmin, Vmax, A);

	IT_EXIT;

	return V;

	#endif

	#ifdef DO_NOT_RESCALE

	return A;

	#endif //DO_NOT_RESCALE
}


void opcua_imp::check_for_commands(struct iec_item *queued_item)
{
	if(!fExit)
	{ 
		fprintf(stderr,"Receiving %d th message \n", queued_item->msg_id);
		fflush(stderr);
					
		/////////////////////write command///////////////////////////////////////////////////////////
		if(queued_item->iec_type == C_SC_TA_1
			|| queued_item->iec_type == C_DC_TA_1
			|| queued_item->iec_type == C_SE_TA_1
			|| queued_item->iec_type == C_SE_TB_1
			|| queued_item->iec_type == C_SE_TC_1
			|| queued_item->iec_type == C_BO_TA_1
			|| queued_item->iec_type == C_SC_NA_1
			|| queued_item->iec_type == C_DC_NA_1
			|| queued_item->iec_type == C_SE_NA_1 
			|| queued_item->iec_type == C_SE_NB_1
			|| queued_item->iec_type == C_SE_NC_1
			|| queued_item->iec_type == C_BO_NA_1)
		{
			Sleep(100); //Delay between one command and the next one

			/////////Here we execute the QUERY:////////////////////////////////////////// /////////////////////////////
			// select from Config_db table the rowNumber where ioa is equal to ioa of packet arriving (command) from monitor.exe
			///////////////////////////////////////////////////////////////////////////////////////
			int found = 0;
			DWORD rowNumber = -1;

			for(int dw = 0; dw < db_n_rows; dw++) 
			{ 
				if(queued_item->iec_obj.ioa == Config_db[dw].ioa_control_center)
				{
					found = 1;
					rowNumber = dw;
					break;
				}
			}

			if(found == 0)
			{
				fprintf(stderr,"Error: Command with IOA %d not found in I/O list\n", queued_item->iec_obj.ioa);
				fflush(stderr);
				fprintf(stderr,"Command NOT executed\n");
				fflush(stderr);
				return;
			}
						
			//Receive a write command
								
			fprintf(stderr,"Receiving command for ioa %d\n", queued_item->iec_obj.ioa);
			fflush(stderr);
			
			//Check the life time of the command/////////////////////////////////////////////////////////////////
			//If life time > MAX_COMMAND_SEND_TIME seconds => DO NOT execute the command

			int64_t command_generation_time_in_seconds = 0;

			switch(queued_item->iec_type)
			{
				case C_SC_TA_1:
				case C_SC_NA_1:
				{
					//time contains the UTC time
					command_generation_time_in_seconds = epoch_from_cp56time2a(&(queued_item->iec_obj.o.type58.time));

					fprintf(stderr,"Command generation at UTC time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					queued_item->iec_obj.o.type58.time.hour,
					queued_item->iec_obj.o.type58.time.min,
					queued_item->iec_obj.o.type58.time.msec/1000,
					queued_item->iec_obj.o.type58.time.msec%1000,
					queued_item->iec_obj.o.type58.time.mday,
					queued_item->iec_obj.o.type58.time.month,
					queued_item->iec_obj.o.type58.time.year,
					queued_item->iec_obj.o.type58.time.iv,
					queued_item->iec_obj.o.type58.time.su);
					fflush(stderr);
				}
				break;
				case C_DC_TA_1:
				case C_DC_NA_1:
				{
					//time contains the UTC time
					command_generation_time_in_seconds = epoch_from_cp56time2a(&(queued_item->iec_obj.o.type59.time));

					fprintf(stderr,"Command generation at UTC time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					queued_item->iec_obj.o.type59.time.hour,
					queued_item->iec_obj.o.type59.time.min,
					queued_item->iec_obj.o.type59.time.msec/1000,
					queued_item->iec_obj.o.type59.time.msec%1000,
					queued_item->iec_obj.o.type59.time.mday,
					queued_item->iec_obj.o.type59.time.month,
					queued_item->iec_obj.o.type59.time.year,
					queued_item->iec_obj.o.type59.time.iv,
					queued_item->iec_obj.o.type59.time.su);
					fflush(stderr);
				}
				break;
				case C_SE_TA_1:
				case C_SE_NA_1:
				{
					//time contains the UTC time
					command_generation_time_in_seconds = epoch_from_cp56time2a(&(queued_item->iec_obj.o.type61.time));

					fprintf(stderr,"Command generation at UTC time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					queued_item->iec_obj.o.type61.time.hour,
					queued_item->iec_obj.o.type61.time.min,
					queued_item->iec_obj.o.type61.time.msec/1000,
					queued_item->iec_obj.o.type61.time.msec%1000,
					queued_item->iec_obj.o.type61.time.mday,
					queued_item->iec_obj.o.type61.time.month,
					queued_item->iec_obj.o.type61.time.year,
					queued_item->iec_obj.o.type61.time.iv,
					queued_item->iec_obj.o.type61.time.su);
					fflush(stderr);
				}
				break;
				case C_SE_TB_1:
				case C_SE_NB_1:
				{
					//time contains the UTC time
					command_generation_time_in_seconds = epoch_from_cp56time2a(&(queued_item->iec_obj.o.type62.time));

					fprintf(stderr,"Command generation at UTC time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					queued_item->iec_obj.o.type62.time.hour,
					queued_item->iec_obj.o.type62.time.min,
					queued_item->iec_obj.o.type62.time.msec/1000,
					queued_item->iec_obj.o.type62.time.msec%1000,
					queued_item->iec_obj.o.type62.time.mday,
					queued_item->iec_obj.o.type62.time.month,
					queued_item->iec_obj.o.type62.time.year,
					queued_item->iec_obj.o.type62.time.iv,
					queued_item->iec_obj.o.type62.time.su);
					fflush(stderr);
				}
				break;
				case C_SE_TC_1:
				case C_SE_NC_1:
				{
					//time contains the UTC time
					command_generation_time_in_seconds = epoch_from_cp56time2a(&(queued_item->iec_obj.o.type63.time));

					fprintf(stderr,"Command generation at UTC time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					queued_item->iec_obj.o.type63.time.hour,
					queued_item->iec_obj.o.type63.time.min,
					queued_item->iec_obj.o.type63.time.msec/1000,
					queued_item->iec_obj.o.type63.time.msec%1000,
					queued_item->iec_obj.o.type63.time.mday,
					queued_item->iec_obj.o.type63.time.month,
					queued_item->iec_obj.o.type63.time.year,
					queued_item->iec_obj.o.type63.time.iv,
					queued_item->iec_obj.o.type63.time.su);
					fflush(stderr);
				}
				break;
				case C_BO_TA_1:
				case C_BO_NA_1:
				{
					//time contains the UTC time
					command_generation_time_in_seconds = epoch_from_cp56time2a(&(queued_item->iec_obj.o.type64.time));

					fprintf(stderr,"Command generation at UTC time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
					queued_item->iec_obj.o.type64.time.hour,
					queued_item->iec_obj.o.type64.time.min,
					queued_item->iec_obj.o.type64.time.msec/1000,
					queued_item->iec_obj.o.type64.time.msec%1000,
					queued_item->iec_obj.o.type64.time.mday,
					queued_item->iec_obj.o.type64.time.month,
					queued_item->iec_obj.o.type64.time.year,
					queued_item->iec_obj.o.type64.time.iv,
					queued_item->iec_obj.o.type64.time.su);
					fflush(stderr);
				}
				break;
				default:
				{
					//error
					//fprintf(stderr,"Error %d, %s\n",__LINE__, __FILE__);
					//fflush(stderr);

					char show_msg[200];
					sprintf(show_msg, "Error %d, %s\n",__LINE__, __FILE__);
					opcua_imp::LogMessage(0, show_msg);
				
					return;
				}
				break;
			}

			struct cp56time2a actual_time;
			get_utc_host_time(&actual_time);

			time_t command_execution_time_in_seconds = epoch_from_cp56time2a(&actual_time);

			fprintf(stderr,"Command execution UTC time: h:%i m:%i s:%i ms:%i %02i-%02i-%02i, iv %i, su %i\n",
			actual_time.hour,
			actual_time.min,
			actual_time.msec/1000,
			actual_time.msec%1000,
			actual_time.mday,
			actual_time.month,
			actual_time.year,
			actual_time.iv,
			actual_time.su);
			fflush(stderr);

			time_t delta = command_execution_time_in_seconds  - command_generation_time_in_seconds;

			fprintf(stderr,"Aged delta time= %d\n", delta);
			fflush(stderr);

			OpcUa_DataValue ValueWrite;

			if(delta < MAX_COMMAND_SEND_TIME && delta >= 0)
			{
				union {
					unsigned int v;
					float f;
				} cmd_val;

				switch(queued_item->iec_type)
				{
					case C_SC_TA_1:
					{
						cmd_val.v = queued_item->iec_obj.o.type58.scs;

						ValueWrite.Value.Datatype = OpcUaType_Int32;
						ValueWrite.Value.Value.Int32 = cmd_val.v;
					}
					break;
					case C_DC_TA_1:
					{
						cmd_val.f = (float)queued_item->iec_obj.o.type59.dcs;

						ValueWrite.Value.Datatype = OpcUaType_Float;
						ValueWrite.Value.Value.Float = cmd_val.f;
					}
					break;
					case C_SE_TA_1:
					{
						//double Vmin = Item[hClient - 1].min_measure;
						//double Vmax = Item[hClient - 1].max_measure;
						//double A = (double)queued_item->iec_obj.o.type61.sv;
						//int error = 0;

						//cmd_val = rescale_value_inv(A, Vmin, Vmax, &error);
						//if(error){ return;}
					}
					break;
					case C_SE_TB_1:
					{
						//double Vmin = Item[hClient - 1].min_measure;
						//double Vmax = Item[hClient - 1].max_measure;
						//double A = (double)queued_item->iec_obj.o.type62.sv;
						//int error = 0;

						//cmd_val = rescale_value_inv(A, Vmin, Vmax, &error);
						//if(error){ return;}
					}
					break;
					case C_SE_TC_1:
					{
						cmd_val.f = queued_item->iec_obj.o.type63.sv;

						ValueWrite.Value.Datatype = OpcUaType_Float;
						ValueWrite.Value.Value.Float = cmd_val.f;
					}
					break;
					case C_BO_TA_1:
					{
						memcpy(&(cmd_val.v), &(queued_item->iec_obj.o.type64.stcd), sizeof(struct iec_stcd));

						ValueWrite.Value.Datatype = OpcUaType_Int32;
						ValueWrite.Value.Value.Int32 = cmd_val.v;
					}
					break;
					case C_SC_NA_1:
					{
						cmd_val.f = (float)queued_item->iec_obj.o.type45.scs;

						ValueWrite.Value.Datatype = OpcUaType_Float;
						ValueWrite.Value.Value.Float = cmd_val.f;
					}
					break;
					case C_DC_NA_1:
					{
						cmd_val.f = (float)queued_item->iec_obj.o.type46.dcs;

						ValueWrite.Value.Datatype = OpcUaType_Float;
						ValueWrite.Value.Value.Float = cmd_val.f;
					}
					break;
					case C_SE_NA_1:
					{
						//double Vmin = Item[hClient - 1].min_measure;
						//double Vmax = Item[hClient - 1].max_measure;
						//double A = (double)queued_item->iec_obj.o.type48.sv;
						//int error = 0;

						//cmd_val = rescale_value_inv(A, Vmin, Vmax, &error);
						//if(error){ return;}
					}
					break;
					case C_SE_NB_1:
					{
						//double Vmin = Item[hClient - 1].min_measure;
						//double Vmax = Item[hClient - 1].max_measure;
						//double A = (double)queued_item->iec_obj.o.type49.sv;
						//int error = 0;

						//cmd_val = rescale_value_inv(A, Vmin, Vmax, &error);
						//if(error){ return;}
					}
					break;
					case C_SE_NC_1:
					{
						cmd_val.f = queued_item->iec_obj.o.type50.sv;

						ValueWrite.Value.Datatype = OpcUaType_Float;
						ValueWrite.Value.Value.Float = cmd_val.f;
					}
					break;
					case C_BO_NA_1:
					{
						memcpy(&(cmd_val.v), &(queued_item->iec_obj.o.type51.stcd), sizeof(struct iec_stcd));

						ValueWrite.Value.Datatype = OpcUaType_Int32;
						ValueWrite.Value.Value.Int32 = cmd_val.v;
					}
					break;
					default:
					{
						//error
						//fprintf(stderr,"Error %d, %s\n",__LINE__, __FILE__);
						//fflush(stderr);

						char show_msg[200];
						sprintf(show_msg, "Error %d, %s\n",__LINE__, __FILE__);
						opcua_imp::LogMessage(0, show_msg);
						
						return;
					}
					break;
				}
								
				char node_id[50];
				int namespace_index;

				strcpy(node_id, Config_db[rowNumber].nodeid);
				namespace_index = Config_db[rowNumber].namespace_index;

				Client_WriteNode(&session, node_id, namespace_index, &ValueWrite);
			}
		}
		else if(queued_item->iec_type == C_EX_IT_1)
		{
			//Receiving EXIT process command from monitor.exe
			//exit the thread, and stop the process
			fExit = true;
		}
		else if(queued_item->iec_type == C_IC_NA_1)
		{
			//Receiving general interrogation command from monitor.exe
			IT_COMMENT("Receiving general interrogation command from monitor.exe");
			fprintf(stderr,"Receiving general interrogation command from monitor.exe\n");
			fflush(stderr);

			////////////General interrogation condition//////////////
			general_interrogation = true;
			loops = 0;
			//////////////////////////////////////////////////////////
		}
	}

	return;
}



