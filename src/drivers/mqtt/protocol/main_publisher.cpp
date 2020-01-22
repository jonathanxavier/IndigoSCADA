/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#define STRICT
#define VC_EXTRALEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <assert.h>
#include <errno.h>
#include "iec104types.h"
#include "iec_item.h"
#include <signal.h>
#include "getopt.h"
#include "itrace.h"
#include "mqtt_client_app_publisher.h"
#include "clear_crc_eight.h"

#define SUPPLIER "@ enscada.com"
#define APPLICATION "mqtt_client_publisher.exe"

int IsSingleInstance(char *name)
{
   HANDLE hMutex;
   char mutex_name[50];

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
   return 1;
}

///////////////////////////////////////////////
#include "process.h"
struct args{
	char line_number[80];
};

int gl_timeout_connection_with_parent;

void PipeWorker(void* pParam);
///////////////////////////////////////////////

char *optarg;

#define RUNTIME_USAGE "Run time usage: %s -a broker_host_name\
 -p subscribe_topic_name -q user_name -r password\
 -l line_number -t port\n"

void usage(char** argv)
{
	fprintf(stderr, RUNTIME_USAGE, argv[0]);
	fflush(stderr);
}

int main(int argc, char **argv)
{
	char version[100];
	int c;
	char BrokerAddress[80];
	char SubscribeTopicName[80];
	char UserName[80];
	char Password[80];
	int Port;
	char ClientID[80];

	DWORD len;
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	char line_number[80];
			
	IT_IT("main MQTT CLIENT");

	//version control///////////////////////////////////////////////////////////////
	sprintf(version, ""APPLICATION" - Built on %s %s %s",__DATE__,__TIME__,SUPPLIER);
	fprintf(stderr, "%s\n", version);
	fflush(stderr);
	IT_COMMENT(version);
	////////////////////////////////////////////////////////////////////////////////

	BrokerAddress[0] = '\0';
	SubscribeTopicName[0] = '\0';
	UserName[0] = '\0';
	Password[0] = '\0';
	Port = 0;
	ClientID[0] = '\0';
	OldConsoleTitle[0] = '\0';
	NewConsoleTitle[0] = '\0';
	line_number[0] = '\0';

	while( ( c = getopt ( argc, argv, "a:p:l:q:r:t:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
				strcpy(BrokerAddress, optarg);
				strcat(NewConsoleTitle, optarg);
				strcat(NewConsoleTitle, "   ");
			break;
			case 'p' :
				strcpy(SubscribeTopicName, optarg);
				strcat(NewConsoleTitle, optarg);
				strcat(NewConsoleTitle, "   ");
			break;
			case 'l' :
				strcpy(line_number, optarg);
				strcat(NewConsoleTitle, optarg);
				strcat(NewConsoleTitle, "   ");
			break;
			case 'q' :
				strcpy(UserName, optarg);
				strcat(NewConsoleTitle, optarg);
				strcat(NewConsoleTitle, "   ");
			break;
			case 'r' :
				strcpy(Password, optarg);
				strcat(NewConsoleTitle, "xxxx");
				strcat(NewConsoleTitle, "   ");
			break;
			case 't' :
				Port = atoi(optarg);
				strcat(NewConsoleTitle, optarg);
				strcat(NewConsoleTitle, "   ");
			break;
			case '?' :
				fprintf(stderr, RUNTIME_USAGE, argv[0]);
				fflush(stderr);
				IT_EXIT;
				return EXIT_FAILURE;
		}
	}

	if(argc < 2) 
	{
		usage(argv);
		IT_EXIT;
		return EXIT_FAILURE;
    }

	if(strlen(BrokerAddress) == 0)
	{
		fprintf(stderr,"MQTT broker IP address not known\n");
		fflush(stderr);
		IT_EXIT;
		return EXIT_FAILURE;
	}

	if((len = GetConsoleTitle(OldConsoleTitle, sizeof(OldConsoleTitle))) > 0)
	{
		SetConsoleTitle(NewConsoleTitle);
	}

	if(strlen(line_number) > 0)
	{
		if(!IsSingleInstance(line_number))
		{
			fprintf(stderr,"Another instance is already running\n");
			fflush(stderr);
			IT_EXIT;
			return EXIT_FAILURE;
		}
	}
	
	//Alloc MQTT client class and start
	MQTT_client_imp_publisher* po = new MQTT_client_imp_publisher(BrokerAddress, line_number);

	strcpy(ClientID, "IndigoSCADAMQTTClientPublisher");
	strcat(ClientID, line_number);

	// connect to a MQTT broker
	int nRet = po->MQTTStart(SubscribeTopicName, UserName, Password, Port, ClientID);
	
	if(nRet)
	{
		po->MQTTStop();
		IT_EXIT;
		return EXIT_FAILURE;
	}

	nRet = po->AddItems();

    po->mqttCtx.parent_class = po;

	if(nRet)
	{
		po->MQTTStop();
		IT_EXIT;
		return EXIT_FAILURE;
	}

	po->Update();

	po->MQTTStop();

	//if(po)
	//{
		//delete po;
	//}

	IT_EXIT;
	return EXIT_SUCCESS;
}

