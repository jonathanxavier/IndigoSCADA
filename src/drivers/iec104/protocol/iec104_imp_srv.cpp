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

//#include "stdint.h"
#include "clear_crc_eight.h"
#include "iec104_imp_srv.h"
#include "stdlib.h"
#include "queue.h"
#include "iec104.h"
#include "iec_item.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <semaphore.h>
#endif
#include <errno.h>
#ifdef WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#endif
#include "event.h"
#ifndef WIN32
#include <sys/queue.h>
#endif

#include <compat/sys/queue.h>
#include "iec_library.h"
#include "iec104_types.h"
#include "itrace.h"
#include <signal.h>
#include <time.h>
#include <process.h>
#include "iec_item.h"

#define MAX_COMMAND_SEND_TIME 60

#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

void set_fifo_monitor_dir_handle(fifo_h f_m_d);
void set_fifo_control_dir_handle(fifo_h f_c_d);

#ifdef __cplusplus
}
#endif

//commands
void control_dir_consumer(void* pParam)
{
	struct subs_args* arg = (struct subs_args*)pParam;
	iec104_imp* parent = (iec104_imp*)arg->parent;
	unsigned char buf[sizeof(struct iec_item)];
    int len, rc;
	const unsigned wait_limit_ms = 1;
	struct iec_item* p_item;
	//////////////////iec 104 part //////////////////
	struct iec_item item_to_send;
	/////////////////////////////////////////////////

	while(1)
	{
		if(parent->exit_threads)
		{
			break;
		}
	
		for(int i = 0; (len = fifo_get(parent->fifo_control_direction, (char*)buf, sizeof(struct iec_item), wait_limit_ms)) >= 0; i += 1)	
		{ 
			//Receive in control direction as iec 104 packet
			p_item = (struct iec_item*)buf;

			//for (int j = 0; j < len; j++) 
			//{ 
			//	unsigned char c = *((unsigned char*)buf + j);
				//printf("rx <--- 0x%02x-\n", c);

			//	IT_COMMENT1("rx <--- 0x%02x-\n", c);
			//}

			rc = clearCrc((unsigned char *)buf, sizeof(struct iec_item));

			if(rc != 0)
			{
				fprintf(stderr, "Error CRC8 = %d\n", rc);
				fflush(stderr);
				ExitProcess(0);
			}

			memcpy(&item_to_send, p_item, sizeof(struct iec_item));

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

			//Send in control direction as iec 104 packet
			fprintf(stderr,"Sending message %u th\n", parent->n_msg_sent_control_dir);
			fflush(stderr);

			//publishing data
			#ifdef USE_RIPC_MIDDLEWARE
			parent->queue_control_dir->put(&item_to_send, sizeof(struct iec_item));
			#endif

			//Send in monitor direction
			//prepare published data
			memset(&(parent->instanceSend),0x00, sizeof(iec_item_type));

			parent->instanceSend.iec_type = item_to_send.iec_type;
			memcpy(&(parent->instanceSend.iec_obj), &(item_to_send.iec_obj), sizeof(struct iec_object));
			parent->instanceSend.cause = item_to_send.cause;
			parent->instanceSend.msg_id = item_to_send.msg_id;
			parent->instanceSend.ioa_control_center = item_to_send.ioa_control_center;
			parent->instanceSend.casdu = item_to_send.casdu;
			parent->instanceSend.is_neg = item_to_send.is_neg;
			parent->instanceSend.checksum = item_to_send.checksum;

			ORTEPublicationSend(parent->publisher);
			
			parent->n_msg_sent_control_dir++;
		}
	}
}

#ifdef USE_RIPC_MIDDLEWARE
void monitoring_dir_consumer(void* pParam)
{
	struct subs_args* arg = (struct subs_args*)pParam;
	struct iec_item item;
	RIPCObject objDesc(&item, sizeof(struct iec_item));

	iec104_imp* parent = (iec104_imp*)arg->parent;

	while(1)
	{
		if(parent->exit_threads)
		{
			break;
		}

		parent->queue_monitor_dir->get(objDesc);

		fifo_put(parent->fifo_monitor_direction, (char*)&item, sizeof(struct iec_item));
	}
}
#endif

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

//	fprintf(stderr,"iec_type = %u\n", item2->iec_type);
//	fprintf(stderr,"iec_obj = %x\n", item2->iec_obj);
//	fprintf(stderr,"cause = %u\n", item2->cause);
//	fprintf(stderr,"msg_id =%u\n", item2->msg_id);
//	fprintf(stderr,"ioa_control_center = %u\n", item2->ioa_control_center);
//	fprintf(stderr,"casdu =%u\n", item2->casdu);
//	fprintf(stderr,"is_neg = %u\n", item2->is_neg);
//	fprintf(stderr,"checksum = %u\n", item2->checksum);
}

