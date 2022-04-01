/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2021 Enscada 
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
#include "modbus_slave_db.h"
#include "modbus_slave_imp.h"
#include "stdlib.h"
#include "time64.h"

#define MAX_KEYLEN 256
#define MAX_COMMAND_SEND_TIME 60

#include <sys/stat.h>

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


void iec_call_exit_handler(int line, char* file, char* reason);

//   
//  Class constructor.   
//   
modbus_imp::modbus_imp(struct modbusContext* my_ctx):
fExit(false), general_interrogation(true), is_connected(false)
{   
	my_modbus_context.use_context = my_ctx->use_context;

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
		my_modbus_context.rtsOnTime = my_ctx->rtsOnTime;
		my_modbus_context.rtsOffTime = my_ctx->rtsOffTime;
	}

	if(my_modbus_context.use_context == TCP) 
	{
        ctx = modbus_new_tcp(my_modbus_context.modbus_server_address, atoi(my_modbus_context.modbus_server_port));
    } 
	else 
	{
        ctx = modbus_new_rtu(my_modbus_context.serial_device, my_modbus_context.baud, my_modbus_context.parity, my_modbus_context.data_bit, my_modbus_context.stop_bit, my_modbus_context.rtsOnTime, my_modbus_context.rtsOffTime);
    }

    if (ctx != NULL)
	{
		#define MAX_FIFO_SIZE 65535
		fifo_monitor_direction = fifo_open("fifo_local_monitor_dir_in_modbus_slave", MAX_FIFO_SIZE, iec_call_exit_handler);

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

		char fifo_control_name[150];
		strcpy(fifo_control_name,"fifo_global_control_direction");
		
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
		
		char fifo_monitor_name[150];
		strcpy(fifo_monitor_name,"fifo_global_monitor_direction");

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
	}
	else
	{
        fprintf(stderr, "Unable to allocate libmodbus context\n");
		fExit = 1;
		return;
    }

	modbus_set_debug(ctx, TRUE);

	mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, MODBUS_MAX_READ_BITS,
                                    MODBUS_MAX_READ_REGISTERS, MODBUS_MAX_READ_REGISTERS);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
		fExit = 1;
        return;
    }

    //modbus_set_error_recovery(ctx,(modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL));
}   
//   
//  Class destructor.   
//   
modbus_imp::~modbus_imp()  
{   
    // free resources   
	fExit = 1;
	///////////////////////////////////Middleware//////////////////////////////////////////////////
	ORTEDomainAppDestroy(domain);
    domain = NULL;
	////////////////////////////////////Middleware//////////////////////////////////////////////////
    return;   
}   

char err_msg[100];

