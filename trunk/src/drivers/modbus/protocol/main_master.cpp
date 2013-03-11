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
#include "modbus.h"
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

enum {
    TCP,
    TCP_PI,
    RTU
};

int gl_timeout_connection_with_parent = 0;

#define SUPPLIER "@ enscada.com"
#define APPLICATION "modbus_master.exe"

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

#define RUNTIME_USAGE "Run time usage: %s -a server IP address -p server TCP port -l line number -t polling time\n"

void usage(char** argv)
{
	fprintf(stderr, RUNTIME_USAGE, argv[0]);
	fflush(stderr);
}

int main( int argc, char **argv )
{
	char version[100];
	char modbusServerAddress[80];
	char modbusServerPort[80];
	char line_number[80];
	char polling_time[80];
	char fifo_monitor_direction_name[70];
	char fifo_control_direction_name[70];
	char OldConsoleTitle[500];
	char NewConsoleTitle[500];
	int c;
	unsigned long pollingTime = 1000;
	int serverID = 0;
	
	modbusServerAddress[0] = '\0';
	modbusServerPort[0] = '\0';
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
			strcpy(modbusServerAddress, optarg);
			break;
			case 'p' :
			strcpy(modbusServerPort, optarg);
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

	if(strlen(modbusServerAddress) == 0)
	{
		fprintf(stderr,"modbus server IP address is not known\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}

	if(strlen(modbusServerPort) == 0)
	{
		fprintf(stderr,"modbus TCP port is not known\n");
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

	pollingTime = atoi(polling_time);

	serverID = atoi(line_number);

	strcpy(fifo_monitor_direction_name, "fifo_monitor_direction");
	strcpy(fifo_control_direction_name, "fifo_control_direction");
	
	if(strlen(line_number) > 0)
	{
		strcat(fifo_monitor_direction_name, line_number);
		strcat(fifo_control_direction_name, line_number);
	}

    strcat(fifo_control_direction_name, "modbus");
    strcat(fifo_monitor_direction_name, "modbus");

	strcpy(NewConsoleTitle, "modbus_master IP ");
	strcat(NewConsoleTitle, modbusServerAddress);
	strcat(NewConsoleTitle, " PORT ");
	strcat(NewConsoleTitle, modbusServerPort);
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

	#ifdef TEST_ALONE
	if(_beginthread(PipeWorker, 0, (void*)&arg) == -1)
	{
		long nError = GetLastError();

		fprintf(stderr,"PipeWorker _beginthread failed, error code = %d", nError);
		fflush(stderr);
		return EXIT_FAILURE;	
	}
	#endif

//TODO:  Sobstitute the following code with the class modbus_imp

	////////////////////////////////Start protocol implementation///////////////////////////////////
	uint8_t *tab_rp_bits;
    uint16_t *tab_rp_registers;
    modbus_t *ctx;
    int i;
    uint8_t value;
    int nb_points;
    float real;
    struct timeval old_response_timeout;
    struct timeval response_timeout;

    const uint16_t UT_BITS_ADDRESS = 0x13;
    const uint16_t UT_BITS_NB = 0x25;
    const uint8_t UT_BITS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };
    const uint16_t UT_INPUT_BITS_ADDRESS = 0xC4;
    const uint16_t UT_INPUT_BITS_NB = 0x16;
    const uint16_t UT_REGISTERS_ADDRESS = 0x6B;
    const uint16_t UT_REGISTERS_NB = 0x3;
    const uint16_t UT_REGISTERS_TAB[] = { 0x022B, 0x0001, 0x0064 };
    const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x08;
    const uint16_t UT_INPUT_REGISTERS_NB = 0x1;
    const float UT_REAL = (float)916.540649;
    const uint32_t UT_IREAL = 0x4465229a;

	ctx = modbus_new_tcp(modbusServerAddress, atoi(modbusServerPort));

    if(ctx == NULL)
	{
        fprintf(stderr, "Unable to allocate modbus context\n");
        return -1;
    }

    modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx, (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL));

    if(modbus_connect(ctx) == -1)
	{
        fprintf(stderr, "Connection failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the memory to store the bits */
    nb_points = (UT_BITS_NB > UT_INPUT_BITS_NB) ? UT_BITS_NB : UT_INPUT_BITS_NB;
    tab_rp_bits = (uint8_t *) malloc(nb_points * sizeof(uint8_t));
    memset(tab_rp_bits, 0, nb_points * sizeof(uint8_t));

    /* Allocate and initialize the memory to store the registers */
    nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
        UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
    tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

    printf("** UNIT TESTING **\n");

	for(;;)
	{
		Sleep(pollingTime);

	    /* Allocate and initialize the memory to store the bits */
		memset(tab_rp_bits, 0, nb_points * sizeof(uint8_t));

		/* Allocate and initialize the memory to store the registers */
		memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

#if 0
		printf("\nTEST WRITE/READ:\n");

		/** COIL BITS **/

		/* Single */
		rc = modbus_write_bit(ctx, UT_BITS_ADDRESS, ON);
		printf("1/2 modbus_write_bit: ");
		if (rc == 1) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
//			goto close;
		}
#endif
		rc = modbus_read_bits(ctx, UT_BITS_ADDRESS, 1, tab_rp_bits);
		printf("2/2 modbus_read_bits: ");
		if (rc != 1) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		if (tab_rp_bits[0] != ON) {
			printf("FAILED (%0X = != %0X)\n", tab_rp_bits[0], ON);
			goto close;
		}
		printf("OK\n");
		/* End single */
#if 0
		/* Multiple bits */
		{
			//uint8_t tab_value[UT_BITS_NB];
			uint8_t tab_value[0x25];

			modbus_set_bits_from_bytes(tab_value, 0, UT_BITS_NB, UT_BITS_TAB);
			rc = modbus_write_bits(ctx, UT_BITS_ADDRESS,
								   UT_BITS_NB, tab_value);
			printf("1/2 modbus_write_bits: ");
			if (rc == UT_BITS_NB) {
				printf("OK\n");
			} else {
				printf("FAILED\n");
				goto close;
			}
		}
#endif
		rc = modbus_read_bits(ctx, UT_BITS_ADDRESS, UT_BITS_NB, tab_rp_bits);
		printf("2/2 modbus_read_bits: ");
		if (rc != UT_BITS_NB) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		i = 0;
		nb_points = UT_BITS_NB;
		while (nb_points > 0) {
			int nb_bits = (nb_points > 8) ? 8 : nb_points;

			value = modbus_get_byte_from_bits(tab_rp_bits, i*8, nb_bits);
			if (value != UT_BITS_TAB[i]) {
				printf("FAILED (%0X != %0X)\n", value, UT_BITS_TAB[i]);
				goto close;
			}

			nb_points -= nb_bits;
			i++;
		}
		printf("OK\n");
		/* End of multiple bits */
	
#if 0		
		/* Single register */
		rc = modbus_write_register(ctx, UT_REGISTERS_ADDRESS, 0x1234);
		printf("1/2 modbus_write_register: ");
		if (rc == 1) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}
#endif
		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   1, tab_rp_registers);
		printf("2/2 modbus_read_registers: ");
		if (rc != 1) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		if (tab_rp_registers[0] != 0x1234) {
			printf("FAILED (%0X != %0X)\n",
				   tab_rp_registers[0], 0x1234);
			goto close;
		}
		printf("OK\n");
		/* End of single register */
#if 0
		/* Many registers */
		rc = modbus_write_registers(ctx, UT_REGISTERS_ADDRESS,
									UT_REGISTERS_NB, UT_REGISTERS_TAB);
		printf("1/5 modbus_write_registers: ");
		if (rc == UT_REGISTERS_NB) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

#endif

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   UT_REGISTERS_NB, tab_rp_registers);
		printf("2/5 modbus_read_registers: ");
		if (rc != UT_REGISTERS_NB) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}

		for (i=0; i < UT_REGISTERS_NB; i++) {
			if (tab_rp_registers[i] != UT_REGISTERS_TAB[i]) {
				printf("FAILED (%0X != %0X)\n",
					   tab_rp_registers[i],
					   UT_REGISTERS_TAB[i]);
				goto close;
			}
		}
		printf("OK\n");

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   0, tab_rp_registers);
		printf("3/5 modbus_read_registers (0): ");
		if (rc != 0) {
			printf("FAILED (nb points %d)\n", rc);
			goto close;
		}
		printf("OK\n");
	
		/* End of many registers */

		printf("\nTEST FLOATS\n");
		/** FLOAT **/
		printf("1/2 Set float: ");
		modbus_set_float(UT_REAL, tab_rp_registers);
		if (tab_rp_registers[1] == (UT_IREAL >> 16) &&
			tab_rp_registers[0] == (UT_IREAL & 0xFFFF)) {
			printf("OK\n");
		} else {
			printf("FAILED (%x != %x)\n",
				   *((uint32_t *)tab_rp_registers), UT_IREAL);
			goto close;
		}

		printf("2/2 Get float: ");
		real = modbus_get_float(tab_rp_registers);
		if (real == UT_REAL) {
			printf("OK\n");
		} else {
			printf("FAILED (%f != %f)\n", real, UT_REAL);
			goto close;
		}

		printf("\nAt this point, error messages doesn't mean the test has failed\n");
	
		/** TOO MANY DATA **/
		printf("\nTEST TOO MANY DATA ERROR:\n");

		rc = modbus_read_bits(ctx, UT_BITS_ADDRESS,
							  MODBUS_MAX_READ_BITS + 1, tab_rp_bits);
		printf("* modbus_read_bits: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
									MODBUS_MAX_READ_BITS + 1, tab_rp_bits);
		printf("* modbus_read_input_bits: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   MODBUS_MAX_READ_REGISTERS + 1,
								   tab_rp_registers);
		printf("* modbus_read_registers: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}

		rc = modbus_read_input_registers(ctx, UT_INPUT_REGISTERS_ADDRESS,
										 MODBUS_MAX_READ_REGISTERS + 1,
										 tab_rp_registers);
		printf("* modbus_read_input_registers: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}
#if 0
		rc = modbus_write_bits(ctx, UT_BITS_ADDRESS,
							   MODBUS_MAX_WRITE_BITS + 1, tab_rp_bits);
		printf("* modbus_write_bits: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			goto close;
			printf("FAILED\n");
		}

		rc = modbus_write_registers(ctx, UT_REGISTERS_ADDRESS,
									MODBUS_MAX_WRITE_REGISTERS + 1,
									tab_rp_registers);
		printf("* modbus_write_registers: ");
		if (rc == -1 && errno == EMBMDATA) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}
#endif
		/** SLAVE REPLY **/
		printf("\nTEST SLAVE REPLY:\n");
		modbus_set_slave(ctx, serverID);
		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS, UT_REGISTERS_NB, tab_rp_registers);
		
		/* Response in TCP mode */
		printf("1/4 Response from slave %d: ", 18);

		if (rc == UT_REGISTERS_NB) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}
		
		rc = modbus_set_slave(ctx, MODBUS_BROADCAST_ADDRESS);
		if (rc == -1) {
			printf("Invalid broacast address\n");
			goto close;
		}

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   UT_REGISTERS_NB, tab_rp_registers);
		printf("2/4 Reply after a broadcast query: ");
		if (rc == UT_REGISTERS_NB) {
			printf("OK\n");
		} else {
			printf("FAILED\n");
			goto close;
		}
		
		modbus_set_slave(ctx, MODBUS_TCP_SLAVE);
		
		/* Save original timeout */
		modbus_get_response_timeout(ctx, &old_response_timeout);

		/* Define a new and too short timeout */
		response_timeout.tv_sec = 0;
		response_timeout.tv_usec = 0;
		modbus_set_response_timeout(ctx, &response_timeout);

		rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
								   UT_REGISTERS_NB, tab_rp_registers);
		printf("4/4 Too short timeout: ");
		if (rc == -1 && errno == ETIMEDOUT) {
			printf("OK\n");
		} else {
			printf("FAILED (can fail on slow systems or Windows)\n");
		}

		/* Restore original timeout */
		modbus_set_response_timeout(ctx, &old_response_timeout);

		/* A wait and flush operation is done by the error recovery code of
		 * libmodbus */
	
		printf("\nALL TESTS PASS WITH SUCCESS.\n");

	}

close:
    /* Free the memory */
    free(tab_rp_bits);
    free(tab_rp_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

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

	strcpy(pipe_name, "\\\\.\\pipe\\modbus_master_namedpipe");
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
