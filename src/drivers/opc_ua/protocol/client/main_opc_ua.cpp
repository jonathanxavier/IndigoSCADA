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
#include <stdlib.h>
#include <errno.h>
#include "opc_ua_imp.h"

int gl_timeout_connection_with_parent = 0;

#define SUPPLIER "@ enscada.com"
#define APPLICATION "opc_ua_client.exe"

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

void PipeWorker(void* pParam);

#define RUNTIME_USAGE "Run time usage: %s -a server URL -l line number -t polling time -f log_file_name\n"

void usage(char** argv)
{
	fprintf(stderr, RUNTIME_USAGE, argv[0]);
	fflush(stderr);
}

int main( int argc, char **argv )
{
	char version[100];
	char serverURL[80];
	char line_number[80];
	char polling_time[80];
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	int  c, rc;
	unsigned long pollingTime = 1000;

	FILE *iec_protocol_log_stream = NULL;
	char iec_optional_log_file_name[80] = {0};
	char log_file_name[250] = {0};
	
	serverURL[0] = '\0';

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

	while( ( c = getopt ( argc, argv, "a:l:t:f:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
			strcpy(serverURL, optarg);
			break;
			case 't' :
			strcpy(polling_time, optarg);
			break;
			case 'l' :
			strcpy(line_number, optarg);
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

	if(strlen(serverURL) > 0)
	{
		strcpy(NewConsoleTitle, "OPC UA URL ");
		strcat(NewConsoleTitle, serverURL);
	}

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

    /////////////////keep alive////////////////////////////////////////////////////
    struct args arg;
    strcpy(arg.line_number, line_number);

	if(_beginthread(PipeWorker, 0, (void*)&arg) == -1)
	{
		long nError = GetLastError();

		fprintf(stderr,"PipeWorker _beginthread failed, error code = %d", nError);
		fflush(stderr);
		return EXIT_FAILURE;	
	}
	/////////////////end keep alive////////////////////////////////////////////////

	opcua_imp* po = new opcua_imp(serverURL, line_number, atoi(polling_time));

	if(po == NULL)
	{
		return EXIT_FAILURE;
	}

	rc = po->Start();

	if(rc)
	{
		return EXIT_FAILURE;
	}

	rc = po->PollServer();

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

	strcpy(pipe_name, "\\\\.\\pipe\\opcua_client_namedpipe");
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
			//ExitProcess(0);
		}

		if ((evnt[i] = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
		{
			fprintf(stderr,"CreateEvent for pipe %d failed with error %d\n",	i, GetLastError());
			fflush(stderr);
			//ExitProcess(0);
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
				//ExitProcess(0);
			}
		}
	}

	while(1)
	{
		if((rc = WaitForMultipleObjects(N_PIPES, evnt, FALSE, INFINITE)) == WAIT_FAILED)
		{
			fprintf(stderr,"WaitForMultipleObjects failed with error %d\n", GetLastError());
			fflush(stderr);
			//ExitProcess(0);
		}

		pipe_id = rc - WAIT_OBJECT_0;

		ResetEvent(evnt[pipe_id]);

		if(GetOverlappedResult(pipeHnds[pipe_id], &ovrp[pipe_id], &len, TRUE) == 0)
		{
			fprintf(stderr,"GetOverlapped result failed %d start over\n", GetLastError());
			fflush(stderr);
			ExitProcess(0);
		
			if(DisconnectNamedPipe(pipeHnds[pipe_id]) == 0)
			{
				fprintf(stderr,"DisconnectNamedPipe failed with error %d\n", GetLastError());
				fflush(stderr);
				//ExitProcess(0);
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
					//ExitProcess(0);
				}

				if(p_item->iec_obj.ioa == 4004)
				{ 
					gl_timeout_connection_with_parent = 0;
					//fprintf(stderr, "Receive keep alive # %d from front end\n", p_item->msg_id);
                    fprintf(stdout, "wdg %d\r", p_item->msg_id);
				    fflush(stdout);
				}
			}
		}
	}
}