int modbus_imp::RunServer(void)
{
	IT_IT("modbus_imp::RunServer");

	int rc;

	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	struct iec_item one_item;
	int i, len;
	const unsigned wait_limit_ms = 1;
	
    for(;;) {

		/////update slave memory with message riceived in monitor direction////////
		for(i = 0; (len = fifo_get(fifo_monitor_direction, (char*)&one_item, sizeof(struct iec_item), wait_limit_ms)) >= 0; i += 1)
		{ 
			fprintf(stderr,"Receiving %d th message\n", one_item.msg_id);
			fflush(stderr);

            //unsigned char* pt = (unsigned char*)&one_item;
			//for(j = 0; j < len; j++)
			//{ 
				//unsigned char c = *((unsigned char*)pt + j);
				//fprintf(stderr,"rx <--- 0x%02x-\n", c);
				//fflush(stderr);
				//IT_COMMENT1("fifo rx <--- 0x%02x-", c);
			//}

			rc = clearCrc((unsigned char *)&one_item, sizeof(struct iec_item));
			if(rc != 0)
			{
				sprintf(err_msg, "Message checksum error");
				iec_call_exit_handler(__LINE__,__FILE__,err_msg);
			}

			//Here update slave memory that is made by:

		    //int nb_bits; MODBUS_MAX_READ_BITS
			//int nb_input_bits; MODBUS_MAX_READ_BITS
			//int nb_input_registers; MODBUS_MAX_READ_REGISTERS
			//int nb_registers; MODBUS_MAX_READ_REGISTERS
			//uint8_t *tab_bits;
			//uint8_t *tab_input_bits;
			//uint16_t *tab_input_registers;
			//uint16_t *tab_registers;

			for(int rowNumber = 0; rowNumber < db_n_rows; rowNumber++)
			{
				if(Config_db[rowNumber].modbus_function_read == FC_READ_COILS) //0x01
				{
					if(Config_db[rowNumber].modbus_type == VT_BOOL)
					{
						if(one_item.iec_obj.ioa == Config_db[rowNumber].ioa_control_center)
						{
							int offset = Config_db[rowNumber].modbus_address;

							if(one_item.iec_type == M_SP_TB_1)
							{
								mb_mapping->tab_bits[offset] = one_item.iec_obj.o.type30.sp;
								break;
							}
						}
					}
				}
				else if(Config_db[rowNumber].modbus_function_read == FC_READ_DISCRETE_INPUTS) //0x02
				{
					if(Config_db[rowNumber].modbus_type == VT_BOOL)
					{
						if(one_item.iec_obj.ioa == Config_db[rowNumber].ioa_control_center)
						{
							int offset = Config_db[rowNumber].modbus_address;

							if(one_item.iec_type == M_SP_TB_1)
							{
								mb_mapping->tab_input_bits[offset] = one_item.iec_obj.o.type30.sp;
								break;
							}
						}
					}
				}
				else if(Config_db[rowNumber].modbus_function_read == FC_READ_HOLDING_REGISTERS) //0x03
				{
					if((Config_db[rowNumber].modbus_type == VT_I4) || 
					   (Config_db[rowNumber].modbus_type == VT_UI4)|| 
					   (Config_db[rowNumber].modbus_type == VT_R4) ||
					   (Config_db[rowNumber].modbus_type == VT_R4SWAP)
					   )
					{
						if(one_item.iec_obj.ioa == Config_db[rowNumber].ioa_control_center)
						{
							int offset = Config_db[rowNumber].modbus_address;

							if(one_item.iec_type == M_IT_TB_1)
							{
								mb_mapping->tab_registers[offset] = one_item.iec_obj.o.type37.counter;
								break;
							}

							if(one_item.iec_type == M_ME_TF_1)
							{
								memcpy(&(mb_mapping->tab_registers[offset]),&(one_item.iec_obj.o.type36.mv), sizeof(float));
								break;
							}
						}
					}
					else if(Config_db[rowNumber].modbus_type == VT_I2)
					{
						if(one_item.iec_obj.ioa == Config_db[rowNumber].ioa_control_center)
						{
							int offset = Config_db[rowNumber].modbus_address;
							
							if(one_item.iec_type == M_ME_TE_1)
							{
								mb_mapping->tab_registers[offset] = one_item.iec_obj.o.type35.mv;
								break;
							}
						}
					}
					else if(Config_db[rowNumber].modbus_type == VT_UI2)
					{
						if(one_item.iec_obj.ioa == Config_db[rowNumber].ioa_control_center)
						{
							int offset = Config_db[rowNumber].modbus_address;
							
							if(one_item.iec_type == M_ME_TQ_1)
							{
								mb_mapping->tab_registers[offset] = one_item.iec_obj.o.type153.mv;
								break;
							}
						}
					}
				}
				else if(Config_db[rowNumber].modbus_function_read == FC_READ_INPUT_REGISTERS) //0x04
				{
					if((Config_db[rowNumber].modbus_type == VT_I4) || 
					   (Config_db[rowNumber].modbus_type == VT_UI4)|| 
					   (Config_db[rowNumber].modbus_type == VT_R4) ||
					   (Config_db[rowNumber].modbus_type == VT_R4SWAP)
					   )
					{
						if(one_item.iec_obj.ioa == Config_db[rowNumber].ioa_control_center)
						{
							int offset = Config_db[rowNumber].modbus_address;

							if(one_item.iec_type == M_IT_TB_1)
							{
								mb_mapping->tab_input_registers[offset] = one_item.iec_obj.o.type37.counter;
								break;
							}

							if(one_item.iec_type == M_ME_TF_1)
							{
								memcpy(&(mb_mapping->tab_input_registers[offset]),&(one_item.iec_obj.o.type36.mv), sizeof(float));
								break;
							}
						}
					}
					else if(Config_db[rowNumber].modbus_type == VT_I2)
					{
						if(one_item.iec_obj.ioa == Config_db[rowNumber].ioa_control_center)
						{
							int offset = Config_db[rowNumber].modbus_address;
							
							if(one_item.iec_type == M_ME_TE_1)
							{
								mb_mapping->tab_input_registers[offset] = one_item.iec_obj.o.type35.mv;
								break;
							}
						}
					}
					else if(Config_db[rowNumber].modbus_type == VT_UI2)
					{
						if(one_item.iec_obj.ioa == Config_db[rowNumber].ioa_control_center)
						{
							int offset = Config_db[rowNumber].modbus_address;
							
							if(one_item.iec_type == M_ME_TQ_1)
							{
								mb_mapping->tab_input_registers[offset] = one_item.iec_obj.o.type153.mv;
								break;
							}
						}
					}
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
			}
		}
		
        rc = modbus_receive(ctx, query);
        if (rc >= 0) 
		{
            modbus_reply(ctx, query, rc, mb_mapping);
        } 
		else 
		{
            // Connection closed by the client or server
            break;
        }
    }

  	
	IT_EXIT;
	return 0;
}

int modbus_imp::Start(void)
{
	IT_IT("modbus_imp::Start");
	
	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA MODBUS slave Start\n");
	LogMessage(NULL, show_msg);

	if(fExit == 1)
	{
		return(1); //error
	}
	
	if(AddItems())
	{
		return(1); //error
	}

	if(my_modbus_context.use_context == TCP) 
	{
        socket = modbus_tcp_listen(ctx, 1);
        modbus_tcp_accept(ctx, &socket);
    } 
	else 
	{
        modbus_set_slave(ctx, 1);
        modbus_connect(ctx);
    }

	IT_EXIT;
    return(0);
}

int modbus_imp::Stop()
{
	IT_IT("modbus_imp::Stop");

	fprintf(stderr,"Entering Stop()\n");
	fflush(stderr);

	fifo_close(fifo_monitor_direction);

	/* Free the memory */
    //free(tab_rp_bits);
    //free(tab_rp_registers);
    modbus_mapping_free(mb_mapping);

	if(my_modbus_context.use_context == TCP) 
	{
        closesocket(socket);
    }
    
	/* Close the connection */
	modbus_close(ctx);
    modbus_free(ctx);
	
	// terminate server and it will clean up itself

	char show_msg[200];
	sprintf(show_msg, " IndigoSCADA MODBUS slave End\n");
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

uint64_t getTimeInMs()
{
	FILETIME ft;
	uint64_t now;

	static const uint64_t DIFF_TO_UNIXTIME = 11644473600000i64;

	GetSystemTimeAsFileTime(&ft);

	now = (LONGLONG)ft.dwLowDateTime + ((LONGLONG)(ft.dwHighDateTime) << 32i64);

	return (now / 10000i64) - DIFF_TO_UNIXTIME;
}

void modbus_imp::get_local_host_time(struct cp56time2a* time)
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

void modbus_imp::get_utc_host_time(struct cp56time2a* time)
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

int64_t modbus_imp::epoch_from_cp56time2a(const struct cp56time2a* time)
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

#include <signal.h>

char* get_date_time()
{
	static char sz[128];
	time_t t = time(NULL);
	struct tm *ptm = localtime(&t);
	
	strftime(sz, sizeof(sz)-2, "%m/%d/%y %H:%M:%S", ptm);

	strcat(sz, "|");
	return sz;
}

void iec_call_exit_handler(int line, char* file, char* reason)
{
	FILE* fp;
	char program_path[_MAX_PATH];
	char log_file[_MAX_FNAME+_MAX_PATH];
	IT_IT("iec_call_exit_handler");

	program_path[0] = '\0';
#ifdef WIN32
	if(GetModuleFileName(NULL, program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '\\')) = '\0';        // Strip \\bin off path
    }
#elif __unix__
	if(getcwd(program_path, _MAX_PATH))
	{
		*(strrchr(program_path, '/')) = '\0';        // Strip \\filename.exe off path
		*(strrchr(program_path, '/')) = '\0';        // Strip \\bin off path
    }
#endif

	strcpy(log_file, program_path);

#ifdef WIN32
	strcat(log_file, "\\logs\\modbus_slave.log");
#elif __unix__
	strcat(log_file, "/logs/modbus_slave.log");	
#endif

	fp = fopen(log_file, "a");

	if(fp)
	{
		if(line && file && reason)
		{
			fprintf(fp, "PID:%d time:%s exit process at line: %d, file %s, reason:%s\n", GetCurrentProcessId, get_date_time(), line, file, reason);
		}
		else if(line && file)
		{
			fprintf(fp, "PID:%d time:%s exit process at line: %d, file %s\n", GetCurrentProcessId, get_date_time(), line, file);
		}
		else if(reason)
		{
			fprintf(fp, "PID:%d time:%s exit process for reason %s\n", GetCurrentProcessId, get_date_time(), reason);
		}

		fflush(fp);
		fclose(fp);
	}

	raise(SIGABRT);   //raise abort signal which in turn starts automatically a separete thread and call iec104SignalHandler

	IT_EXIT;
}