void recvCallBack(const ORTERecvInfo *info,void *vinstance, void *recvCallBackParam) 
{
	iec104_imp * cl = (iec104_imp*)recvCallBackParam;
	iec_item_type *item1 = (iec_item_type*)vinstance;

	switch (info->status) 
	{
		case NEW_DATA:
		{
		  if(!quite)
		  {
			  struct iec_item item2;
			  rebuild_iec_item_message(&item2, item1);
			  fifo_put(cl->fifo_monitor_direction, (char*)&item2, sizeof(struct iec_item));
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
iec104_imp::iec104_imp(struct iec104Context* my_ctx):
fExit(false), is_connected(false),
exit_threads(0),n_msg_sent_monitor_dir(0),n_msg_sent_control_dir(0)
{   
	strcpy(my_iec104_context.iec104ServerAddress, my_ctx->iec104ServerAddress);
	strcpy(my_iec104_context.iec104ServerPort, my_ctx->iec104ServerPort);
			
	/////////////////////Middleware/////////////////////////////////////////////////////////////////
	char fifo_monitor_name[150];
	char fifo_control_name[150];

	strcpy(fifo_monitor_name,"fifo_global_monitor_direction");
	strcpy(fifo_control_name,"fifo_global_control_direction");

	#ifdef USE_RIPC_MIDDLEWARE
	port = 6000;
	hostname = "localhost";

	factory1 = RIPCClientFactory::getInstance();
	factory2 = RIPCClientFactory::getInstance();

	session1 = factory1->create(hostname, port);
	session2 = factory2->create(hostname, port);
	queue_monitor_dir = session1->createQueue(fifo_monitor_name);
	queue_control_dir = session2->createQueue(fifo_control_name);
	#endif

	arg.parent = this;
	///////////////////////////////////Middleware//////////////////////////////////////////////////

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

	publisher = ORTEPublicationCreate(
	domain,
	fifo_control_name,
	"iec_item_type",
	&instanceSend,
	&persistence,
	strength,
	NULL,
	NULL,
	NULL);

	//if(publisher == NULL){} //check this error
			
	//Create subscriber
	NTPTIME_BUILD(deadline,3);

	subscriber = ORTESubscriptionCreate(
	domain,
	IMMEDIATE,
	BEST_EFFORTS,
	fifo_monitor_name,
	"iec_item_type",
	&instanceRecv,
	&deadline,
	&minimumSeparation,
	recvCallBack,
	this,
	smIPAddress);

	//if(subscriber == NULL){} //check this error
	///////////////////////////////////Middleware//////////////////////////////////////////////////

	#define MAX_FIFO_SIZE 65535
	fifo_monitor_direction = fifo_open("fifo_local_monitor_dir", MAX_FIFO_SIZE, iec_call_exit_handler);

	set_fifo_monitor_dir_handle(fifo_monitor_direction);
	
	unsigned long threadid;
	
	//CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(monitoring_dir_consumer), (void*)&arg, 0, &threadid);

	fifo_control_direction = fifo_open("fifo_local_control_dir", MAX_FIFO_SIZE, iec_call_exit_handler);

	set_fifo_control_dir_handle(fifo_control_direction);
		
	CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(control_dir_consumer), (void*)&arg, 0, &threadid);
}   
//   
//  Class destructor.   
//   
iec104_imp::~iec104_imp()  
{   
    // free resources   
	fExit = 1;
	#ifdef USE_RIPC_MIDDLEWARE
	////////Middleware/////////////
	queue_monitor_dir->close();
	queue_control_dir->close();
	session1->close();
	session2->close();
	delete session1;
	delete session2;
	////////Middleware/////////////
	#endif
	///////////////////////////////////Middleware//////////////////////////////////////////////////
	ORTEDomainAppDestroy(domain);
    domain = NULL;
	////////////////////////////////////Middleware//////////////////////////////////////////////////
    return;   
}   

int iec104_imp::Run(void)
{
	IT_IT("iec104_imp::PollServer");
	
	event_dispatch();
	
	IT_EXIT;
	return 0;
}

#define BACKLOG	1

int iec104_imp::Start(void)
{
	IT_IT("iec104_imp::Start");
	
	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA iec104 slave Start\n");
	LogMessage(NULL, show_msg);

	if(fExit == 1)
	{
		return(1); //error
	}

	struct sockaddr_in server_addr;
	int rc = 0;

	if(alloc_queues())
	{
		if(iec_protocol_log_stream)
			fclose(iec_protocol_log_stream);
		return 1;
	}

	if(initialize_win32_socket())
	{
		if(iec_protocol_log_stream)
			fclose(iec_protocol_log_stream);
		return 1;
	}

	event_init();
	
	default_hooks.disconnect_indication = disconnect_hook;
	default_hooks.connect_indication = connect_hook;
	default_hooks.data_indication = data_received_hook;
	default_hooks.activation_indication = activation_hook;
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	int n_iec104server_port = atoi(my_iec104_context.iec104ServerPort);
	server_addr.sin_port = htons(n_iec104server_port);
	server_addr.sin_addr.s_addr = inet_addr(my_iec104_context.iec104ServerAddress);
	
	iecsock_listen(&server_addr, BACKLOG);
	
	IT_EXIT;
    return(0);
}

int iec104_imp::Stop()
{
	IT_IT("iec104_imp::Stop");

	fprintf(stderr,"Entering Stop()\n");
	fflush(stderr);

	exit_threads = 1;
	Sleep(2000);

	fifo_close(fifo_monitor_direction);
	fifo_close(fifo_control_direction);

	if(iec_protocol_log_stream)
		fclose(iec_protocol_log_stream);
	
	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA iec104 master End\n");
	LogMessage(NULL, show_msg);

	IT_EXIT;
	return 1;
}

struct log_message{

	int ioa;
	char message[150];
};

void iec104_imp::LogMessage(int* error, const char* name)
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
/*
#include <time.h>
#include <sys/timeb.h>

void iec104_imp::get_local_host_time(struct cp56time2a* time)
{
	struct timeb tb;
	struct tm	*ptm;

    ftime (&tb);
	ptm = localtime(&tb.time);
		
	time->hour = ptm->tm_hour;					//<0.23>
	time->min = ptm->tm_min;					//<0..59>
	time->msec = ptm->tm_sec*1000 + tb.millitm; //<0.. 59999>
	time->mday = ptm->tm_mday; //<1..31>
	time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
	time->month = ptm->tm_mon + 1; //<1..12>
	time->year = ptm->tm_year - 100; //<0.99>
	time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
	time->su = (u_char)tb.dstflag; //<0..1> SUmmer time: <0> is standard time, <1> is summer time

    return;
}

void iec104_imp::get_utc_host_time(struct cp56time2a* time)
{
	struct timeb tb;
	struct tm	*ptm;
		
	IT_IT("get_utc_host_time");

    ftime (&tb);
	ptm = gmtime(&tb.time);
		
	time->hour = ptm->tm_hour;					//<0..23>
	time->min = ptm->tm_min;					//<0..59>
	time->msec = ptm->tm_sec*1000 + tb.millitm; //<0..59999>
	time->mday = ptm->tm_mday; //<1..31>
	time->wday = (ptm->tm_wday == 0) ? ptm->tm_wday + 7 : ptm->tm_wday; //<1..7>
	time->month = ptm->tm_mon + 1; //<1..12>
	time->year = ptm->tm_year - 100; //<0.99>
	time->iv = 0; //<0..1> Invalid: <0> is valid, <1> is invalid
	time->su = (u_char)tb.dstflag; //<0..1> SUmmer time: <0> is standard time, <1> is summer time

	IT_EXIT;
    return;
}

time_t iec104_imp::epoch_from_cp56time2a(const struct cp56time2a* time)
{
	struct tm	t;
	time_t epoch = 0;
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
	
	epoch = mktime(&t);

	if((epoch == -1) || (time->iv == 1))
	{
		epoch = 0;
	}

	return epoch;
}

#define _EPSILON_ ((double)(2.220446E-16))

#define DO_NOT_RESCALE

short iec104_imp::rescale_value(double V, double Vmin, double Vmax, int* error)
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

double iec104_imp::rescale_value_inv(double A, double Vmin, double Vmax, int* error)
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

void iec104_imp::check_for_commands(struct iec_item *queued_item)
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
			
			//Receive a write command
								
			fprintf(stderr,"Receiving command for ioa %d\n", queued_item->iec_obj.ioa);
			fflush(stderr);
			
			//Check the life time of the command/////////////////////////////////////////////////////////////////
			//If life time > MAX_COMMAND_SEND_TIME seconds => DO NOT execute the command

			time_t command_generation_time_in_seconds = 0;

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
					iec104_imp::LogMessage(0, show_msg);
				
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

			if(delta < MAX_COMMAND_SEND_TIME && delta >= 0)
			{
				struct iec_item item_to_send;

				memcpy(&item_to_send, queued_item, sizeof(struct iec_item));

				////////////////////////////////////////////////////////////////
				fifo_put(fifo_control_direction, (char*)&item_to_send, sizeof(struct iec_item));
				//////////////////////////////////////////////////////////////////////////////
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
			fprintf(stderr,"Receiving general interrogation command from monitor.exe\n");
			fflush(stderr);

			struct iec_item item_to_send;

			memcpy(&item_to_send, queued_item, sizeof(struct iec_item));

			////////////////////////////////////////////////////////////////
			fifo_put(fifo_control_direction, (char*)&item_to_send, sizeof(struct iec_item));
			//////////////////////////////////////////////////////////////////////////////

			fprintf(stderr,"Send General Interrogation request\n");
			fflush(stderr);
		}
	}

	return;
}

void iec104_imp::alloc_command_resources(void)
{

}

void iec104_imp::free_command_resources(void)
{

}
*/