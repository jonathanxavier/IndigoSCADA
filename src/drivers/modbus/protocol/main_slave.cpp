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

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <winsock2.h>
#include "clear_crc_eight.h"
#include "iec104types.h"
#include "iec_item.h"
#include "process.h"
#include "modbus.h"
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include "modbus_slave_imp.h"

#define SUPPLIER "@ enscada.com"
#define APPLICATION "modbus_slave.exe"

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

struct args{
	char line_number[80];
};

#define RUNTIME_USAGE "Run time usage: %s -a server IP address -p server TCP port -d serial device -b serial baud -c serial databits -e serial stopbit -f serial parity\n"

void usage(char** argv)
{
	fprintf(stderr, RUNTIME_USAGE, argv[0]);
	fflush(stderr);
}

int main( int argc, char **argv )
{
	char version[100];
	/////////MODBUS TCP/////////////////
	char modbusServerAddress[80];
	char modbusServerPort[80];
	/////////MODBUS RTU/////////////////
	char serial_device[40]; /* "/dev/ttyS0" or "\\\\.\\COM10" */
	char baud[20];/* Bauds: 9600, 19200, 57600, 115200, etc */
    char data_bit[5];/* Data bit, eg. 8 */
    char stop_bit[5];/* Stop bit, eg. 1 */
    char parity[5];/* Parity: 'N', 'O', 'E' */
	

	char line_number[80];
	char polling_time[80];
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	int  c, rc;
	unsigned long pollingTime = 1000;
	int use_context = TCP;
	
	//TCP
	modbusServerAddress[0] = '\0';
	modbusServerPort[0] = '\0';
	//RTU
	serial_device[0] = '\0';
	baud[0] = '\0';
    data_bit[0] = '\0';
    stop_bit[0] = '\0';
    parity[0] = '\0';
	//
	line_number[0] = '\0';
	polling_time[0] = '\0';

	//version control///////////////////////////////////////////////////////////////
	sprintf(version, ""APPLICATION" - Built on %s %s %s",__DATE__,__TIME__,SUPPLIER);
	fprintf(stderr, "%s\n", version);
	fflush(stderr);
	SYSTEMTIME oT;
	::GetLocalTime(&oT);
	fprintf(stderr,"%02d/%02d/%04d, %02d:%02d:%02d Starting ... %s\n",oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,APPLICATION); 
	fflush(stderr);
	////////////////////////////////////////////////////////////////////////////////

	while( ( c = getopt ( argc, argv, "a:b:c:d:e:f:p:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
			strcpy(modbusServerAddress, optarg);
			break;
			case 'b' :
			strcpy(baud, optarg);
			break;
			case 'c' :
			strcpy(data_bit, optarg);
			break;
			case 'd' :
			strcpy(serial_device, optarg);
			break;
			case 'e' :
			strcpy(stop_bit, optarg);
			break;
			case 'f' :
			strcpy(parity, optarg);
			break;
			case 'p' :
			strcpy(modbusServerPort, optarg);
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
	
	if(strlen(modbusServerAddress) > 0 && strlen(modbusServerPort) > 0)
	{
		strcpy(NewConsoleTitle, "MODBUS TCP address ");
		strcat(NewConsoleTitle, modbusServerAddress);
		strcat(NewConsoleTitle, " PORT ");
		strcat(NewConsoleTitle, modbusServerPort);
	
		use_context = TCP;
	}
	else
	{
		strcpy(NewConsoleTitle, "MODBUS RTU device ");
		strcat(NewConsoleTitle, serial_device);
		strcat(NewConsoleTitle, " BAUD ");
		strcat(NewConsoleTitle, baud);

		strcat(NewConsoleTitle, " DATA BITS ");
		strcat(NewConsoleTitle, data_bit);

		strcat(NewConsoleTitle, " STOP BIT ");
		strcat(NewConsoleTitle, stop_bit);

		strcat(NewConsoleTitle, " PARITY ");
		strcat(NewConsoleTitle, parity);

		use_context = RTU;
	}

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

	struct modbusContext my_ctx;

	my_ctx.use_context = use_context;
	
	if(use_context == TCP)
	{
		//MODBUS TCP
		strcpy(my_ctx.modbus_server_address, modbusServerAddress); 
		strcpy(my_ctx.modbus_server_port, modbusServerPort); 
	}
	else if(use_context == RTU)
	{
		//MODBUS RTU
		strcpy(my_ctx.serial_device, serial_device);
		my_ctx.baud = atoi(baud);
		my_ctx.data_bit = atoi(data_bit);
		my_ctx.stop_bit = atoi(stop_bit);
		my_ctx.parity = parity[0];
	}
	
	modbus_imp* po = new modbus_imp(&my_ctx);

	if(po == NULL)
	{
		return EXIT_FAILURE;
	}

	rc = po->Start();

	if(rc)
	{
		return EXIT_FAILURE;
	}

	rc = po->RunServer();

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

