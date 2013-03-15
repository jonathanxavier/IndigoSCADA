/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2013 Enscada 
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
#include "modbus_item.h"
#include "modbus_imp.h"
#include "stdlib.h"

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

	fprintf(stderr,"iec_type = %u\n", item2->iec_type);
	fprintf(stderr,"iec_obj = %x\n", item2->iec_obj);
	fprintf(stderr,"cause = %u\n", item2->cause);
	fprintf(stderr,"msg_id =%u\n", item2->msg_id);
	fprintf(stderr,"ioa_control_center = %u\n", item2->ioa_control_center);
	fprintf(stderr,"casdu =%u\n", item2->casdu);
	fprintf(stderr,"is_neg = %u\n", item2->is_neg);
	fprintf(stderr,"checksum = %u\n", item2->checksum);
}

void recvCallBack(const ORTERecvInfo *info,void *vinstance, void *recvCallBackParam) 
{
	modbus_imp * cl = (modbus_imp*)recvCallBackParam;
	iec_item_type *item1 = (iec_item_type*)vinstance;

	switch (info->status) 
	{
		case NEW_DATA:
		{
		  if(!quite)
		  {
			  struct iec_item item2;
			  rebuild_iec_item_message(&item2, item1);
			  cl->received_command_callback = 1;
			  cl->check_for_commands(&item2);
			  cl->received_command_callback = 0;
		  }
		}
		break;
		case DEADLINE:
		{
			printf("deadline occurred\n");
		}
		break;
	}
}
////////////////////////////////Middleware/////////////////////////////////////

////////////////////////////////Middleware/////////////////
iec_item_type modbus_imp::instanceSend;
ORTEPublication* modbus_imp::publisher = NULL;
////////////////////////////////Middleware/////////////////

//   
//  Class constructor.   
//   
modbus_imp::modbus_imp(struct modbusContext* my_ctx, char* line_number, int polling_time):
fExit(false),pollingTime(polling_time), general_interrogation(true)
{   
	strcpy(lineNumber, line_number);
	my_modbus_context.use_context = my_ctx->use_context;
	my_modbus_context.server_id = my_ctx->server_id;

	if(my_modbus_context.use_context == TCP)
	{
		strcpy(my_modbus_context.modbus_server_address, my_ctx->modbus_server_address);
		strcpy(my_modbus_context.modbus_server_port, my_ctx->modbus_server_port);
	}
	else if(my_modbus_context.use_context == RTU)
	{
		strcpy(my_modbus_context.serial_device, my_ctx->serial_device);
		my_modbus_context.baud = my_ctx->baud;
		my_modbus_context.data_bit = my_ctx->data_bit;
		my_modbus_context.stop_bit = my_ctx->stop_bit;
		my_modbus_context.parity = my_ctx->parity;
	}

	if(my_modbus_context.use_context == TCP) 
	{
        ctx = modbus_new_tcp(my_modbus_context.modbus_server_address, atoi(my_modbus_context.modbus_server_port));
    } 
	else 
	{
        ctx = modbus_new_rtu(my_modbus_context.serial_device, my_modbus_context.baud, my_modbus_context.parity, my_modbus_context.data_bit, my_modbus_context.stop_bit);
    }

    if (ctx != NULL) 
	{
		/////////////////////Middleware/////////////////////////////////////////////////////////////////
		received_command_callback = 0;

		int32_t                 strength = 1;
		NtpTime                 persistence, deadline, minimumSeparation, delay;
		IPAddress				smIPAddress = IPADDRESS_INVALID;
		
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
		strcat(fifo_monitor_name, "modbus");

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
		strcat(fifo_control_name, "modbus");

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
	else
	{
        fprintf(stderr, "Unable to allocate libmodbus context\n");
		fExit = 1;
		return;
    }

	modbus_set_debug(ctx, TRUE);

    modbus_set_error_recovery(ctx,(modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL));

    modbus_set_slave(ctx, my_modbus_context.server_id);

    if (modbus_connect(ctx) == -1) 
	{
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
		fExit = 1;
        return;
    }
}   
//   
//  Class destructor.   
//   
modbus_imp::~modbus_imp()  
{   
    // free resources   
	fExit = 1;
    return;   
}   

static u_int n_msg_sent = 0;

int modbus_imp::PollServer(void)
{
	IT_IT("modbus_imp::PollServer");
	
	int rc = 0;

	/* Allocate and initialize the memory to store the bits */
	#define MAX_BITS_IN_MEMORY_BLOCK 30

	nb_points = MAX_BITS_IN_MEMORY_BLOCK;
	tab_rp_bits = (uint8_t *) malloc(nb_points * sizeof(uint8_t));

	memset(tab_rp_bits, 0, nb_points * sizeof(uint8_t));

	#define MAX_REGISTERS_IN_MEMORY_BLOCK 30

	/* Allocate and initialize the memory to store the registers */
	nb_points = MAX_REGISTERS_IN_MEMORY_BLOCK;

	tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));

	memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

	general_interrogation = true;
  
	int loops = 0;

	while(true) //the polling loop
	{	
		rc = PollItems();

		loops++;
		if(loops == 2)
		{
			general_interrogation = false;
		}
	
		if(rc)
		{ 
			fprintf(stderr,"modbus on line %d exiting...., due to lack of connection with server\n", lineNumber);
			fflush(stderr);

			IT_COMMENT("modbus_imp exiting...., due to lack of connection with server");
			
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
			//prepare published data
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
		
			break; 
		}


		if(fExit)
		{
			IT_COMMENT("Terminate modbus loop!");
			break;
		}
				
		::Sleep(pollingTime);
	}
	
	IT_EXIT;
	return 0;
}

