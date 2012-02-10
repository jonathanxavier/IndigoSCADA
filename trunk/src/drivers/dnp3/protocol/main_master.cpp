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
#include "dummy.hpp"

#define ENOENT 1
#define EIO 1
#define EOK 0
#define WAIT_FOREVER ((time_t)-1)
 
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
   char mutex_name[50];

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

enum error_codes {
    ok = 0,
    not_opened = -1,
    broken_pipe = -2,
    timeout_expired = -3
};

int read(SOCKET s, void* buf, size_t min_size, size_t max_size, time_t timeout)
{ 
	int errcode = 0;
    size_t size = 0;
    time_t start = 0;

    if (timeout != WAIT_FOREVER) 
	{ 
        start = time(NULL); 
    }

    do{ 
        int rc;

        if (timeout != WAIT_FOREVER)
		{ 
            fd_set events;
            struct timeval tm;
            FD_ZERO(&events);
            FD_SET(s, &events);
            tm.tv_sec = (long)timeout;
            tm.tv_usec = 0;
            rc = select((int)s+1, &events, NULL, NULL, &tm);
            if (rc < 0) 
			{ 
                errcode = WSAGetLastError();
                fprintf(stderr, "Socket select is failed: %d\n", errcode);
			    fflush(stderr);

                return -1;
            }

            if (rc == 0) 
			{
                return size;
            }

            time_t now = time(NULL);
            timeout = start + timeout >= now ? timeout + start - now : 0;  
        }

        rc = recv(s, (char*)buf + size, max_size - size, 0);

        if (rc < 0) 
		{ 
            errcode = WSAGetLastError();
            fprintf(stderr,"Socket read is failed: %d\n", errcode);
			fflush(stderr);

            return -1;
        } 
		else if (rc == 0) 
		{
            errcode = broken_pipe;
            fprintf(stderr,"Socket is disconnected\n");
			fflush(stderr);
            return -1; 
        }
		else 
		{
            size += rc; 
        }

    }while (size < min_size); 

    return (int)size;
}

#include "getopt.h" 

int main( int argc, char **argv )
{
	char version[100];
	char dnp3ServerAddress[80];
	char dnp3ServerPort[80];
	int c;

	dnp3ServerAddress[0] = '\0';
	dnp3ServerPort[0] = '\0';

	//version control///////////////////////////////////////////////////////////////
	sprintf(version, ""APPLICATION" - Built on %s %s %s",__DATE__,__TIME__,SUPPLIER);
	fprintf(stderr, "%s\n", version);
	fflush(stderr);
	////////////////////////////////////////////////////////////////////////////////

	while( ( c = getopt ( argc, argv, "a:p:?" )) != EOF ) {
		switch ( c ) {
			case 'a' :
				strcpy(dnp3ServerAddress, optarg);
			break;
			case 'p' :
				strcpy(dnp3ServerPort, optarg);
			break;
			case '?' :
				fprintf(stderr, "Run time usage: %s -a server IP address -p server IP port\n", argv[0]);
				fflush(stderr);
				exit( 0 );
		}
	}

	DNP3MasterApp* master_app;

	master_app = new DNP3MasterApp();   

    if(!master_app->OpenLink(dnp3ServerAddress, atoi(dnp3ServerPort)))
    {  
		Master* master_p;
		DummyDb db;
		DummyTimer timer;
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

		DummyTx tx(&debugLevel, 'M', 'S', master_app->getSocket());

		datalinkConfig.tx_p                  = &tx;
		datalinkConfig.debugLevel_p          = &debugLevel;

		master_p = new Master (masterConfig, datalinkConfig, &stationConfig, 1, &db, &timer);

		//Write
		master_p->poll( Master::INTEGRITY);

		//Read
		char data_p[80];
		int n_read;

		n_read = read(master_app->getSocket(), data_p, 1, 80, 60);

		if(n_read > 0)
		{
			// put the char data into a Bytes container
			Bytes bytes((unsigned char*)data_p, (unsigned char*)data_p + n_read);

			master_p->rxData(&bytes, 0);
		}

        for(;;)   
        {   
			master_p->startNewTransaction();

			n_read = read(master_app->getSocket(), data_p, 1, 80, WAIT_FOREVER);

			if(n_read > 0)
			{
				// put the char data into a Bytes container
				Bytes bytes((unsigned char*)data_p, (unsigned char*)data_p + n_read);

				master_p->rxData(&bytes, 0);
			}
			else
			{
				break; //exit loop
			}

            Sleep(500);   
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




