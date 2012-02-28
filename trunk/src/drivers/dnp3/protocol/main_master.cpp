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

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <winsock2.h>
#include "station.hpp"
#include "master.hpp"
#include "datalink.hpp"
#include "custom.hpp"
#include "clear_crc_eight.h"
#include "iec104types.h"
#include "iec_item.h"
#include "process.h"

#define ENOENT 1
#define EIO 1
#define EOK 0

static int timeout_connection_with_parent = 0;
 
class DNP3MasterApp { 
private: 
	SOCKET      Socket; 
	WSADATA     wsaData; 
	SOCKADDR_IN	RemoteInfo; 
	char        RemoteHost[32]; 
	int         RemotePort; 
	bool		Connected; 
 
	char		ErrorMode; 
	char		LastError[100]; 
 
public: 
	DNP3MasterApp(void); 
	~DNP3MasterApp(void); 
	 
	int OpenLink(char *serverIP,int port=20000); 
	int CloseLink(bool free=true); 
	SOCKET getSocket(void) { return Socket;};
	bool GetSockConnectStatus(void);
};

//   
//  Class constructor.   
//   
DNP3MasterApp::DNP3MasterApp()   
{   
    this->Connected=false;   
    return;   
}   
   
//   
//  Class destructor.   
//   
DNP3MasterApp::~DNP3MasterApp()   
{   
    // free resources   
    CloseLink();   
    return;   
}   

bool DNP3MasterApp::GetSockConnectStatus(void)
{
	return Connected;
}

//   
//  Open TCP/IP connection.   
//   
int DNP3MasterApp::OpenLink(char *serverIP, int port)   
{   
    WORD wVersionRequested;   
    int wsaerr;   
   
    // connected ?   
    if(GetSockConnectStatus())   
    {   
		// report warning   
		sprintf(LastError, "Socket is connected!\n");   
		fprintf(stderr, "%s\n", LastError);
		fflush(stderr);

		return EIO;   
    }   
    else   
    {   
		// Using MAKEWORD macro, Winsock version request 2.2   
		wVersionRequested = MAKEWORD(2, 2);   
		wsaerr = WSAStartup(wVersionRequested, &wsaData);   
		if(wsaerr != 0)   
		{   
			/* Tell the user that we could not find a usable */   
			/* WinSock DLL.*/   
			sprintf(LastError, "The Winsock dll not found!\n");
			fprintf(stderr, "%s\n", LastError);
			fflush(stderr);

			return ENOENT;   
		}   
	#ifdef _DEBUG   
		sprintf(LastError, "The Winsock dll found! - The status: %s.\n", wsaData.szSystemStatus);
		fprintf(stderr, "%s\n", LastError);
		fflush(stderr);
    
	#endif   
		sprintf(this->RemoteHost, serverIP);

		this->RemotePort = port;
		
		fprintf(stderr, "remote port %d\n", this->RemotePort);
		fflush(stderr);

		// init socket   
		// Create the socket as an IPv4 (AF_INET)
		Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   
		if(Socket == INVALID_SOCKET)   
		{   
			//Output the error recieved when creating the socket.   
			sprintf(LastError, "Socket error: %ld\n", WSAGetLastError());
			fprintf(stderr, "%s\n", LastError);
			fflush(stderr);
        
			WSACleanup();   
			Socket = 0;   
        
			return EIO;   
		}   

		//Assign the remote info to our socket address structure.   
		RemoteInfo.sin_family = AF_INET;   
		RemoteInfo.sin_addr.s_addr = inet_addr(this->RemoteHost);   
		RemoteInfo.sin_port = htons((SHORT)this->RemotePort);   
       
		//Time to connect to the remote computer   
		if(connect(Socket, (SOCKADDR*)&RemoteInfo, sizeof(RemoteInfo)) == SOCKET_ERROR)   
		{   
			sprintf(LastError, "Socket error: Unable to establish a connection to %s:%i!\n", RemoteHost, RemotePort);
			fprintf(stderr, "%s\n", LastError);
			fflush(stderr);
        
			closesocket(Socket);   
			WSACleanup();   
			Socket = 0;   
			return EIO;   
		}   
    }
	
    this->Connected=true;   
    return EOK;   
}   
   
