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
#include <winsock2.h>
#include "clear_crc_eight.h"
#include "process.h"
#include "iec104_types.h"
#include "iec_item.h"
#include "iec104.h"
#include <stdlib.h>
#include <errno.h>
#include "iec104_imp_srv.h"

#define SUPPLIER "@ enscada.com"
#define APPLICATION "iec104slave.exe"

int IsSingleInstance(const char* name)
{
   HANDLE hMutex;
   char mutex_name[200];

   strcpy(mutex_name, ""APPLICATION"");
   strcat(mutex_name, name);

   hMutex = CreateMutex(NULL, TRUE, mutex_name);

   if(GetLastError() == ERROR_ALREADY_EXISTS)
   {
      if(hMutex)
	  {
		 CloseHandle(hMutex);
	  }

	  return 0;
   }

   return 1;
}

#include "getopt.h"

#define RUNTIME_USAGE "Run time usage: %s -a server IP address -c CASDU -p server TCP port -f log_file_name.txt\n"

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
	u_short n_iec104server_port = 0;
	char CommonAddressOfAsdu[80];
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	char iec104ServerAddress[80];
	char iec104ServerPort[80];

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
	iec104ServerAddress[0] = '\0';
   	iec104ServerPort[0] = '\0';
	CommonAddressOfAsdu[0] = '\0';
	fifo_monitor_direction_name[0] = '\0';
	fifo_control_direction_name[0] = '\0';

	while( ( c = getopt ( argc, argv, "a:c:f:p:l:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
				strcpy(iec104ServerAddress, optarg);
			break;
			case 'c' :
				strcpy(CommonAddressOfAsdu, optarg);
			break;
			case 'p' :
				strcpy(iec104ServerPort, optarg);
			break;
			case 'f' :
			{
				struct tm *ptm;
				time_t t;
				char log_time_stamp[100];

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
				strcpy(log_file_name, iec_optional_log_file_name);
				strcat(log_file_name, log_time_stamp);
				strcat(log_file_name, ".log");

				//Reassign "stderr" to log file
				iec_protocol_log_stream = freopen(log_file_name, "w", stderr);
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

	if(strlen(iec104ServerAddress) == 0)
	{
		fprintf(stderr,"Server IP address not known\n");
		fflush(stderr);
		if(iec_protocol_log_stream)
			fclose(iec_protocol_log_stream);
		return EXIT_FAILURE;
	}

   	if(strlen(iec104ServerPort) == 0)
	{
		fprintf(stderr,"Server TCP port not known\n");
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

	common_address_of_asdu = atoi(CommonAddressOfAsdu);

//	if(!IsSingleInstance(line_number))
//	{
//		fprintf(stderr,"Another instance of iec104master for line %s is already running!", line_number);
//		fflush(stderr);
//		if(iec_protocol_log_stream)
//			fclose(iec_protocol_log_stream);
//		return EXIT_FAILURE;
//	}

	#ifdef WIN32
	strcpy(NewConsoleTitle, "iec104slave IP ");
	strcat(NewConsoleTitle, iec104ServerAddress);
	strcat(NewConsoleTitle, " PORT ");
	strcat(NewConsoleTitle, iec104ServerPort);
	strcat(NewConsoleTitle, " CASDU ");
	strcat(NewConsoleTitle, CommonAddressOfAsdu);

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
  
/*
	signal (SIGTERM, iec104SignalHandler);
#ifdef WIN32
	signal (SIGBREAK, iec104SignalHandler);
#endif
	signal (SIGABRT, iec104SignalHandler);
	signal (SIGFPE, iec104SignalHandler);
	signal (SIGSEGV, iec104SignalHandler);
	signal (SIGILL, iec104SignalHandler);
	signal (SIGINT, iec104SignalHandler);
*/
	struct iec104Context my_ctx;
	
	//iec104 RTU
	strcpy(my_ctx.iec104ServerAddress, iec104ServerAddress);
	strcpy(my_ctx.iec104ServerPort, iec104ServerPort);
		
	iec104_imp* po = new iec104_imp(&my_ctx);

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

