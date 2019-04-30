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

#include "stdint.h"
#include "iec104types.h"
#include "iec_item.h"
#include "clear_crc_eight.h"
#include "iec103_db.h"
#include "iec103_imp.h"
#include "stdlib.h"
#include "queue.h"
#include "iecserial.h"
#include "link_unbalance_master.h"
#define IEC_TIME_STRUCTS
#include "iec103.h"
#include "iec_103_item.h"
#include "serial.h"

#define MAX_COMMAND_SEND_TIME 60

#include <sys/stat.h>

extern int gl_timeout_connection_with_parent;

#ifdef __cplusplus
extern "C" {
#endif

void set_fifo_monitor_dir_handle(fifo_h f_m_d);
void set_fifo_control_dir_handle(fifo_h f_c_d);

#ifdef CHECK_TIMEOUT_WITH_PARENT
int check_timeout_with_parent(int polling_time)
{
	gl_timeout_connection_with_parent++;

	//fprintf(stderr, "timeout_connection_with_parent = %d\n", timeout_connection_with_parent);
	//fflush(stderr);

	if(gl_timeout_connection_with_parent > 1000*30/polling_time)
	{
		fprintf(stderr, "Exiting loop for timeout of connection with parent\n");
		fflush(stderr);
		return 1; //exit loop for timeout of connection with parent
	}

	return 0;
}
#endif

#ifdef __cplusplus
}
#endif

