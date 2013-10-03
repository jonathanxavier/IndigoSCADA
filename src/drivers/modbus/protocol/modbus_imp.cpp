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
fExit(false),pollingTime(polling_time), general_interrogation(true), is_connected(false)
{   
	lineNumber = atoi(line_number);
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
        //modbus_free(ctx);
		//fExit = 1;
        //return;
		is_connected = false;
    }
	else
	{
		is_connected = true;
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

	memset(tab_rp_bits, 0x00, nb_points * sizeof(uint8_t));

	#define MAX_REGISTERS_IN_MEMORY_BLOCK 30

	/* Allocate and initialize the memory to store the registers */
	nb_points = MAX_REGISTERS_IN_MEMORY_BLOCK;

	tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));

	memset(tab_rp_registers, 0x00, nb_points * sizeof(uint16_t));

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
			
				//break; //this terminate the loop and the program

				is_connected = false;

				modbus_close(ctx);
			}
		}
		else
		{
			//Try to reconnect
			if (modbus_connect(ctx) == -1) 
			{
				fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
			}
			else
			{
				is_connected = true;

				////////////General interrogation condition//////////////
				general_interrogation = true;
				loops = 0;
				//////////////////////////////////////////////////////////
			}
		}

		if(fExit)
		{
			IT_COMMENT("Terminate modbus loop!");
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

#define ABS(x) ((x) >= 0 ? (x) : -(x))

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
    	
    comm_error_counter = 0;

	for(int rowNumber = 0; rowNumber < db_n_rows; rowNumber++)
	{
		memset(&item_to_send,0x00, sizeof(struct iec_item));

		memset(tab_rp_bits, 0x00, nb_points * sizeof(uint8_t));

		memset(tab_rp_registers, 0x00, nb_points * sizeof(uint16_t));

		/* Function codes */
		if(Config_db[rowNumber].modbus_function_read == FC_READ_COILS)
		{
			//0x01
				
			if(Config_db[rowNumber].modbus_type == VT_BOOL)
			{
				bit_size = 1;

				int address = Config_db[rowNumber].modbus_start_address;

				rc = modbus_read_bits(ctx, address, bit_size, tab_rp_bits);

				if (rc != 1) 
				{
                    comm_error_counter++;
					
                    continue;
				}

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
			else
			{
				printf("Modbus type %d not supported with FC_READ_COILS", Config_db[rowNumber].modbus_type);
			}
		}
		else if(Config_db[rowNumber].modbus_function_read == FC_READ_DISCRETE_INPUTS)
		{
			//0x02

			if(Config_db[rowNumber].modbus_type == VT_BOOL)
			{
				bit_size = 1;

				int address = Config_db[rowNumber].modbus_start_address;

				rc = modbus_read_input_bits(ctx, address, bit_size, tab_rp_bits);

				if (rc != 1) 
				{
                    comm_error_counter++;
					
                    continue;
				}

				uint8_t value = tab_rp_bits[0];

				printf("modbus_read_input_bits: value = %d\n", (int)value);

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
			else
			{
				printf("Modbus type %d not supported with FC_READ_COILS", Config_db[rowNumber].modbus_type);
			}
		}
		else if(Config_db[rowNumber].modbus_function_read == FC_READ_HOLDING_REGISTERS)
		{
			//0x03
			if((Config_db[rowNumber].modbus_type == VT_I4) || (Config_db[rowNumber].modbus_type == VT_R4))
			{
				int registers = 2; //read 32 bits

				int address = Config_db[rowNumber].modbus_start_address;

				rc = modbus_read_registers(ctx, address, registers, tab_rp_registers);

				printf("modbus_read_registers: ");

				if (rc != registers) 
				{
                    comm_error_counter++;
					
                    continue;
				}

				if(Config_db[rowNumber].iec_type_read == M_ME_TF_1)
				{
					float real;

					real = modbus_get_float(tab_rp_registers);

					printf("Get float: %f\n", real);

					if(ABS(Config_db[rowNumber].last_value.f - real) > Config_db[rowNumber].deadband)
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
					int integer32;
					integer32 = modbus_get_int(tab_rp_registers);

					printf("Get integer: %d\n", integer32);

					if(ABS(Config_db[rowNumber].last_value.a - integer32) > (int)Config_db[rowNumber].deadband)
					{
						Config_db[rowNumber].last_value.a = integer32;

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

					item_to_send.iec_obj.o.type37.counter = integer32;
					item_to_send.iec_obj.o.type37.time = actual_time;
					item_to_send.iec_obj.o.type37.iv = 0;
				}
			}
			else if(Config_db[rowNumber].modbus_type == VT_I2)
			{
				int registers = 1; //read 16 bits

				int address = Config_db[rowNumber].modbus_start_address;

				rc = modbus_read_registers(ctx, address, registers, tab_rp_registers);
				printf("modbus_read_registers: ");

				if (rc != registers) 
				{
                    comm_error_counter++;
					
                    continue;
				}

				if(Config_db[rowNumber].iec_type_read == M_ME_TE_1)
				{
					short integer16;
					integer16 = tab_rp_registers[0];

					printf("Get integer: %d\n", integer16);

					if(ABS(Config_db[rowNumber].last_value.a - integer16) > (short)Config_db[rowNumber].deadband)
					{
						Config_db[rowNumber].last_value.a = integer16;

						send_item = true;
					}
					else
					{
						send_item = false;
					}

					item_to_send.iec_obj.ioa = Config_db[rowNumber].ioa_control_center;

					item_to_send.cause = 0x03;

					item_to_send.iec_type = M_ME_TE_1;
					
					get_utc_host_time(&actual_time);

					item_to_send.iec_obj.o.type35.mv = integer16;
					item_to_send.iec_obj.o.type35.time = actual_time;
					item_to_send.iec_obj.o.type35.iv = 0;

				}
				else if(item_to_send.iec_type = M_SP_TB_1)
				{
					short integer16;
					integer16 = tab_rp_registers[0];

					//uint8_t value = get_bit_from_word(integer16, Config_db[rowNumber].offset);				
					//get a bit value from a word
					uint8_t value = integer16&(1 << Config_db[rowNumber].offset)  ? 1 : 0;				
					
					printf("get bit from word: value = %d\n", (int)value);

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
			}
			else
			{
				printf("Modbus type %d not supported with FC_READ_HOLDING_REGISTERS", Config_db[rowNumber].modbus_type);
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

    if(comm_error_counter >= db_n_rows)
    {
        IT_EXIT; //Lost connection with server...
	    return 1;
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

			if(delta < MAX_COMMAND_SEND_TIME && de   ;
tÿ×‹D$TÆ„$t  ‹J‰‹…Éu¡ˆB‹L$T;tÿ×‹D$Æ„$t  ‹J‰‹…Éu‹ˆB‹L$;
tÿ×‹D$Ç„$t  ÿÿÿÿ‹J‰‹…É…½  ‹L$¡ˆB;„¬  ÿ„Bé¡  ‹ˆB‹…Àuÿ¸B‰D$‹A‰‹Ex»   ;ÃÇ„$t  )   |U¿X   ‹M|h ÏÿC‹u|3À÷T$$R‰F8‰F<ˆF@ÿŒC‹ƒÄ‰NHN‹PÆFP ‹‰VLÿP‹ExCƒÇX;Ø~°‹IÿÓ‹‹Èÿ’    ‹=¼B‹5„B…À‰D$ ÇD$    ‚  h¸L$8ÿ×Æ„$t  *ÿÓ‹L$4j Q‹Èÿ’   ‹ğ‹D$4Æ„$t  )‹J‰‹…Éu‹ˆB‹L$4;
tÿ„B…ö·   ;ux®   hüŒ$    ÿ×Æ„$t  +ÿÓ‹Œ$œ   QŒ$Ä   Q‹Èÿ’ˆ   ‹M|¶PÆ„$x  ,VÁÿˆA‹„$À   Æ„$t  +‹J‰‹…Éu‹ˆB‹Œ$À   ;
tÿ„B‹„$œ   Æ„$t  )‹J‰‹…Éu¡ˆB‹Œ$œ   ;tÿ„B‹D$…Àth´L$ÿˆChüL$hÿ×Æ„$t  -ÿÓ‹L$dQŒ$ä   Q‹Èÿ’ˆ   P”$ğ   h°RÆ„$€  .ÿ$Ch°P„$ø   Æ„$ˆ  /Pÿ,CƒÄPL$Æ„$x  0ÿ@C‹„$ä   Æ„$t  /‹J‰‹…Éu‹ˆB‹Œ$ä   ;
t
‹5„BÿÖë‹5„B‹„$ì   Æ„$t  .‹J‰‹…Éu¡ˆB‹Œ$ì   ;tÿÖ‹„$à   Æ„$t  -‹J‰‹…Éu‹ˆB‹Œ$à   ;
tÿÖ‹D$dÆ„$t  )‹J‰‹…Éu¡ˆB‹L$d;tÿÖ‹t$F‰t$ÿÓ‹‹Èÿ’Ô   ;t$ Œ„ıÿÿ‹5„BD$Œ$Ğ   PhtQÿ$Ch¸T$$PRÆ„$Œ  1ÿ,CƒÄ‹„$Ğ   Æ„$t  3‹J‰‹…Éu¡ˆB‹Œ$Ğ   ;tÿÖh L$Dÿ×h L$<Æ„$x  4ÿ×h L$pÆ„$x  5ÿ×h Œ$ˆ   Æ„$x  6ÿ×Æ„$t  7ÿÓL$@‹QL$<QL$tQŒ$   QL$$jQU‹ÈÿRx‹„$„   Æ„$t  6‹J‰‹…Éu‹ˆB‹Œ$„   ;
tÿÖ‹D$lÆ„$t  5‹J‰‹…Éu¡ˆB‹L$l;tÿÖ‹D$8Æ„$t  4‹J‰‹…Éu‹ˆB‹L$8;
tÿÖ‹D$@Æ„$t  3‹J‰‹…Éu¡ˆB‹L$@;tÿÖ‹D$Æ„$t  )‹J‰‹…Éu‹ˆB‹L$;
tÿÖ‹D$Ç„$t  ÿÿÿÿ‹J‰‹…É…ª  ¡ˆB‹L$;„™  ÿÖé’  ‹=Iÿ×‹‹Èÿ’    …Àx  ‰D$‹Mx¸   ;È‰D$ŒË   ¾X   hlL$Tÿ¼BÇ„$t  8   ÿ×‹L$PQŒ$ô   Q‹Èÿ’ˆ   ‹U|PÖÆ„$x  9Rÿ„CŠØ‹„$ø   ƒÄÆ„$t  8‹J‰‹…Éu¡ˆB‹Œ$ğ   ;tÿ„B‹D$PÇ„$t  ÿÿÿÿ‹J‰‹…Éu‹ˆB‹L$P;
tÿ„B„Ûu‹D$‹Mx@ƒÆX;Á‰D$:ÿÿÿÿ×‹‹Èÿ’Ô   ‹D$H‰D$…ÿÿÿéu
  ‹=Iÿ×‹‹Èÿ’    …À†[
  ‹5¼BhdL$\ÿÖÇ„$t  :   ÿ×‹L$XQŒ$Ü   Q‹Èÿ’ˆ   T$PRÆ„$|  ;ÿ(IƒÄ‹„$Ø   ‹=„BÆ„$t  >‹J‰‹…Éu¡ˆB‹Œ$Ø   ;tÿ×‹D$XÆ„$t  =‹J‰‹…Éu‹ˆB‹L$X;
tÿ×D$jPŒ$D  ÿ€C‹|C‰Œ$<  ExŒ$<  PÆ„$x  ?ÿxC…¬   Œ$<  PÿtC‹=pC•ˆ   RŒ$@  ÿ×…   Œ$<  Pÿ×”   QŒ$@  ÿ×˜   Œ$<  Sÿ×•œ   Œ$<  Rÿ×…    Œ$<  Pÿ×¤   QŒ$@  ÿ×•¨   Œ$<  Rÿ×‹ËÿC‹øƒÉÿ3Àò®÷ÑIu‰…Œ   ë
Ç…Œ      ‹E|3Û;ÃÇEh   t ‹HüxühĞ9 QjXPè‹—  Wè—  ƒÄ‰]|‹}xG¿WÅ   Qè ˜  ƒÄ‰D$$;ÃÆ„$t  @thĞ9 hp‘ XWjXS‰8è˜  ŠE5Æ„$t  ?„À‰]|t
Ç…¬   è  ‹Íèd  „À…L  j ”$¸   h<Rè8±ÿÿ‹ø‹,ID$0WPÆ„$ˆ  AƒÅ(ÿÓƒÄŒ$    hÄ	  Q‹ÈÆ„$|  BÿlC‰D$”$ü   URÆ„$|  CÿÓƒÄŒ$  hÄ	  Q‹ÈÆ„$|  DÿlC‹Ø”$  Æ„$t  ERÿ0IP„$  h PÆ„$„  Fÿ$ChŒ$  PQÆ„$  Gÿ,CS”$,  PRÆ„$œ  Hÿ(ChdP„$4  Æ„$¤  IPÿ,C‹L$P”$,  QPRÆ„$´  Jÿ(CƒÄ@hPD$Æ„$|  KPÿ,CƒÄŒ$ø   Æ„$t  Vÿ¨BŒ$  Æ„$t  Uÿ¨BŒ$  Æ„$t  Tÿ¨BŒ$   Æ„$t  Sÿ¨BŒ$  Æ„$t  Rÿ¨BŒ$  Æ„$t  Qÿ¨BŒ$  Æ„$t  Pÿ¨BŒ$ü   Æ„$t  Oÿ¨BŒ$    Æ„$t  Nÿ¨B³ML$$ˆœ$t  ÿ¨Bh L$ ÿÖh L$4Æ„$x  WÿÖh L$0Æ„$x  XÿÖh L$Æ„$x  YÿÖÆ„$t  Zÿ4IL$‹QL$4QL$4QL$Qj L$(Qj ‹ÈÿRxL$Æ„$t  Yÿ¨BL$,Æ„$t  Xÿ¨BL$0Æ„$t  Wÿ¨BL$ˆœ$t  ÿ¨BÿTI…À„½   h L$ÿÖh L$0Æ„$x  [ÿÖh L$4Æ„$x  \ÿÖh L$ Æ„$x  ]ÿÖÆ„$t  ^ÿTIL$‹QL$0QL$8QL$(QL$$j Qj ‹ÈÿRxL$Æ„$t  ]ÿ¨BL$0Æ„$t  \ÿ¨BL$,Æ„$t  [ÿ¨BL$ˆœ$t  ÿ¨BhT$ URÿ,CWPD$4Æ„$ˆ  _Pÿ(CƒÄL$Æ„$t  aÿ¨BÿXI‹ğj j L$(‹>ÿCPjx‹ÎÿWXL$ ˆœ$t  ÿ¨BL$Æ„$t  Aÿ¨B‹„$´   Æ„$t  ?‹J‰‹…Éu‹ˆB‹Œ$´   ;
tÿ„BŒ$<  Æ„$t  =ÿhC‹D$Ç„$t  ÿÿÿÿ‹J‰‹…É…  ‹L$éÜòÿÿ‹5IÿÖ‹‹Èÿ’Ì   ‰D$ ÿÖ‹‹Èÿ’    …À†k  hüL$dÿ¼BÇ„$t  b   ÿÖ‹L$`QŒ$À   Q‹Èÿ’ˆ   T$PRÆ„$|  cÿ(IƒÄ‹„$¼   ‹5„BÆ„$t  f‹J‰‹…Éu¡ˆB‹Œ$¼   ;tÿÖ‹D$`Æ„$t  e‹J‰‹…Éu‹ˆB‹L$`;
tÿÖ‹D$ ‹5CÇD$$    ÇD$(    X‹ËÿÖ‹øƒÉÿ3Àò®÷ÑItp‹ËÿÖPÿt@İ\$(ƒÄL$hhdÿ¼B‹L$(‹T$$QD$lRL$PQ‹ÍÆ„$„  gÿ\I‹D$hÆ„$t  e‹J‰‹…Éu‹ˆB‹L$h;
tÿ„BL$ÿÖ‹L$$‹T$ ‹ø‹D$(PQJÿÖPWhàÿh@‹D$$ƒÄÇ„$t  ÿÿÿÿ‹J‰‹…É…à  ‹L$éñÿÿ‹5IÿÖ‹‹Èÿ’Ì   ‰D$ÿÖ‹‹Èÿ’    …À†­  hÜL$tÿ¼BÇ„$t  h   ÿÖ‹L$pj Q‹Èÿ’   ‹ğ‹D$pÇ„$t  ÿÿÿÿ‹J‰‹…Éu‹ˆB‹L$p;
tÿ„B‹D$‹CÇ„$        Ç„$¤       HÿÓ‹øƒÉÿ3Àò®÷ÑIt‹L$ƒÁÿÓPÿt@İœ$¤   ƒÄ‹”$¤   ‹„$    ‹L$RPVƒÁÿÓPh´ÿh@İ„$´   ¹   3À¼$,  ƒÄó«T$$‹Íf«Ùœ$  RÆ„$  ?‰´$  è.  ‹D$$f‹L$(‰„$"  ‹…„   ŠT$*‰„$.  @j"‰…„   „$  Pf‰Œ$.  ˆ”$0  è8†ÿÿ•È   ŠØ¹	   3À‹ú´$!  ó«ŠŒ$   ‹…À   ˆ
‹”$6  ½É   ¹   ˆœ$A  Pó¥‰•à   ˆë   è”  ƒÄ‹Œ$l  _^][d‰    Äh  Â ¼† € Ÿ… x œ Z jÿhé/d¡    Pd‰%    Q¡ˆBV‹ñ‹ ‰t$…Àuÿ¸B‰‹A‰‹„INÇD$    ‰ÿI‹L$ÇFH    ÇFL    ‹Æ^d‰    ƒÄÃVW‹|$‹ñ‹Gƒèt!Hu.GN(PQè   ƒÄ‹ÎWÿèH_^Â WF(RPè   ƒÄW‹ÎÿèH_^Â jÿh½0d¡    Pd‰%    ƒì<SU‹l$XV‹5,IWD$4UPÿÖƒÄ‹=lCL$0hÄ	  Q‹ÈÇD$\    ÿ×‹Ø‹T$\D$,RPÆD$\ÿÖƒÄL$(hÄ	  Q‹ÈÆD$\ÿ×‹ğT$$ÆD$TRÿ0IPD$(h PÆD$dÿ$ChL$0PQÆD$pÿ,CVT$8PRÆD$|ÿ(ChdPD$DÆ„$„   Pÿ,CSL$HPQÆ„$”   ÿ(CƒÄ@hÆD$X	PT$hRÿ,CƒÄ‹D$‹5„BÆD$T‹J‰‹…Éu¡ˆB‹L$;tÿÖ‹D$ÆD$T‹J‰‹…Éu‹ˆB‹L$;
tÿÖ‹D$ÆD$T‹J‰‹…Éu¡ˆB‹L$;tÿÖ‹D$ÆD$T‹J‰‹…Éu‹ˆB‹L$;
tÿÖ‹D$ ÆD$T‹J‰‹…Éu¡ˆB‹L$ ;tÿÖ‹D$$ÆD$T‹J‰‹…Éu‹ˆB‹L$$;
tÿÖ‹D$(ÆD$T‹J‰‹…Éu¡ˆB‹L$(;tÿÖ‹D$,ÆD$T‹J‰‹…Éu‹ˆB‹L$,;
tÿÖ‹D$0ÆD$T‹J‰‹…Éu¡ˆB‹L$0;tÿÖ‹D$4³ˆ\$T‹J‰‹…Éu‹ˆB‹L$4;
tÿÖ‹5¼Bh L$HÿÖh L$DÆD$XÿÖh L$@ÆD$XÿÖh L$<ÆD$XÿÖÆD$Tÿ4IL$D‹QL$DQL$DQL$DQL$pj Qj ‹ÈÿRxL$8ÆD$Tÿ¨BL$<ÆD$Tÿ¨BL$@ÆD$Tÿ¨BL$Dˆ\$Tÿ¨B‹=TIÿ×…À„¡   h L$<ÿÖh L$@ÆD$XÿÖh L$DÆD$XÿÖh L$HÆD$XÿÖÆD$Tÿ×L$8‹QL$@QL$HQL$PQL$pj Qj ‹ÈÿRxL$DÆD$Tÿ¨BL$@ÆD$Tÿ¨BL$<ÆD$Tÿ¨BL$8ˆ\$Tÿ¨B‹T$\hD$`RPÿ,CUL$XPQÆD$lÿ(CƒÄL$\ÆD$Tÿ¨BÿXI‹ğj j L$P‹>ÿCPjx‹ÎÿWXL$Hˆ\$Tÿ¨BL$`ÇD$Tÿÿÿÿÿ¨B‹L$L_^][d‰    ƒÄHÃjÿh1d¡    Pd‰%    ƒì<SU‹l$XV‹5,IWD$4UPÿÖƒÄ‹=lCL$0hÄ	  Q‹ÈÇD$\    ÿ×‹Ø‹T$\D$,RPÆD$\ÿÖƒÄL$(hÄ	  Q‹ÈÆD$\ÿ×‹ğT$$ÆD$TRÿ0IPD$(htPÆD$dÿ$ChL$0PQÆD$pÿ,CVT$8PRÆD$|ÿ(ChdPD$DÆ„$„   Pÿ,CSL$HPQÆ„$”   ÿ(CƒÄ@hÆD$X	PT$hRÿ,CƒÄ‹D$‹5„BÆD$T‹J‰‹…Éu¡ˆB‹L$;tÿÖ‹D$ÆD$T‹J‰‹…Éu‹ˆB‹L$;
tÿÖ‹D$ÆD$T‹J‰‹…Éu¡ˆB‹L$;tÿÖ‹D$ÆD$T‹J‰‹…Éu‹ˆB‹L$;
tÿÖ‹D$ ÆD$T‹J‰‹…Éu¡ˆB‹L$ ;tÿÖ‹D$$ÆD$T‹J‰‹…Éu‹ˆB‹L$$;
tÿÖ‹D$(ÆD$T‹J‰‹…Éu¡ˆB‹L$(;tÿÖ‹D$,ÆD$T‹J‰‹…Éu‹ˆB‹L$,;
tÿÖ‹D$0ÆD$T‹J‰‹…Éu¡ˆB‹L$0;tÿÖ‹D$4³ˆ\$T‹J‰‹…Éu‹ˆB‹L$4;
tÿÖ‹5¼Bh L$HÿÖh L$DÆD$XÿÖh L$@ÆD$XÿÖh L$<ÆD$XÿÖÆD$Tÿ4IL$D‹QL$DQL$DQL$DQL$pj Qj ‹ÈÿRxL$8ÆD$Tÿ¨BL$<ÆD$Tÿ¨BL$@ÆD$Tÿ¨BL$Dˆ\$Tÿ¨B‹=TIÿ×…À„¡   h L$<ÿÖh L$@ÆD$XÿÖh L$DÆD$XÿÖh L$HÆD$XÿÖÆD$Tÿ×L$8‹QL$@QL$HQL$PQL$pj Qj ‹ÈÿRxL$DÆD$Tÿ¨BL$@ÆD$Tÿ¨BL$<ÆD$Tÿ¨BL$8ˆ\$Tÿ¨B‹T$\hD$`RPÿ,CUL$XPQÆD$lÿ(CƒÄL$\ÆD$Tÿ¨BÿXI‹ğj j L$P‹>ÿCPj{‹ÎÿWXL$Hˆ\$Tÿ¨BL$`ÇD$Tÿÿÿÿÿ¨B‹L$L_^][d‰    ƒÄHÃ‹A8Vq8‹Îÿ…Àt+‹v…öt‹6‹D$‹NL^;È”ÀÂ ‹D$3ö‹NL^;È”ÀÂ 2À^Â VW‹ù‹G8w8‹Îÿ…Àv‹Îÿ˜Cj ‹Îÿ”C‹GX‹H‹Î‰GXÿ_^Ã‹T$SUVW‹úƒÉÿ3Àò®‹t$÷ÑI‹ş‹Ù‹L$‹éÁéó«‹Íƒáóª3É…Û~Š< tˆFA;Ë|ñ_^][Ãƒì$S‹Ù‹ClHƒø‡Á   ÿ$…¼œ ÇCl   [ƒÄ$ÃVW¹   3À|$j"ó«f«‹ƒ„   ÆD$d‰D$&@‰ƒ„   D$PÇD$    ÆD$èÊzÿÿ³È   ŠĞ¹	   3À‹şˆT$5ó«ŠL$‹D$*ˆ»É   ¹   t$ó¥‹‹À   ‰ƒà   Qˆ“ë   è2ƒ  ƒÄÇCl   _^[ƒÄ$ÃÇCl   [ƒÄ$Ã‹Ëè   [ƒÄ$ÃI ú› œ ¡œ ­œ ­œ U‹ìƒäøjÿhä2d¡    Pd‰%    ì¨   SUV‹ñWj‹  D$Pj"PQÇD$4    èº‚  ƒÄ…ÀŒ
  ‹0C‹=ÔHƒ~l…Á  €|$LÉ„¶  ‹ˆB‹…Àuÿ¸B‰D$‹(E‰(‹†¸   L$@Ç„$À       PhìQÿÓƒÄT$‹Îj RÿĞHhäL$<ÿ¼Bj „$œ   hÌPÆ„$Ì   ÿ×‹èj Œ$ˆ   hÄQÆ„$Ø   ÿ×N(”$ˆ   QPRÆ„$ä   ÿ(CUP„$à   Æ„$ì   Pÿ(CL$HT$xQPRÆ„$ü   ÿ(CPD$xPÆ„$  èôÿÿ‹„$Œ   ƒÄDÆ„$À   ‹J‰‹…Éu‹ˆB‹L$H;
tÿ„BŒ$´   Æ„$À   ÿ¨BL$pÆ„$À   ÿ¨BL$xÆ„$À   ÿ¨BŒ$˜   Æ„$À   ÿ¨BL$8Æ„$À    ÿ¨BŠF4„À„ø   j „$”   hÌPÿ×‹èj Œ$°   hÄQÆ„$Ø   ÿ×N(”$    QPRÆ„$ä   ÿ(CUP„$Ô   Æ„$ì   	Pÿ(CL$H”$°   QPRÆ„$ü   
ÿ(CƒÄ<P‹ÎÆ„$Ä   ÿäHŒ$€   Æ„$À   
ÿ¨BŒ$¨   Æ„$À   	ÿ¨BŒ$ˆ   Æ„$À   ÿ¨BŒ$    Æ„$À   ÿ¨BŒ$   Æ„$À    ÿ¨B‹D$ÇFl   Ç„$À   ÿÿÿÿ‹J‰‹…Éu¡ˆB‹L$;tÿ„B‹¸   ‹T$b‹-h@AQRh”ÿÕD$Xj"PèôvÿÿƒÄ„Àtjÿ@‹ˆB‹…Àuÿ¸B‰D$‹A‰‹T$LÇ„$À      âÿ   Bÿ=È   ‡¨  3ÉŠˆh§ ÿ$8§ ŠT$QD$ƒâRhTPé˜  ŠL$QT$ƒáQhTRé  ¿D$QPél  ÙD$QƒìL$İ$hTQÿÓƒÄé]  ¿T$QRD$hTPéC  ÙD$QƒìL$İ$hQÿÓƒÄé*  ‹T$U‹D$QRPL$hŒQÿÓƒÄé  ‹T$QD$RhTPéó  hlÿÕƒÄéé  ƒ~l„ß  ‹¸   AQh(ÿÕ‹ˆBƒÄ‹…Àuÿ¸B‰D$‹(E‰(‹†¸   L$@Æ„$À   PhäQÿÓƒÄT$‹ÎjRÿĞHhäL$Dÿ¼Bj „$ˆ   hÔPÆ„$Ì   ÿ×‹èj Œ$Œ   hÄQÆ„$Ø   ÿ×N(”$Œ   QPRÆ„$ä   ÿ(CUP„$Ü   Æ„$ì   Pÿ(CL$LT$`QPRÆ„$ü   ÿ(CP„$€   PÆ„$  èãïÿÿ‹D$tƒÄDÆ„$À   ‹J‰‹…Éu‹ˆB‹L$0;
tÿ„BŒ$°   Æ„$À   ÿ¨BL$tÆ„$À   ÿ¨BL$|Æ„$À   ÿ¨BŒ$„   Æ„$À   ÿ¨BL$@Æ„$À   ÿ¨BŠF4„À„ø   j „$°   hÔPÿ×‹èj Œ$´   hÄQÆ„$Ø   ÿ×N(”$´   QPRÆ„$ä   ÿ(CUP„$À   Æ„$ì   Pÿ(CL$L”$¼   QPRÆ„$ü   ÿ(CƒÄ<P‹ÎÆ„$Ä   ÿäHŒ$Œ   Æ„$À   ÿ¨BŒ$”   Æ„$À   ÿ¨BŒ$œ   Æ„$À   ÿ¨BŒ$¤   Æ„$À   ÿ¨BŒ$¬   Æ„$À   ÿ¨B‹D$ÇFl   Æ„$À   ‹J‰‹…Éu0¡ˆB‹L$;t#ÿ„BëhÀÿÕƒÄj L$hTQÿÓƒÄ‹ˆB‹…Àuÿ¸B‰D$‹B‰‹D$ML$PhTQÆ„$Ì   ÿÓT$ D$PRhœPÿ$ChL$XPQÆ„$ä   ÿ,CN(T$XQPRÆ„$ğ   ÿ(ChlPD$XÆ„$ø   Pÿ,CƒÄ<‹D$4Æ„$À    ‹J‰‹…Éu‹ˆB‹L$4;
t
‹-„BÿÕë‹-„B‹D$<Æ„$À   ‹J‰‹…Éu¡ˆB‹L$<;tÿÕ‹D$DÆ„$À   ‹J‰‹…Éu‹ˆB‹L$D;
tÿÕ‹-¼Bh L$0ÿÕh L$,Æ„$Ä   !ÿÕÆ„$À   "ÿIL$,‹QL$,QL$QL$QL$0jQV‹ÈÿRx‹D$(Æ„$À   !‹J‰‹…Éu‹ˆB‹L$(;
t
‹-„BÿÕë‹-„B‹D$,Æ„$À   ‹J‰‹…Éu¡ˆB‹L$,;tÿÕ‹D$$Æ„$À   ƒø2‹D$ ‹¨   J‰‹…Éu‹ˆB‹L$ ;
tÿÕ‹D$Æ