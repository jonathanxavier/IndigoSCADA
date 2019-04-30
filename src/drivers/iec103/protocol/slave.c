/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2011 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "queue.h"
#include "clear_crc_eight.h"
#include "iec103.h"
#include "iecserial.h"
#include "iec103_types.h"
#include "itrace.h"
#include <signal.h>
#include <time.h>
#include "link_unbalance_slave.h"
#include "getopt.h"
#include <process.h>

#ifndef WIN32
#include "portable.h"
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#endif

#include "serial.h"

//void PipeWorker(void* pParam);

#define SUPPLIER "@ enscada.com"
#define APPLICATION "iec103slave.exe"

int IsSingleInstance(const char* name)
{
   char mutex_name[200];

#ifdef WIN32
   HANDLE hMutex;
   strcpy(mutex_name, ""APPLICATION"");
   strcat(mutex_name, name);

   hMutex = CreateMutex (NULL, TRUE, mutex_name);

   if (GetLastError() == ERROR_ALREADY_EXISTS)
   {
      if(hMutex)
	  {
			CloseHandle(hMutex);
	  }
	  return 0;
   }
#elif __unix__
                sem_t *hSem = NULL;
                strcpy(mutex_name, ""APPLICATION"");
                strcat(mutex_name, name);

	hSem = sem_open(mutex_name, O_CREAT | O_EXCL);
	
	if (hSem == SEM_FAILED && errno == EEXIST) {
	
		return 0;
	}
	
#endif
   return 1;
}


#define RUNTIME_USAGE "Run time usage: %s -a link_address -n serial_COM_port -s baud_rate -l line_number -m read_timeout_ms -p rts_on_time_ms -q rts_off_time_ms -f log_file_name\n"

void usage(char** argv)
{
	fprintf(stderr, RUNTIME_USAGE, argv[0]);
	fflush(stderr);
}

struct args{
	char line_number[80];
};

