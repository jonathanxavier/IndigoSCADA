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

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "clear_crc_eight.h"
#include "process.h"
#include "iec101_types.h"
#include "iec_101_item.h"
#include "iec101.h"
#include <stdlib.h>
#include <errno.h>
#include "iec101_imp_srv.h"
#include <signal.h>

#define SUPPLIER "@ enscada.com"
#define APPLICATION "iec101slave.exe"

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

#include "getopt.h"

#define RUNTIME_USAGE "Run time usage: %s -a link_address -c CADSU -n serial_COM_port -s baud_rate -l line_number -m read_timeout_ms -f log_file_name\n"

void usage(char** argv)
{
	fprintf(stderr, RUNTIME_USAGE, argv[0]);
	fflush(stderr);
}

int main( int argc, char **argv )
{
	char log_file_name[250];
	char version[100];
	int c;
	char fifo_monitor_direction_name[70];
	char fifo_control_direction_name[70];
	char line_number[20];
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	char CommonAddressOfAsdu[80];
	char LinkAddress[80];
	char ReadTimeOutInMilliseconds[80];
    char bits_per_second[80];
    int baud_rate = 9600;
	char slave_port_name[80];
	int rc;
	
	IT_IT("main SLAVE");

	//version control///////////////////////////////////////////////////////////////
	sprintf(version, ""APPLICATION" - Built on %s %s %s",__DATE__,__TIME__,SUPPLIER);
	fprintf(stderr, "%s\n", version);
	fflush(stderr);
	SYSTEMTIME oT;
	::GetLocalTime(&oT);
	fprintf(stderr,"%02d/%02d/%04d, %02d:%02d:%02d Starting ... %s\n",oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,APPLICATION); 
	fflush(stderr);
	////////////////////////////////////////////////////////////////////////////////

	iec_protocol_log_stream = NULL;
	slave_port_name[0] = '\0';
    strcpy(bits_per_second, "9600");
	fifo_monitor_direction_name[0] = '\0';
	fifo_control_direction_name[0] = '\0';
	line_number[0] = '\0';
	CommonAddressOfAsdu[0] = '\0';
	LinkAddress[0] = '\0';
	ReadTimeOutInMilliseconds[0] = '\0';

	while( ( c = getopt ( argc, argv, "a:c:n:s:l:f:m:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
				strcpy(LinkAddress, optarg);
			break;
			case 'c' :
				strcpy(CommonAddressOfAsdu, optarg);
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

	if(strlen(slave_port_name) == 0)
	{
		fprintf(stderr,"Serial port not known\n");
		fflush(stderr);
		if(iec_protocol_log_stream)
			fclose(iec_protocol_log_stream);
		return EXIT_FAILURE;
	}

    if(strlen(CommonAddressOfAsdu) == 0)
	{
		fprintf(stderr,"Common Address Of Asdu is not known\n");
		fflush(stderr);
		if(iec_protocol_log_stream)
			fclose(iec_protocol_log_stream);
		return EXIT_FAILURE;
	}

	gl_common_address_of_asdu = atoi(CommonAddressOfAsdu);
	gl_link_address = atoi(LinkAddress);
	gl_slave_polling_time_in_milliseconds = 10; //ms
	gl_read_timeout_ms = atoi(ReadTimeOutInMilliseconds);

	if(!IsSingleInstance(line_number))
	{
		fprintf(stderr,"Another instance of iec101slave for line %s is already running!", line_number);
		fflush(stderr);
		if(iec_protocol_log_stream)
			fclose(iec_protocol_log_stream);
		return EXIT_FAILURE;
	}

	#ifdef WIN32
	strcpy(NewConsoleTitle, "iec101slave LINK ");
	strcat(NewConsoleTitle, LinkAddress);
	strcat(NewConsoleTitle, " CASDU ");
	strcat(NewConsoleTitle, CommonAddressOfAsdu);
	strcat(NewConsoleTitle, " LINE ");
	strcat(NewConsoleTitle, line_number);
	strcat(NewConsoleTitle, "  ");
	strcat(NewConsoleTitle, slave_port_name);
    strcat(NewConsoleTitle, " baud rate ");
    strcat(NewConsoleTitle, bits_per_second);
	strcat(NewConsoleTitle, " read timeout ");
	strcat(NewConsoleTitle, ReadTimeOutInMilliseconds);

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

	signal (SIGTERM, iec101SignalHandler);
#ifdef WIN32
	signal (SIGBREAK, iec101SignalHandler);
#endif
	signal (SIGABRT, iec101SignalHandler);
	signal (SIGFPE, iec101SignalHandler);
	signal (SIGSEGV, iec101SignalHandler);
	signal (SIGILL, iec101SignalHandler);
	signal (SIGINT, iec101SignalHandler);

	baud_rate = atoi(bits_per_second);
	
	struct iec101Context my_ctx;
	
	//iec101 RTU
	strcpy(my_ctx.slave_port_name, slave_port_name);
	my_ctx.baud_rate = baud_rate;
		
	iec101_imp* po = new iec101_imp(&my_ctx);

	if(po == NULL)
	{
		return EXIT_FAILURE;
	}

	rc = po->Start();

	if(rc)
	{
		return EXIT_FAILURE;
	}

	rc = po->Run();

	if(rc)
	{
		return EXIT_FAILURE;
	}

	po->Stop();

	if(po)
	{
		delete po;
	}

	return 0;
}

