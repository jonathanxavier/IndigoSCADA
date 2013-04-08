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

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <winsock2.h>
#include "clear_crc_eight.h"
#include "iec104types.h"
#include "iec_item.h"
#include "process.h"
#include "mms_client_connection.h"
#include "mms_types.h"

#define SUPPLIER "@ enscada.com"
#define APPLICATION "iec61850client.exe"

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

///////////////////////////////////////////////
struct args{
	char line_number[80];
};

int gl_timeout_connection_with_parent;

void PipeWorker(void* pParam);
///////////////////////////////////////////////

#define RUNTIME_USAGE "Run time usage: %s -a server IP address -p server TCP port -l\
 line number -t polling time\n"

void usage(char** argv)
{
	fprintf(stderr, RUNTIME_USAGE, argv[0]);
	fflush(stderr);
}

int main( int argc, char **argv )
{
	char version[100];
	char iec61850ServerAddress[80];
	char iec61850ServerPort[80];
	char line_number[80];
	char polling_time[80];
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	int c;
	int pollingTime = 1000;
	SYSTEMTIME oT;
	int rc;
	struct args arg;
	int i;

	/////////////////////MMS//////////////////////
	MmsValue* value;
	LinkedList nameList;
	IsoConnectionParameters* connectionParams;
	AcseAuthenticationParameter auth;
	MmsIndication indication;
	MmsConnection con;
	int tcpPort = 102;
	MmsTypeSpecification* typeSpec;
	//////////////////////////////////////////////
			
	iec61850ServerAddress[0] = '\0';
	iec61850ServerPort[0] = '\0';
	line_number[0] = '\0';
	polling_time[0] = '\0';
		
	//version control///////////////////////////////////////////////////////////////
	sprintf(version, ""APPLICATION" - Built on %s %s %s",__DATE__,__TIME__,SUPPLIER);
	fprintf(stderr, "%s\n", version);
	fflush(stderr);
	
	GetLocalTime(&oT);
	fprintf(stderr,"%02d/%02d/%04d, %02d:%02d:%02d Starting ... %s\n",oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,APPLICATION); 
	fflush(stderr);
	////////////////////////////////////////////////////////////////////////////////

	while( ( c = getopt ( argc, argv, "a:p:l:t:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
			strcpy(iec61850ServerAddress, optarg);
			break;
			case 'p' :
			strcpy(iec61850ServerPort, optarg);
			break;
			case 'l' :
			strcpy(line_number, optarg);
			break;
			case 't' :
			strcpy(polling_time, optarg);
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

	if(strlen(iec61850ServerAddress) == 0)
	{
		fprintf(stderr,"DNP3 server IP address is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	if(strlen(iec61850ServerPort) == 0)
	{
		fprintf(stderr,"DNP3 TCP port is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	tcpPort = atoi(iec61850ServerPort);

	if(strlen(line_number) == 0)
	{
		fprintf(stderr,"line_number is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	
	if(strlen(polling_time) == 0)
	{
		fprintf(stderr,"polling_time is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	pollingTime = atoi(polling_time);
	
	strcpy(NewConsoleTitle, "iec61850master IP ");
	strcat(NewConsoleTitle, iec61850ServerAddress);
	strcat(NewConsoleTitle, " PORT ");
	strcat(NewConsoleTitle, iec61850ServerPort);
	strcat(NewConsoleTitle, " LINE ");
	strcat(NewConsoleTitle, line_number);
	strcat(NewConsoleTitle, " polling time ");
	strcat(NewConsoleTitle, polling_time);

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
    
    strcpy(arg.line_number, line_number);

	if(_beginthread(PipeWorker, 0, (void*)&arg) == -1)
	{
		long nError = GetLastError();

		fprintf(stderr,"PipeWorker _beginthread failed, error code = %d", nError);
		fflush(stderr);
		return EXIT_FAILURE;	
	}

	///////////////////////////////////Protocol//////////////////////////////////
	//TODO: substitute the following code with class IEC61850ClientApp

	con = MmsConnection_create();

	/* use authentication */
	auth = calloc(1, sizeof(struct sAcseAuthenticationParameter));
	auth->mechanism = AUTH_PASSWORD;
	auth->value.password.string = "indigoscada";

	connectionParams = calloc(1, sizeof(IsoConnectionParameters));
	connectionParams->acseAuthParameter = auth;

	MmsConnection_setIsoConnectionParameters(con, connectionParams);

	indication = MmsConnection_connect(con, iec61850ServerAddress, tcpPort);

	if (indication != MMS_OK) {
		printf("MMS connect failed!\n");
		goto exit;
	}
	else
		printf("MMS connected.\n\n");

	printf("Domains present on server:\n--------------------------\n");
	nameList = MmsConnection_getDomainNames(con);
	LinkedList_printStringList(nameList);
	LinkedList_destroy(nameList);
	printf("\n");


	printf("Named variables for domain SampleIEDDevice1:\n--------------------------------------------\n");
	nameList = MmsConnection_getDomainVariableNames(con, "SampleIEDDevice1");
	//LinkedList_printStringList(nameList);
	
	{
		LinkedList element = nameList;
		int elementCount = 0;
		char* str;

		while ((element = LinkedList_getNext(element)) != NULL) 
		{
			str = (char*) (element->data);

			typeSpec = MmsConnection_getVariableAccessAttributes(con, "SampleIEDDevice1", str);

			switch(typeSpec->type) 
			{
				case MMS_STRUCTURE:
					//fprintf(stderr, "%s   ", str);
					//fprintf(stderr, "MMS_STRUCTURE\n");
					//fflush(stderr);
				break;
				case MMS_BOOLEAN:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_BOOLEAN\n");
					fflush(stderr);
				break;
				case MMS_BIT_STRING:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_BIT_STRING\n");
					fflush(stderr);
				break;
				case MMS_UNSIGNED:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_UNSIGNED\n");
					fflush(stderr);
				break;
				case MMS_OCTET_STRING:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_OCTET_STRING\n");
					fflush(stderr);
				break;
				case MMS_GENERALIZED_TIME:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_GENERALIZED_TIME\n");
					fflush(stderr);
				break;
				case MMS_BINARY_TIME:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_BINARY_TIME\n");
					fflush(stderr);
				break;
				case MMS_BCD:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_BCD\n");
					fflush(stderr);
				break;
				case MMS_OBJ_ID:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_OBJ_ID\n");
					fflush(stderr);
				break;
				case MMS_STRING:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_STRING\n");
					fflush(stderr);
				break;
				case MMS_ARRAY:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_ARRAY\n");
					fflush(stderr);
				break;
				case MMS_UTC_TIME:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_UTC_TIME\n");
					fflush(stderr);
				break;
				case MMS_INTEGER:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_INTEGER\n");
					fflush(stderr);
				break;
				case MMS_VISIBLE_STRING:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_VISIBLE_STRING\n");
					fflush(stderr);
				break;
				case MMS_FLOAT:
					fprintf(stderr, "%s   ", str);
					if (typeSpec->typeSpec.floatingpoint.formatWidth == 64) {
						fprintf(stderr, "MMS_FLOAT 64\n");
						fflush(stderr);
					}
					else {
						fprintf(stderr, "MMS_FLOAT 32\n");
						fflush(stderr);
					}
				break;
				default:
					fprintf(stderr, "%s   ", str);
					fprintf(stderr, "MMS_DEFAULT\n");
					fflush(stderr);
				break;
			}

			elementCount++;
		}
	}

	LinkedList_destroy(nameList);
	printf("\n");

	printf("Data sets for domain SampleIEDDevice1:\n--------------------------------------------\n");
	nameList = MmsConnection_getDomainVariableListNames(con, "SampleIEDDevice1");
	LinkedList_printStringList(nameList);
	LinkedList_destroy(nameList);
	printf("\n");
	
	for(i = 0; ;i++)
	{
		value = MmsConnection_readVariable(con, "SampleIEDDevice1", "MMXU2$MX$TotW$mag$f");

		printf("Read variable with value: %f\n", MmsValue_toFloat(value));

		MmsValue_setFloat(value, 1.3333f + (float)i);

		printf("Write variable with value: %f\n", MmsValue_toFloat(value));

		MmsConnection_writeVariable(con, "SampleIEDDevice1", "MMXU2$MX$TotW$mag$f", value);

		MmsValue_delete(value);

		value = MmsConnection_readVariable(con, "SampleIEDDevice1", "MMXU2$MX$TotW$mag$f");

		printf("Read variable with value: %f\n", MmsValue_toFloat(value));

		MmsValue_delete(value);

		value = MmsConnection_readVariable(con, "SampleIEDDevice1", "MMXU2$ST$Health$stVal");

		printf("Read integer variable with value: %d\n", MmsValue_toInt32(value));

		MmsValue_setInt32(value, i);

		MmsConnection_writeVariable(con, "SampleIEDDevice1", "MMXU2$ST$Health$stVal", value);

		MmsValue_delete(value);

		value = MmsConnection_readVariable(con, "SampleIEDDevice1", "MMXU2$ST$Health$stVal");

		printf("Read integer variable with value: %d\n", MmsValue_toInt32(value));

		MmsValue_delete(value);


		Sleep(pollingTime);
	}

exit:
	MmsConnection_destroy(con);

	return 0;
}

///////////////////////////////////Keep alive pipe management/////////////////////////////////////////////////////

#define BUF_SIZE 200
#define N_PIPES 3

void PipeWorker(void* pParam)
{
	HANDLE pipeHnds[N_PIPES];
	char in_buffer[N_PIPES][BUF_SIZE];
	OVERLAPPED ovrp[N_PIPES];
	HANDLE evnt[N_PIPES];
	DWORD rc, len, pipe_id;
    unsigned char buf[sizeof(struct iec_item)];
    struct iec_item* p_item;
	char pipe_name[150];
    int i;

    struct args* arg = (struct args*)pParam;

	strcpy(pipe_name, "\\\\.\\pipe\\iec61850master_namedpipe");
    strcat(pipe_name, arg->line_number);

	for(i = 0; i < N_PIPES; i++)
	{
		if ((pipeHnds[i] = CreateNamedPipe(
			pipe_name,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, N_PIPES,
			0, 
			0, 
			1000, 
			NULL)) == INVALID_HANDLE_VALUE)
		{
			fprintf(stderr,"CreateNamedPipe for pipe %d failed with error %d\n", i, GetLastError());
			fflush(stderr);
			ExitProcess(0);
		}

		if ((evnt[i] = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		{
			fprintf(stderr,"CreateEvent for pipe %d failed with error %d\n",	i, GetLastError());
			fflush(stderr);
			ExitProcess(0);
		}

		ZeroMemory(&ovrp[i], sizeof(OVERLAPPED));

		ovrp[i].hEvent = evnt[i];

		if (ConnectNamedPipe(pipeHnds[i], &ovrp[i]) == 0)
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				fprintf(stderr,"ConnectNamedPipe for pipe %d failed with error %d\n", i, GetLastError());
				fflush(stderr);
				
				CloseHandle(pipeHnds[i]);
				ExitProcess(0);
			}
		}
	}

	while(1)
	{
		if((rc = WaitForMultipleObjects(N_PIPES, evnt, FALSE, INFINITE)) == WAIT_FAILED)
		{
			fprintf(stderr,"WaitForMultipleObjects failed with error %d\n", GetLastError());
			fflush(stderr);
			ExitProcess(0);
		}

		pipe_id = rc - WAIT_OBJECT_0;

		ResetEvent(evnt[pipe_id]);

		if(GetOverlappedResult(pipeHnds[pipe_id], &ovrp[pipe_id], &len, TRUE) == 0)
		{
			fprintf(stderr,"GetOverlapped result failed %d start over\n", GetLastError());
			fflush(stderr);
		
			if(DisconnectNamedPipe(pipeHnds[pipe_id]) == 0)
			{
				fprintf(stderr,"DisconnectNamedPipe failed with error %d\n", GetLastError());
				fflush(stderr);
				ExitProcess(0);
			}

			if(ConnectNamedPipe(pipeHnds[pipe_id],	&ovrp[pipe_id]) == 0)
			{
				if(GetLastError() != ERROR_IO_PENDING)
				{
					fprintf(stderr,"ConnectNamedPipe for pipe %d failed with error %d\n", i, GetLastError());
					fflush(stderr);
					CloseHandle(pipeHnds[pipe_id]);
				}
			}
		}
		else
		{
			ZeroMemory(&ovrp[pipe_id], sizeof(OVERLAPPED));

			ovrp[pipe_id].hEvent = evnt[pipe_id];

			if((rc = ReadFile(pipeHnds[pipe_id], in_buffer[pipe_id], sizeof(struct iec_item), NULL, &ovrp[pipe_id])) == 0)
			{
				if(GetLastError() != ERROR_IO_PENDING)
				{
					fprintf(stderr,"ReadFile failed with error %d\n", GetLastError());
					fflush(stderr);
				}
			}
			
			memcpy(buf, in_buffer[pipe_id], sizeof(struct iec_item));
			
			if(len)
			{
				p_item = (struct iec_item*)buf;

				//fprintf(stderr, "Receiving from pipe %d th message\n", p_item->msg_id);
				//fflush(stderr);
											
				//for (j = 0; j < len; j++) 
				//{ 
					//unsigned char c = *((unsigned char*)buf + j);
					//fprintf(stderr, "rx pipe <--- 0x%02x-", c);
					//fflush(stderr);
					//
				//}
				
				rc = clearCrc((unsigned char *)buf, sizeof(struct iec_item));
				if(rc != 0)
				{
					ExitProcess(0);
				}

				if(p_item->iec_obj.ioa == 4004)
				{ 
					gl_timeout_connection_with_parent = 0;
					//fprintf(stderr, "Receive keep alive # %d from front end\n", p_item->msg_id);
                    fprintf(stderr, "wdg %d\r", p_item->msg_id);
				    fflush(stderr);
				}
			}
		}
	}
}