//   
//  Close TCP/IP connection   
//   
int DNP3MasterApp::CloseLink(bool free)   
{   
    this->Connected=false;   
    shutdown(Socket, SD_BOTH);   
    closesocket(Socket);   
    WSACleanup();   
    return EOK;   
}

#define SUPPLIER "@ enscada.com"
#define APPLICATION "dnp3master.exe"

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

int main( int argc, char **argv )
{
	char version[100];
	char dnp3ServerAddress[80];
	char dnp3ServerPort[80];
	char line_number[80];
	char polling_time[80];
	char fifo_monitor_direction_name[70];
	char fifo_control_direction_name[70];
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	int c;
	double pollingTime = 1000;
	
	dnp3ServerAddress[0] = '\0';
	dnp3ServerPort[0] = '\0';
	line_number[0] = '\0';
	polling_time[0] = '\0';
	fifo_monitor_direction_name[0] = '\0';
	fifo_control_direction_name[0] = '\0';

	//version control///////////////////////////////////////////////////////////////
	sprintf(version, ""APPLICATION" - Built on %s %s %s",__DATE__,__TIME__,SUPPLIER);
	fprintf(stderr, "%s\n", version);
	fflush(stderr);
	SYSTEMTIME oT;
	::GetLocalTime(&oT);
	fprintf(stderr,"%02d/%02d/%04d, %02d:%02d:%02d Starting ... %s\n",oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,APPLICATION); 
	fflush(stderr);
	////////////////////////////////////////////////////////////////////////////////

	while( ( c = getopt ( argc, argv, "a:p:l:t:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
			strcpy(dnp3ServerAddress, optarg);
			break;
			case 'p' :
			strcpy(dnp3ServerPort, optarg);
			break;
			case 'l' :
			strcpy(line_number, optarg);
			break;
			case 't' :
			strcpy(polling_time, optarg);
			break;
			case '?' :
			fprintf(stderr, "Run time usage: %s -a server IP address -p server IP port -l line number -t polling time\n", argv[0]);
			fflush(stderr);
			exit( 0 );
		}
	}

	if(strlen(dnp3ServerAddress) == 0)
	{
		fprintf(stderr,"DNP3 server IP address is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	if(strlen(dnp3ServerPort) == 0)
	{
		fprintf(stderr,"DNP3 port is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
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

	pollingTime = atof(polling_time);

	strcpy(fifo_monitor_direction_name, "fifo_monitor_direction");
	strcpy(fifo_control_direction_name, "fifo_control_direction");
	
	if(strlen(line_number) > 0)
	{
		strcat(fifo_monitor_direction_name, line_number);
		strcat(fifo_control_direction_name, line_number);
	}

    strcat(fifo_control_direction_name, "dnp3");
    strcat(fifo_monitor_direction_name, "dnp3");

	strcpy(NewConsoleTitle, "dnp3master IP ");
	strcat(NewConsoleTitle, dnp3ServerAddress);
	strcat(NewConsoleTitle, " PORT ");
	strcat(NewConsoleTitle, dnp3ServerPort);
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

	int rc;
	if((rc = GetConsoleTitle(OldConsoleTitle, sizeof(OldConsoleTitle))) > 0)
	{
		SetConsoleTitle(NewConsoleTitle);
	}

    struct args arg;
    strcpy(arg.line_number, line_number);

	if(_beginthread(PipeWorker, 0, (void*)&arg) == -1)
	{
		long nError = GetLastError();

		fprintf(stderr,"PipeWorker _beginthread failed, error code = %d", nError);
		fflush(stderr);
		return EXIT_FAILURE;	
	}

	///////////////////////////////////Start protocol//////////////////////////////////

	DNP3MasterApp* master_app;

	master_app = new DNP3MasterApp();   

	if(!master_app->OpenLink(dnp3ServerAddress, atoi(dnp3ServerPort)))
	{  
		Master* master_p;
		CustomDb db;
		CustomTimer timer;
		int debugLevel = 1;

		int integrityPollInterval = 10;

		Master::MasterConfig          masterConfig;
		Datalink::DatalinkConfig      datalinkConfig;
		Station::StationConfig        stationConfig;

		masterConfig.addr = 1;
		masterConfig.consecutiveTimeoutsForCommsFail = 3;
		masterConfig.integrityPollInterval_p = &integrityPollInterval;
		masterConfig.debugLevel_p = &debugLevel;

		stationConfig.addr = 1;
		stationConfig.debugLevel_p = &debugLevel;

		datalinkConfig.addr                  = masterConfig.addr;
		datalinkConfig.isMaster              = 1;
		datalinkConfig.keepAliveInterval_ms  = 10000;

		CustomInter tx(&debugLevel, 'M', 'S', master_app->getSocket());

		datalinkConfig.tx_p                  = &tx;
		datalinkConfig.debugLevel_p          = &debugLevel;

		master_p = new Master (masterConfig, datalinkConfig, &stationConfig, 1, &db, &timer);

		//Write
		master_p->poll(Master::INTEGRITY);

		//Read
		char data_p[80];
		int n_read;

		n_read = tx.read(master_app->getSocket(), data_p, 1, 80, 15);

		if(n_read > 0)
		{
			// put the char data into a Bytes container
			//Bytes bytes((unsigned char*)data_p, (unsigned char*)data_p + n_read);

			//master_p->rxData(&bytes, 0);
		}
		else	
		{
			return 1;
		}

		for(;;)   
		{   
			master_p->startNewTransaction();

			n_read = tx.read(master_app->getSocket(), data_p, 1, 80, 15);

			if(n_read > 0)
			{
				// put the char data into a Bytes container
				//Bytes bytes((unsigned char*)data_p, (unsigned char*)data_p + n_read);

				//master_p->rxData(&bytes, 0);
			}
			else
			{
				break; //exit loop
			}

			timeout_connection_with_parent++;

			if(timeout_connection_with_parent > 1000*20/pollingTime)
			{
				break; //exit loop for timeout of connection with parent
			}

			Sleep((unsigned long)pollingTime);
		}   

		Sleep(1000);   
		master_app->CloseLink();   
	}   
	else   
	{   
		bool t = master_app->GetSockConnectStatus();   
		Sleep(30000);   
		master_app->CloseLink();   
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

	strcpy(pipe_name, "\\\\.\\pipe\\dnp3master_namedpipe");
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

				#ifdef USE_CHECKSUM
				unsigned long int j;
				u_int message_checksum, msg_checksum;
				//////calculate checksum with checksum byte set to value zero/////////
				msg_checksum = p_item->checksum;

				p_item->checksum = 0; //azzero

				message_checksum = 0;

				for(j = 0; j < len; j++) 
				{ 
					message_checksum = message_checksum + buf[j];
				}

				message_checksum = message_checksum%256;

				if(message_checksum != msg_checksum)
				{
					fprintf(stderr, "Cheksum error\n");
					fflush(stderr);
					ExitProcess(0);
				}
				//////////////////end checksum////////////////////////////////////////
				#else
				rc = clearCrc((unsigned char *)buf, sizeof(struct iec_item));
				if(rc != 0)
				{
					ExitProcess(0);
				}
				#endif

				if(p_item->iec_obj.ioa == 4004)
				{ 
					timeout_connection_with_parent = 0;
					//fprintf(stderr, "Receive keep alive # %d from front end\n", p_item->msg_id);
                    fprintf(stderr, "wdg %d\r", p_item->msg_id);
				    fflush(stderr);
				}
			}
		}
	}
}