void monitoring_dir_consumer(void* pParam)
{
	struct subs_args* arg = (struct subs_args*)pParam;
	iec103_imp* parent = (iec103_imp*)arg->parent;
	unsigned char buf[sizeof(struct iec_103_item)];
    int len, rc;
	const unsigned wait_limit_ms = 1;
	struct iec_103_item* p_item;
	//////////////////iec 104 part //////////////////
	struct iec_item item_to_send;
	struct cp56time2a actual_time;
	/////////////////////////////////////////////////

	while(1)
	{
		if(parent->exit_threads)
		{
			break;
		}
	
		for(int i = 0; (len = fifo_get(parent->fifo_monitor_direction, (char*)buf, sizeof(struct iec_103_item), wait_limit_ms)) >= 0; i += 1)	
		{ 
			//Receive in monitor direction as iec 103 packet
			p_item = (struct iec_103_item*)buf;

			//for (int j = 0; j < len; j++) 
			//{ 
			//	unsigned char c = *((unsigned char*)buf + j);
				//printf("rx <--- 0x%02x-\n", c);

			//	IT_COMMENT1("rx <--- 0x%02x-\n", c);
			//}

			rc = clearCrc((unsigned char *)buf, sizeof(struct iec_103_item));

			if(rc != 0)
			{
				fprintf(stderr, "Error CRC8 = %d\n", rc);
				fflush(stderr);
				ExitProcess(0);
			}

			memset(&item_to_send, 0x00, sizeof(struct iec_item));

			item_to_send.cause = 0x03;
			//Make the query:
			//Select ioa_control_center from iec103_table where iec103_func and iec103_info
			//equal the values in the packet arriving

			//fprintf(stderr, "fun_type = %d\n", p_item->iec_obj.fun_type);
			//fflush(stderr);

			//fprintf(stderr, "inf_num = %d\n", p_item->iec_obj.inf_num);
			//fflush(stderr);

			//TODO: linear search, improve if necessary
			int found = 0;
			for(int rowNumber = 0; rowNumber < parent->db_n_rows; rowNumber++)
			{
				if((parent->Config_db[rowNumber].iec103_func == p_item->iec_obj.fun_type) && 
					(parent->Config_db[rowNumber].iec103_info == p_item->iec_obj.inf_num))
				{
					item_to_send.iec_obj.ioa = parent->Config_db[rowNumber].ioa_control_center;
					found = 1;
					break;
				}
			}

            switch(p_item->iec_type)
            {
                case TYPE_1:
                {
					item_to_send.iec_type = M_DP_TB_1;
					item_to_send.iec_obj.o.type31.dp = p_item->iec_obj.o.type1.m_Dpi.dpi.dpi;
									
					parent->get_local_host_time(&actual_time);
					item_to_send.iec_obj.o.type31.time = actual_time;
					item_to_send.iec_obj.o.type31.iv = 0;

                    fprintf(stderr,"Time-tagged message\n");
                    fflush(stderr);
                }
                break;
                case TYPE_2:
                {
					item_to_send.iec_type = M_DP_TB_1;
					item_to_send.iec_obj.o.type31.dp = p_item->iec_obj.o.type2.m_Dpi.dpi.dpi;

					parent->get_local_host_time(&actual_time);
					item_to_send.iec_obj.o.type31.time = actual_time;
					item_to_send.iec_obj.o.type31.iv = 0;

                    fprintf(stderr,"Time-tagged message with relative time\n");
                    fflush(stderr);
                }
                break;
                case TYPE_3:
                {
					item_to_send.iec_type = M_ME_TE_1;
					item_to_send.iec_obj.o.type35.mv = p_item->iec_obj.o.type3.M.mea.MVAL;

					parent->get_local_host_time(&actual_time);
					item_to_send.iec_obj.o.type35.time = actual_time;
					item_to_send.iec_obj.o.type35.iv = p_item->iec_obj.o.type3.M.mea.ER;

                    fprintf(stderr,"Measurands I\n");
                    fflush(stderr);
                }
                break;
                case TYPE_4:
                {
					item_to_send.iec_type = M_ME_TF_1;
					item_to_send.iec_obj.o.type36.mv = p_item->iec_obj.o.type4.m_SCL;

					parent->get_local_host_time(&actual_time);
					item_to_send.iec_obj.o.type36.time = actual_time;
					item_to_send.iec_obj.o.type36.iv = 0;

                    fprintf(stderr,"Time-tagged measurands with relative time\n");
                    fflush(stderr);
                }
                break;
                case TYPE_5:
                {
                    //fprintf(stderr,"Identification\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_6:
                {
                    //fprintf(stderr,"Clock synchronization\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_8:
                {
	                //fprintf(stderr,"End of general interrogation\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_9:
                {
					item_to_send.iec_type = M_ME_TE_1;
					item_to_send.iec_obj.o.type35.mv = p_item->iec_obj.o.type9.M.mea.MVAL;

					parent->get_local_host_time(&actual_time);
					item_to_send.iec_obj.o.type35.time = actual_time;
					item_to_send.iec_obj.o.type35.iv = p_item->iec_obj.o.type9.M.mea.ER;

                    fprintf(stderr,"Measurands II\n");
                    fflush(stderr);
                }
                break;
                case TYPE_10:
                {
                    //fprintf(stderr,"Generic data\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_11:
                {
                    //fprintf(stderr,"Generic identification\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_23:
                {
                    //fprintf(stderr,"List of recorded disturbances\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_26:
                {
                    //fprintf(stderr,"Ready for transmission of disturbance data\n");
                    //fflush(stderr);
	            }
                break;
                case TYPE_27:
                {
                    //fprintf(stderr,"Ready for transmission of a channel\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_28:
                {
                    //fprintf(stderr,"Ready for transmission of tags\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_29:
                {
                    //fprintf(stderr,"Transmission of tags\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_30:
                {
                    //fprintf(stderr,"Transmission of disturbance values\n");
                    //fflush(stderr);
                }
                break;
                case TYPE_31:
                {
                    //fprintf(stderr,"End of transmission\n");
                    //fflush(stderr);
                }
                break;
                default:
                    fprintf(stderr,"Unknown IEC 103 type\n");
                    fflush(stderr);
                break;
            }

			item_to_send.msg_id = parent->n_msg_sent_monitor_dir;
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

			//Send in monitor direction as iec 104 packet
			fprintf(stderr,"Sending message %u th\n", parent->n_msg_sent_monitor_dir);
			fflush(stderr);

			#ifdef USE_RIPC_MIDDLEWARE
			//publishing data
			parent->queue_monitor_dir->put(&item_to_send, sizeof(struct iec_item));
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
			
			parent->n_msg_sent_monitor_dir++;
		}
	}
}

#ifdef USE_RIPC_MIDDLEWARE
///////commands
void control_dir_consumer(void* pParam)
{
	struct subs_args* arg = (struct subs_args*)pParam;
	struct iec_item item;
	RIPCObject objDesc(&item, sizeof(struct iec_item));

	iec103_imp* parent = (iec103_imp*)arg->parent;

	while(1)
	{
		if(parent->exit_threads)
		{
			break;
		}

		parent->queue_control_dir->get(objDesc);

		parent->check_for_commands(&item);
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
	iec103_imp * cl = (iec103_imp*)recvCallBackParam;
	iec_item_type *item1 = (iec_item_type*)vinstance;

	switch (info->status) 
	{
		case NEW_DATA:
		{
		  if(!quite)
		  {
			  struct iec_item item2;
			  rebuild_iec_item_message(&item2, item1);
			  cl->check_for_commands(&item2);
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
iec103_imp::iec103_imp(struct iec103Context* my_ctx, char* line_number, int polling_time):
fExit(false),pollingTime(polling_time), general_interrogation(true), is_connected(false),
exit_threads(0),n_msg_sent_monitor_dir(0),n_msg_sent_control_dir(0),Config_db(NULL)
{   
	lineNumber = atoi(line_number);
		
	strcpy(my_iec103_context.serial_device, my_ctx->serial_device);
	my_iec103_context.baud = my_ctx->baud;
		
	/////////////////////Middleware/////////////////////////////////////////////////////////////////
	char fifo_monitor_name[150];
	char fifo_control_name[150];

	strcpy(fifo_monitor_name,"fifo_monitor_direction");
	strcat(fifo_monitor_name, line_number);
	strcat(fifo_monitor_name, "iec103");
		
	strcpy(fifo_control_name,"fifo_control_direction");
	strcat(fifo_control_name, line_number);
	strcat(fifo_control_name, "iec103");

	#ifdef USE_RIPC_MIDDLEWARE
	port = 6000;
	hostname = "localhost";

	factory1 = RIPCClientFactory::getInstance();
	factory2 = RIPCClientFactory::getInstance();

	session1 = factory1->create(hostname, port);
	session2 = factory2->create(hostname, port);
	queue_monitor_dir = session1->createQueue(fifo_monitor_name);
	queue_control_dir = session2->createQueue(fifo_control_name);
	#endif //USE_RIPC_MIDDLEWARE

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
	fifo_monitor_name,
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

	strcat(fifo_monitor_name, "_fifo_client");

	#define MAX_FIFO_SIZE 65535
	fifo_monitor_direction = fifo_open(fifo_monitor_name, MAX_FIFO_SIZE, iec_call_exit_handler);

	set_fifo_monitor_dir_handle(fifo_monitor_direction);
	
	unsigned long threadid;
	
	CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(monitoring_dir_consumer), (void*)&arg, 0, &threadid);

	strcat(fifo_control_name, "_fifo_client");

	fifo_control_direction = fifo_open(fifo_control_name, MAX_FIFO_SIZE, iec_call_exit_handler);

	set_fifo_control_dir_handle(fifo_control_direction);
	
	#ifdef USE_RIPC_MIDDLEWARE
	CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(control_dir_consumer), (void*)&arg, 0, &threadid);
	#endif
}   
//   
//  Class destructor.   
//   
iec103_imp::~iec103_imp()  
{   
    // free resources   
	fExit = 1;

	#ifdef USE_RIPC_MIDDLEWARE
	////////Middleware/////////////
//	Sleep(3000);
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

int iec103_imp::PollServer(void)
{
	IT_IT("iec103_imp::PollServer");
	
	State_process();
	
	IT_EXIT;
	return 0;
}

int iec103_imp::Start(void)
{
	IT_IT("iec103_imp::Start");
	
	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA iec103 master Start\n");
	LogMessage(NULL, show_msg);

	if(fExit == 1)
	{
		return(1); //error
	}
	
	if(AddItems())
	{
		return(1); //error
	}

	int rc = iecserial_connect(my_iec103_context.serial_device, my_iec103_context.baud);
	
    if (rc == -1) 
	{
        fprintf(stderr, "Connection failed\n");
		is_connected = false;
		return(1); //error
    }
	else
	{
		is_connected = true;
	}

	IT_EXIT;
    return(0);
}

int iec103_imp::Stop()
{
	IT_IT("iec103_imp::Stop");

	fprintf(stderr,"Entering Stop()\n");
	fflush(stderr);

	exit_threads = 1;
	Sleep(3000);
	CloseLink();

	fifo_close(fifo_monitor_direction);
	fifo_close(fifo_control_direction);
	close_port((int)fd);
	
	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA iec103 master End\n");
	LogMessage(NULL, show_msg);

	IT_EXIT;
	return 1;
}

struct log_message{

	int ioa;
	char message[150];
};

void iec103_imp::LogMessage(int* error, const char* name)
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

#include <time.h>
#include <sys/timeb.h>

void iec103_imp::get_local_host_time(struct cp56time2a* time)
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

void iec103_imp::get_utc_host_time(struct cp56time2a* time)
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

time_t iec103_imp::epoch_from_cp56time2a(const struct cp56time2a* time)
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

short iec103_imp::rescale_value(double V, double Vmin, double Vmax, int* error)
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

double iec103_imp::rescale_value_inv(double A, double Vmin, double Vmax, int* error)
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


void iec103_imp::check_for_commands(struct iec_item *queued_item)
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
			
			//check iec type of command
			//if(Config_db[rowNumber].iec_type_write != queued_item->iec_type)
			//{
			//	//error
			//	fprintf(stderr,"Error: Command with IOA %d has iec_type %d, different from IO list type %d\n", queued_item->iec_obj.ioa, queued_item->iec_type, Config_db[rowNumber].iec_type_write);
			//	fflush(stderr);
			//	fprintf(stderr,"Command NOT executed\n");
			//	fflush(stderr);
			//	return;
			//}
			
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
					iec103_imp::LogMessage(0, show_msg);
				
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
				union {
					unsigned int v;
					float f;
				} cmd_val;

				switch(queued_item->iec_type)
				{
					case C_SC_TA_1:
					{
						cmd_val.v = queued_item->iec_obj.o.type58.scs;
					}
					break;
					case C_DC_TA_1:
					{
						cmd_val.f = (float)queued_item->iec_obj.o.type59.dcs;
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
					}
					break;
					case C_BO_TA_1:
					{
						memcpy(&(cmd_val.v), &(queued_item->iec_obj.o.type64.stcd), sizeof(struct iec_stcd));
					}
					break;
					case C_SC_NA_1:
					{
						cmd_val.f = (float)queued_item->iec_obj.o.type45.scs;
					}
					break;
					case C_DC_NA_1:
					{
						cmd_val.f = (float)queued_item->iec_obj.o.type46.dcs;
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
					}
					break;
					case C_BO_NA_1:
					{
						memcpy(&(cmd_val.v), &(queued_item->iec_obj.o.type51.stcd), sizeof(struct iec_stcd));
					}
					break;
					default:
					{
						//error
						//fprintf(stderr,"Error %d, %s\n",__LINE__, __FILE__);
						//fflush(stderr);

						char show_msg[200];
						sprintf(show_msg, "Error %d, %s\n",__LINE__, __FILE__);
						iec103_imp::LogMessage(0, show_msg);
						
						return;
					}
					break;
				}

				switch(Config_db[rowNumber].iec103_type_command)
				{
					case TYPE_20:
					{
						//This is a double command
						struct iec_103_item item_to_send;

						memset(&item_to_send,0x00, sizeof(struct iec_103_item));
							
						item_to_send.iec_type = TYPE_20;
						item_to_send.cause = 20; //Command
						item_to_send.iec_obj.fun_type = Config_db[rowNumber].iec103_func;
						item_to_send.iec_obj.inf_num = Config_db[rowNumber].iec103_info;
						item_to_send.iec_obj.o.type20.m_DCO = (u_int)cmd_val.f; //Command state
						item_to_send.iec_obj.o.type20.m_RII = 33; //Return information identifier
						
						item_to_send.msg_id = n_msg_sent_control_dir++;
						
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_103_item));
						////////////////////////////////////////////////////////////////
						fifo_put(fifo_control_direction, (char*)&item_to_send, sizeof(struct iec_103_item));
						//////////////////////////////////////////////////////////////////////////////

						fprintf(stderr,"Send General command\n");
						fflush(stderr);						
					}
					break;
					case TYPE_6:
					{
						struct iec_103_item item_to_send;
						struct cp56time2a actual_time;

						get_utc_host_time(&actual_time);

						memset(&item_to_send,0x00, sizeof(struct iec_103_item));
							
						item_to_send.iec_type = TYPE_6;
						item_to_send.cause = 8; //Synchronization
						item_to_send.iec_obj.fun_type = Config_db[rowNumber].iec103_func;
						item_to_send.iec_obj.inf_num = Config_db[rowNumber].iec103_info;
						item_to_send.iec_obj.o.type6.time = actual_time;
						item_to_send.msg_id = n_msg_sent_control_dir++;
						
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_103_item));
						////////////////////////////////////////////////////////////////
						fifo_put(fifo_control_direction, (char*)&item_to_send, sizeof(struct iec_103_item));
						//////////////////////////////////////////////////////////////////////////////

						fprintf(stderr,"Send Clock synchronization\n");
						fflush(stderr);
					}
					break;
					/*
					case TYPE_10:
					{
						struct iec_103_item item_to_send;

						memset(&item_to_send,0x00, sizeof(struct iec_103_item));
							
						item_to_send.iec_type = TYPE_10;
						item_to_send.cause = 12; //Remote operation
						item_to_send.iec_obj.fun_type = Config_db[rowNumber].iec103_func;
						item_to_send.iec_obj.inf_num = Config_db[rowNumber].iec103_info;
						
						item_to_send.msg_id = n_msg_sent_control_dir++;
						
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_103_item));
						////////////////////////////////////////////////////////////////
						fifo_put(fifo_control_direction, (char*)&item_to_send, sizeof(struct iec_103_item));
						//////////////////////////////////////////////////////////////////////////////

						fprintf(stderr,"Send Generic data\n");
						fflush(stderr);
					}
					break;
					case TYPE_21:
					{
						struct iec_103_item item_to_send;

						memset(&item_to_send,0x00, sizeof(struct iec_103_item));
							
						item_to_send.iec_type = TYPE_21;
						item_to_send.cause = 42; //ACK read command
						item_to_send.iec_obj.fun_type = Config_db[rowNumber].iec103_func;
						item_to_send.iec_obj.inf_num = Config_db[rowNumber].iec103_info;
						
						item_to_send.msg_id = n_msg_sent_control_dir++;
						
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_103_item));
						////////////////////////////////////////////////////////////////
						fifo_put(fifo_control_direction, (char*)&item_to_send, sizeof(struct iec_103_item));
						//////////////////////////////////////////////////////////////////////////////

						fprintf(stderr,"Send Generic command\n");
						fflush(stderr);
					}
					break;
					case TYPE_24:
					{
						struct iec_103_item item_to_send;

						memset(&item_to_send,0x00, sizeof(struct iec_103_item));
							
						item_to_send.iec_type = TYPE_24; //Order for disturbance data transmission
    
						item_to_send.cause = 31; //Transmission
						item_to_send.iec_obj.fun_type = Config_db[rowNumber].iec103_func;
						item_to_send.iec_obj.inf_num = Config_db[rowNumber].iec103_info;
						item_to_send.iec_obj.o.type24.m_ACC = 0; //Actual channel
						item_to_send.iec_obj.o.type24.m_FAN = 81; //Fault number
						item_to_send.iec_obj.o.type24.m_TOO = 1; //Type of order
						item_to_send.iec_obj.o.type24.m_TOV = 1; //Type of disturbance values
						
						item_to_send.msg_id = n_msg_sent_control_dir++;
						
						item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_103_item));
						////////////////////////////////////////////////////////////////
						fifo_put(fifo_control_direction, (char*)&item_to_send, sizeof(struct iec_103_item));
						//////////////////////////////////////////////////////////////////////////////

						fprintf(stderr,"Send Order for disturbance data transmission\n");
						fflush(stderr);						
					}
					break;
					*/
					default:
					break;
				}
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

			/*
			struct iec_103_item item_to_send;

			memset(&item_to_send,0x00, sizeof(struct iec_103_item));
				
			item_to_send.iec_type = TYPE_7;
			item_to_send.cause = 9; //GI
			item_to_send.iec_obj.fun_type = 255;
			item_to_send.iec_obj.inf_num = 0;
			item_to_send.iec_obj.o.type7.m_SCN = 123;
			
			item_to_send.msg_id = n_msg_sent_control_dir++;
			
			item_to_send.checksum = clearCrc((unsigned char *)&item_to_send, sizeof(struct iec_103_item));
			////////////////////////////////////////////////////////////////
			fifo_put(fifo_control_direction, (char*)&item_to_send, sizeof(struct iec_103_item));
			//////////////////////////////////////////////////////////////////////////////

			fprintf(stderr,"Send General Interrogation request\n");
			fflush(stderr);
			
			////////////General interrogation condition//////////////
			general_interrogation = true;
			//////////////////////////////////////////////////////////
			*/
		}
	}

	return;
}

void iec103_imp::alloc_command_resources(void)
{

}

void iec103_imp::free_command_resources(void)
{

}