int main(int argc, char **argv)
{
	char log_file_name[250];
	char version[100];
	int c;
	char fifo_monitor_direction_name[70];
	char fifo_control_direction_name[70];
	char line_number[20] ;
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	int rc;
	char LinkAddress[80];
	char bits_per_second[80];
	char ReadTimeOutInMilliseconds[80];
	char RTSOnTime[80];
	char RTSOffTime[80];
    struct args arg;
    int baud_rate = 9600;
	
	IT_IT("main SLAVE");

	//version control///////////////////////////////////////////////////////////////
	sprintf(version, ""APPLICATION" - Built on %s %s %s",__DATE__,__TIME__,SUPPLIER);
	fprintf(stderr, "%s\n", version);
	fflush(stderr);
	IT_COMMENT(version);
	////////////////////////////////////////////////////////////////////////////////

	iec_protocol_log_stream = NULL;
	slave_port_name[0] = '\0';
    strcpy(bits_per_second, "9600");
	fifo_monitor_direction_name[0] = '\0';
	fifo_control_direction_name[0] = '\0';
	line_number[0] = '\0';
	LinkAddress[0] = '\0';
	ReadTimeOutInMilliseconds[0] = '\0';
	RTSOnTime[0] = '\0';
	RTSOffTime[0] = '\0';
	
	while( ( c = getopt ( argc, argv, "a:n:s:l:f:m:p:q:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
				strcpy(LinkAddress, optarg);
			break;
			case 'n' :
				strcpy(slave_port_name, optarg);
			break;
            case 's' :
                strcpy(bits_per_second, optarg);
            break;
			case 'l':
				strcpy(line_number, optarg);
			break;
			case 'm' :
				strcpy(ReadTimeOutInMilliseconds, optarg);
			break;
			case 'p':
				strcpy(RTSOnTime, optarg);
			break;
			case 'q':
				strcpy(RTSOffTime, optarg);
			break;
			case 'f' :
			{
				struct tm *ptm;
				time_t t;
				char log_time_stamp[100];
				char program_path[_MAX_PATH];

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

				t = time(NULL);
				ptm = localtime(&t);

				sprintf(log_time_stamp, "_%d_%d_%d_%d_%d_%d",
				ptm->tm_year + 1900,
				ptm->tm_mon + 1,
				ptm->tm_mday,
				ptm->tm_hour,
				ptm->tm_min,
				ptm->tm_sec);
				
				strcpy(iec_optional_log_file_name, optarg);
				strcpy(log_file_name, program_path);
                #ifdef WIN32
                strcat(log_file_name, "\\logs\\");
                #elif __unix__
                strcat(log_file_name, "/logs/");
                #endif
				strcat(log_file_name, iec_optional_log_file_name);
				strcat(log_file_name, log_time_stamp);
				strcat(log_file_name, ".log");

				//Reassign "stderr" to log file
				iec_protocol_log_stream = freopen(log_file_name, "w", stderr);

				fprintf(stderr, "%s\n", version);
	            fflush(stderr);
			}
			break;
			case '?' :
				fprintf(stderr, RUNTIME_USAGE, argv[0]);
				fflush(stderr);
				exit( 0 );
		}
	}

	if(argc < 2) 
	{
		usage(argv);
		exit(-1);
    }

	strcpy(fifo_monitor_direction_name, "fifo_monitor_direction_slave");
	strcpy(fifo_control_direction_name, "fifo_control_direction_slave");
	
	if(strlen(line_number) > 0)
	{
		strcat(fifo_monitor_direction_name, line_number);
		strcat(fifo_control_direction_name, line_number);
	}

//    strcat(fifo_control_direction_name, "iec103");
//    strcat(fifo_monitor_direction_name, "iec103");


	if(strlen(slave_port_name) == 0)
	{
		fprintf(stderr,"Serial port not known\n");
		fflush(stderr);
		if(iec_protocol_log_stream)
			fclose(iec_protocol_log_stream);
		return EXIT_FAILURE;
	}
    
	gl_link_address = atoi(LinkAddress);
    gl_common_address_of_asdu = gl_link_address;
	gl_slave_polling_time_in_milliseconds = 10; //ms

	if(strlen(ReadTimeOutInMilliseconds) > 0)
		gl_read_timeout_ms = atoi(ReadTimeOutInMilliseconds);

	if(strlen(RTSOnTime) > 0)
		gl_rtsOnTime = atoi(RTSOnTime);

	if(strlen(RTSOffTime) > 0)
		gl_rtsOffTime = atoi(RTSOffTime);

	if(!IsSingleInstance(line_number))
	{
		fprintf(stderr,"Another instance of iec103slave for line %s is already running!", line_number);
		fflush(stderr);
		if(iec_protocol_log_stream)
			fclose(iec_protocol_log_stream);
		return EXIT_FAILURE;
	}

	#ifdef WIN32
	strcpy(NewConsoleTitle, "iec103slave LINK ");
	strcat(NewConsoleTitle, LinkAddress);
	strcat(NewConsoleTitle, " LINE ");
	strcat(NewConsoleTitle, line_number);
	strcat(NewConsoleTitle, "  ");
	strcat(NewConsoleTitle, slave_port_name);
    strcat(NewConsoleTitle, " baud rate ");
    strcat(NewConsoleTitle, bits_per_second);
	strcat(NewConsoleTitle, " read timeout ");
	strcat(NewConsoleTitle, ReadTimeOutInMilliseconds);

	if(strlen(RTSOnTime) > 0)
	{
		strcat(NewConsoleTitle, " rts on time ");
		strcat(NewConsoleTitle, RTSOnTime);
	}

	if(strlen(RTSOffTime) > 0)
	{
		strcat(NewConsoleTitle, " rts off time ");
		strcat(NewConsoleTitle, RTSOffTime);
	}

	fprintf(stderr, "%s\n", NewConsoleTitle);
	fflush(stderr);
	
	if(!IsSingleInstance(NewConsoleTitle))
	{
		fprintf(stderr,"Another instance is already running\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	if((rc = GetConsoleTitle(OldConsoleTitle, sizeof(OldConsoleTitle))) > 0)
	{
		SetConsoleTitle(NewConsoleTitle);
	}
	#endif

    strcpy(arg.line_number, line_number);

	//if(_beginthread(PipeWorker, 0, (void*)&arg) == -1)
	//{
	//	long nError = GetLastError();
//
//		fprintf(stderr,"PipeWorker _beginthread failed, error code = %d", nError);
//		fflush(stderr);
//		return EXIT_FAILURE;	
//	}

	signal (SIGTERM, iec103SignalHandler);
#ifdef WIN32
	signal (SIGBREAK, iec103SignalHandler);
#endif
	signal (SIGABRT, iec103SignalHandler);
	signal (SIGFPE, iec103SignalHandler);
	signal (SIGSEGV, iec103SignalHandler);
	signal (SIGILL, iec103SignalHandler);
	signal (SIGINT, iec103SignalHandler);

//	if(init_fifos(fifo_monitor_direction_name, fifo_control_direction_name, NULL))
//	{
//		if(iec_protocol_log_stream)
//			fclose(iec_protocol_log_stream);
//		return EXIT_FAILURE;	
//	}
	
    baud_rate = atoi(bits_per_second);

	iecserial_connect(slave_port_name, baud_rate);
	
	State_process();

	Sleep(2000); //Wait iec103SignalHandler to operate

	if(iec_protocol_log_stream)
			fclose(iec_protocol_log_stream);
	IT_EXIT;
	return EXIT_SUCCESS;
}