int modbus_imp::Start(void)
{
	IT_IT("modbus_imp::Start");
	
	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA MODBUS master Start\n");
	LogMessage(NULL, show_msg);

	if(fExit == 1)
	{
		return(1); //error
	}
	
	if(AddItems())
	{
		return(1); //error
	}

	IT_EXIT;
    return(0);
}

int modbus_imp::Stop()
{
	IT_IT("modbus_imp::Stop");

	fprintf(stderr,"Entering Stop()\n");
	fflush(stderr);

	/* Free the memory */
    free(tab_rp_bits);
    free(tab_rp_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

	while(received_command_callback)
	{
		Sleep(100);
	}
	
	// terminate server and it will clean up itself

	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA MODBUS master End\n");
	LogMessage(NULL, show_msg);

	IT_EXIT;
	return 1;
}

struct log_message{

	int ioa;
	char message[150];
};

void modbus_imp::LogMessage(int* error, const char* name)
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

void modbus_imp::get_utc_host_time(struct cp56time2a* time)
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

time_t modbus_imp::epoch_from_cp56time2a(const struct cp56time2a* time)
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

//#define ABS(x) ((x) >= 0 ? (x) : -(x))

//Retun 1 on error
int modbus_imp::PollItems(void)
{
	IT_IT("modbus_imp::PollItems");

	struct iec_item item_to_send;
	struct cp56time2a actual_time;
	////////////////////////////////Start protocol implementation///////////////////////////////////
	int rc;
    bool send_item;
	int bit_size;
    float real;
	int integer;

	/*
	//////////////////MODBUS RTU part/////////////////////////////////////////
	Config_db[i].name[100];			//Item ID is an unique name identifing it
	Config_db[i].modbus_function_read;	//modbus funtion to read
	Config_db[i].modbus_function_write;	//modbus funtion to write
	Config_db[i].modbus_start_address; //start address of the memory to fetch
	Config_db[i].block_size; //size in bit of the the memory to fetch
	Config_db[i].offset; //offset in bits into the memory to fetch (to sum with start address in order to find the address of interested data)
	//////////////control center part///////////////////////////////////
	Config_db[i].ioa_control_center; //unique inside CASDU
	Config_db[i].iec_type_read;   //IEC 104 type to read
	Config_db[i].iec_type_write;   //IEC 104 type to write
	Config_db[i].size_in_bits_of_iec_type; //The sise in bits of the IEC 104 type
*/

	for(int rowNumber = 0; rowNumber < db_n_rows; rowNumber++)
	{
		memset(&item_to_send,0x00, sizeof(struct iec_item));

		memset(tab_rp_bits, 0x00, nb_points * sizeof(uint8_t));

		memset(tab_rp_registers, 0x00, nb_points * sizeof(uint16_t));

		/* Function codes */
		if(Config_db[rowNumber].modbus_function_read == FC_READ_COILS)
		{
			//0x01
				
				//if(Config_db[rowNumber].block_size != 1)
				//{
					//error
				//}

				bit_size = 1;

				int address = Config_db[rowNumber].modbus_start_address + Config_db[rowNumber].offset;

				rc = modbus_read_bits(ctx, address, bit_size, tab_rp_bits);

				//if (rc != Config_db[rowNumber].block_size) 
				//{
					//printf("FAILED (nb points %d)\n", rc);
					//error
				//}

				uint8_t value = tab_rp_bits[0];

				printf("modbus_read_bits: value = %d\n", (int)value);

				if(Config_db[rowNumber].last_value.a != value)
				{
					Config_db[rowNumber].last_value.a = value;

					send_item = true;
				}
				else
				{
					send_item = false;
				}

				item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;

				item_to_send.cause = 0x03;
			
				item_to_send.iec_type = M_SP_TB_1;
				
				get_utc_host_time(&actual_time);

				item_to_send.iec_obj.o.type30.sp = value;
				item_to_send.iec_obj.o.type30.time = actual_time;
				item_to_send.iec_obj.o.type30.iv = 0;
				
				IT_COMMENT1("Value = %d", value);
			
		}
		else if(Config_db[rowNumber].modbus_function_read == FC_READ_DISCRETE_INPUTS)
		{
			//0x02
			printf("Function %x not supported\n", 0x02);
		}
		else if(Config_db[rowNumber].modbus_function_read == FC_READ_HOLDING_REGISTERS)
		{
			//0x03
			//int registers = Config_db[i].block_size/16;

			int registers = 2; //we read 32 bits

			int address = Config_db[rowNumber].modbus_start_address + Config_db[rowNumber].offset;

			rc = modbus_read_registers(ctx, address, registers, tab_rp_registers);
			printf("modbus_read_registers: ");

			if (rc != registers) 
			{
				printf("FAILED (nb points %d)\n", rc);
				//error
			}

			if(Config_db[rowNumber].iec_type_read == M_ME_TF_1)
			{
				real = modbus_get_float(tab_rp_registers);

				printf("Get float: %f\n", real);

				if(Config_db[rowNumber].last_value.f != real)
				{
					Config_db[rowNumber].last_value.f = real;

					send_item = true;
				}
				else
				{
					send_item = false;
				}

				item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;

				item_to_send.cause = 0x03;

				item_to_send.iec_type = M_ME_TF_1;
				
				get_utc_host_time(&actual_time);

				item_to_send.iec_obj.o.type36.mv = real;
				item_to_send.iec_obj.o.type36.time = actual_time;
				item_to_send.iec_obj.o.type36.iv = 0;
			}
			else if(Config_db[rowNumber].iec_type_read == M_IT_TB_1)
			{
				memcpy(&integer, tab_rp_registers, sizeof(int));

				printf("Get integer: %d\n", integer);

				if(Config_db[rowNumber].last_value.a != integer)
				{
					Config_db[rowNumber].last_value.a = integer;

					send_item = true;
				}
				else
				{
					send_item = false;
				}

				item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;

				item_to_send.cause = 0x03;

				item_to_send.iec_type = M_IT_TB_1;
				
				get_utc_host_time(&actual_time);

				item_to_send.iec_obj.o.type37.counter = integer;
				item_to_send.iec_obj.o.type37.time = actual_time;
				item_to_send.iec_obj.o.type37.iv = 0;
			}
		}
		else if(Config_db[rowNumber].modbus_function_read == FC_READ_INPUT_REGISTERS)
		{
			//0x04
			printf("Function %x not supported\n", 0x04);
		
		}
		else if(Config_db[rowNumber].modbus_function_read == FC_READ_EXCEPTION_STATUS)
		{
			//0x07
			printf("Function %x not supported\n", 0x07);
		
		}
		else if(Config_db[rowNumber].modbus_function_read == FC_REPORT_SLAVE_ID)
		{
			//0x11
			printf("Function %x not supported\n", 0x11);
		}
		else if(Config_db[rowNumber].modbus_function_read == FC_WRITE_AND_READ_REGISTERS)
		{
			//0x17
			printf("Function %x not supported\n", 0x17);
		}
		else
		{
			printf("Function not supported\n");
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

			Sleep(10); //Without delay there is missing of messages in the loading

			//Send in monitor direction
			fprintf(stderr,"Sending message %u th\n", n_msg_sent);
			fflush(stderr);
			IT_COMMENT1("Sending message %u th\n", n_msg_sent);

			//prepare published data
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

short modbus_imp::rescale_value(double V, double Vmin, double Vmax, int* error)
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

double modbus_imp::rescale_value_inv(double A, double Vmin, double Vmax, int* error)
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


void modbus_imp::check_for_commands(struct iec_item *queued_item)
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
					modbus_imp::LogMessage(0, show_msg);
				
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
				
				unsigned int v = 0;
				float cmd_val = 0.0;

				//switch(queued_item->iec_type)
				switch(Config_db[rowNumber].iec_type_write)
				{
					case C_SC_TA_1:
					{
						//v = queued_item->iec_obj.o.type58.scs;

						v = queued_item->iec_obj.o.type63.sv;
						
						if(Config_db[rowNumber].modbus_function_write == FC_WRITE_SINGLE_COIL)
						{
							//0x05

							//COIL BITS

							// Single
							int rc;
							
							rc = modbus_write_bit(ctx, Config_db[rowNumber].modbus_start_address + Config_db[rowNumber].offset, v);

							printf("modbus_write_bit: ");

							if (rc == 1) {
								printf("OK\n");
							} else {
								printf("FAILED\n");
								//error
							}
						}
					}
					break;
					case C_DC_TA_1:
					{
						v = queued_item->iec_obj.o.type59.dcs;
						cmd_val = (float)v;
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
						cmd_val = queued_item->iec_obj.o.type63.sv;

						if(Config_db[rowNumber].modbus_function_write == FC_WRITE_MULTIPLE_REGISTERS)
						{
							//0x10

							modbus_set_float(cmd_val, tab_rp_registers);

							//int registers = (Config_db[rowNumber].block_size - Config_db[rowNumber].offset)/16;

							int registers = 2; //we write 32 bits

							// Many registers
							int rc;
							rc = modbus_write_registers(ctx, Config_db[rowNumber].modbus_start_address + Config_db[rowNumber].offset, registers, tab_rp_registers);

							printf("modbus_write_registers: ");

							if (rc == registers) 
							{
								printf("OK\n");
							} 
							else 
							{
								printf("FAILED\n");
								//error
							}
						}
					}
					break;
					case C_BO_TA_1:
					{
						memcpy(&v, &(queued_item->iec_obj.o.type64.stcd), sizeof(struct iec_stcd));
						
						if(Config_db[rowNumber].modbus_function_write == FC_WRITE_MULTIPLE_REGISTERS)
						{
							//0x10

							memcpy(tab_rp_registers, &v, sizeof(unsigned int));

							//int registers = (Config_db[rowNumber].block_size - Config_db[rowNumber].offset)/16;

							int registers = 2; //we write 32 bits

							// Many registers
							int rc;
							rc = modbus_write_registers(ctx, Config_db[rowNumber].modbus_start_address + Config_db[rowNumber].offset, registers, tab_rp_registers);

							printf("modbus_write_registers: ");

							if (rc == registers) 
							{
								printf("OK\n");
							} 
							else 
							{
								printf("FAILED\n");
								//error
							}
						}
					}
					break;
					case C_SC_NA_1:
					{
						v = queued_item->iec_obj.o.type45.scs;
						cmd_val = (float)v;
					}
					break;
					case C_DC_NA_1:
					{
						v = queued_item->iec_obj.o.type46.dcs;
						cmd_val = (float)v;
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
						cmd_val = queued_item->iec_obj.o.type50.sv;
					}
					break;
					case C_BO_NA_1:
					{
						memcpy(&v, &(queued_item->iec_obj.o.type51.stcd), sizeof(struct iec_stcd));
						cmd_val = (float)v;
					}
					break;
					default:
					{
						//error
						//fprintf(stderr,"Error %d, %s\n",__LINE__, __FILE__);
						//fflush(stderr);

						char show_msg[200];
						sprintf(show_msg, "Error %d, %s\n",__LINE__, __FILE__);
						modbus_imp::LogMessage(0, show_msg);
						
						return;
					}
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
			IT_COMMENT("Receiving general interrogation command from monitor.exe");
			fprintf(stderr,"Receiving general interrogation command from monitor.exe\n");
			fflush(stderr);
		}
	}

	return;
}

/*
		MODBUS FUNCTIONS NOT USED
		
		if(Config_db[i].modbus_function_write == FC_WRITE_SINGLE_REGISTER)
		{
			//0x06

			//Single register
			rc = modbus_write_register(ctx, Config_db[i].modbus_start_address, 0x1234);
			printf("1/2 modbus_write_register: ");
			if (rc == 1) {
				printf("OK\n");
			} else {
				printf("FAILED\n");
				//error
			}
		}
		else if(Config_db[i].modbus_function_write == FC_WRITE_MULTIPLE_COILS)
		{
			//0x0F

			//uint8_t tab_value[UT_BITS_NB];
			uint8_t tab_value[0x25];

			modbus_set_bits_from_bytes(tab_value, 0, Config_db[i].block_size, UT_BITS_TAB);

			rc = modbus_write_bits(ctx, Config_db[i].modbus_start_address, Config_db[i].block_size, tab_value);

			printf("1/2 modbus_write_bits: ");

			if (rc == Config_db[i].block_size) 
			{
				printf("OK\n");
			} else {
				printf("FAILED\n");
				//error
			}
		}
*/


void modbus_imp::alloc_command_resources(void)
{

}

void modbus_imp::free_command_resources(void)
{

}
