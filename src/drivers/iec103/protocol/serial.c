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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "serial.h"
#undef WIN32_LEAN_AND_MEAN
#else
#include "portable.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#endif
#include <math.h>

HANDLE          fd;
//HANDLE  wait_event;

static int l_baud_rate = 0;

int open_port(char* port_name, int baud_rate)
{

#ifdef WIN32

	DCB				CommParam;
	COMMTIMEOUTS	CommTimeouts;

    char port[50];

	l_baud_rate = baud_rate;

    strcpy(port, "\\\\.\\");
    strcat(port, port_name);

	fd = CreateFile(port, 
								GENERIC_READ | GENERIC_WRITE,
							   0, 
							   NULL, 
							   OPEN_EXISTING, 
							   FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_NO_BUFFERING, 
							   NULL);

	if(fd == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	else
	{
		fprintf(stderr,"Serial port %s opened\n", port_name);
		fflush(stderr);
	}
	
	if(!GetCommTimeouts(fd, &CommTimeouts))
		return -1;

	memset(&CommParam, 0x00, sizeof(DCB));

	CommParam.DCBlength = sizeof(DCB);		/* sizeof(DCB)                     */

    /* Baudrate at which running       */
    switch (baud_rate) 
    {
        case 110:
            CommParam.BaudRate = CBR_110;
            break;
        case 300:
            CommParam.BaudRate = CBR_300;
            break;
        case 600:
            CommParam.BaudRate = CBR_600;
            break;
        case 1200:
            CommParam.BaudRate = CBR_1200;
            break;
        case 2400:
            CommParam.BaudRate = CBR_2400;
            break;
        case 4800:
            CommParam.BaudRate = CBR_4800;
            break;
        case 9600:
            CommParam.BaudRate = CBR_9600;
            break;
        case 19200:
            CommParam.BaudRate = CBR_19200;
            break;
        case 38400:
            CommParam.BaudRate = CBR_38400;
            break;
        case 57600:
            CommParam.BaudRate = CBR_57600;
            break;
        case 115200:
            CommParam.BaudRate = CBR_115200;
            break;
        default:
            CommParam.BaudRate = CBR_9600;
            fprintf(stderr, "WARNING Unknown baud rate %d for %s (B9600 used)\n",
                   baud_rate, port_name);
    }

	CommParam.fBinary=TRUE;					/* Binary Mode (skip EOF check)    */
	CommParam.fParity=PARITY_NONE;			/* Enable parity checking          */
	CommParam.fOutxCtsFlow=FALSE;			/* CTS handshaking on output       */
	CommParam.fOutxDsrFlow=FALSE;			/* DSR handshaking on output       */
	CommParam.fDtrControl=DTR_CONTROL_DISABLE;	/* DTR Flow control                */
	CommParam.fDsrSensitivity=FALSE;		/* DSR Sensitivity              */
	CommParam.fTXContinueOnXoff=FALSE;		/* Continue TX when Xoff sent */
	CommParam.fOutX=FALSE;					/* Enable output X-ON/X-OFF        */
	CommParam.fInX=FALSE;					/* Enable input X-ON/X-OFF         */
	CommParam.fErrorChar=FALSE;				/* Enable Err Replacement          */
	CommParam.fNull=FALSE;					/* Enable Null stripping           */
	CommParam.fRtsControl=RTS_CONTROL_DISABLE;	/* Rts Flow control                */
	CommParam.fAbortOnError=FALSE;			/* Abort all reads and writes on Error */
//	CommParam.fDummy2;						/* Reserved                        */
//	CommParam.wReserved;					/* Not currently used              */
//	CommParam.XonLim;						/* Transmit X-ON threshold         */
// 	CommParam.XoffLim;						/* Transmit X-OFF threshold        */
	CommParam.ByteSize=8;					/* Number of bits/byte, 4-8        */
	CommParam.Parity=EVENPARITY;				/* 0-4=None,Odd,Even,Mark,Space    */
	CommParam.StopBits=ONESTOPBIT;			/* 0,1,2 = 1, 1.5, 2               */
	CommParam.XonChar=0;					/* Tx and Rx X-ON character        */
	CommParam.XoffChar=0;					/* Tx and Rx X-OFF character       */
	CommParam.ErrorChar=0;					/* Error replacement char          */
	CommParam.EofChar=0;					/* End of Input character          */
	CommParam.EvtChar=0;					/* Received Event character        */
//	CommParam.wReserved1;					/* Fill for now.                   */

	if(!SetCommState(fd, &CommParam))
		return -1;

	CommTimeouts.ReadIntervalTimeout=0;			/* Maximum time between read chars. */
	CommTimeouts.ReadTotalTimeoutMultiplier=0;		/* Multiplier of characters.        */
	CommTimeouts.ReadTotalTimeoutConstant=100;		/* Constant in milliseconds.        */
	CommTimeouts.WriteTotalTimeoutMultiplier=0;		/* Multiplier of characters.        */
	CommTimeouts.WriteTotalTimeoutConstant=10000;		/* Constant in milliseconds.        */

	if(!SetCommTimeouts(fd, &CommTimeouts))
		return -1;

	PurgeComm(fd, PURGE_RXCLEAR | PURGE_RXABORT);
	PurgeComm(fd, PURGE_TXCLEAR | PURGE_TXABORT);

	//Clear RTS
	if(!EscapeCommFunction(fd, CLRRTS))
	{
		DWORD error = GetLastError();
		fprintf(stderr,"Set RTS failed with error = %d\n", error);
		fflush(stderr);
	}

/*
	char buf[256];
	sprintf(buf, "%s", (const char *)port_name);

	wait_event = CreateEventA(NULL, true, false, buf);

	if (GetLastError() == ERROR_ALREADY_EXISTS) 
	{
		WaitForSingleObject(wait_event, 0);
	}

	if (!wait_event) 
	{
		CloseHandle(wait_event);
	}
*/

	return (int)fd;

#else //__unix__
        
        int fd;
        struct termios config;

		l_baud_rate = baud_rate;
        
        fd = open(port_name, O_RDWR | O_NOCTTY);
        
        if (fd == (int) INVALID_HANDLE_VALUE)
        {
                return -1;
        }
       
        if (!isatty(fd))
        {
                return -1;
        }
        
        if(tcgetattr(fd, &config) == -1)
        {
               return -1;
        }
        else
        {
			fprintf(stderr,"Serial port %s opened\n", port_name);
			fflush(stderr);
        }
        
        cfsetispeed(&config, B9600);
        cfsetospeed(&config, B9600);
        
        config.c_cflag |= (CLOCAL | CREAD);
        
        config.c_cflag &= ~CSIZE;                                         /* Mask the character size bits */
        config.c_cflag |=  (PARENB | CS8);                                                  /* Parity bit Select 8 data bits */
        
        config.c_cflag &= ~(PARODD | CSTOPB);                  /* even parity, 1 stop bit */
        
#ifdef CNEW_RTSCTS
        config.c_cflag |= CNEW_RTSCTS;                        /*enable RTS/CTS flow control - linux only supports rts/cts*/
#elif defined(CRTSCTS)
        config.c_cflag |= CRTSCTS;                                  /*enable RTS/CTS flow control - linux only supports rts/cts*/
#else
        fprintf(stderr,"\nOS does not support hardware flow control.\n");
        fflush(stderr);
        
        close(fd);
        return -1;
#endif
        
        config.c_iflag &= ~(IXON | IXOFF | IXANY);                        /*disable software flow control*/ 
        
        config.c_oflag &= ~OPOST;                                                  /* enable raw output */
        config.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);      /* enable raw input */
        
        config.c_iflag &= ~(INPCK | PARMRK);                                                    /* DANGEROUS no parity check*/
        config.c_iflag &= ~ISTRIP;                                                       /* strip parity bits */
        config.c_iflag |= IGNPAR;                                                        /*DANGEROUS ignore parity errors*/
        
        config.c_cc[VTIME] = 1;                                                         /*timeout to read a character in tenth of a second*/
          
        if(tcsetattr(fd, TCSANOW, &config) == -1)
        {
                fprintf(stderr,"\nCould not set serial port configuration.\n");
                fflush(stderr);
        
                close(fd);
                return -1;
        }
    
		tcflush(fd, TCIOFLUSH );
        
        return fd;
#endif //__unix__
}

int read_from_serial(HANDLE fd, unsigned char* in_buf, unsigned long *bytesread, unsigned max_legth, int read_timeout)
{

#ifdef WIN32

	COMSTAT     Our_Comstat;
	DWORD       Com_Errors, BytesToRead;
	int ret, ret1, ret_bool;
	int t1, t2;
	
	*bytesread = 0;
	
	//
	// Sanity check on the Handle
	//
	if ((fd == NULL) || (fd == INVALID_HANDLE_VALUE)) 
	{
		ret = 1;
		return ret;
	} 

	//
	// By far the most efficient way to input Comport data is the following:
	// ---------------------------------------------------------------------
	// A) See how much data is present
	// B) Read exactly that amount of data
	//
	// Note: It is possible to ask for more data than is present, but the ReadFile() 
	//       function call will not return until that amount of data has been received.
	//       Some programs will request more data and establish a timeout using the 
	//       SetCommTimeouts() function to return from the ReadFile() function when
	//       not enough data has been receive. Although this may work, it is inefficient
	//       and generally works like crap. For everyone's sake, use the technique above 
	//       and handle your own timeout in applications code.
	//

	//
	// Check the Input buffer
	//

	t1 = timeGetTime();

	do
	{
		Sleep(1);

		ret = ClearCommError (fd, &Com_Errors, &Our_Comstat);

		if(!ret)
		{
			printf("ClearCommError GetLastError() = %d", GetLastError());
			ret1 = 1;
			return ret1;
		}

		//printf("ClearCommError Com_Errors = %d", Com_Errors);

		//
		// ReadFile() only if there is data.
		//

		if(Our_Comstat.cbInQue > 0) 
		{
			//t3 = timeGetTime();
			
			//printf("time spent since write to read = %d\n", t3 - twrite);
			//printf("time spent waiting chars = %d\n", t2 - t1);
			//printf("time spent for write = %d\n", t3 - twrite - t2 + t1);

			//printf("Our_Comstat.cbInQue = %d\n", Our_Comstat.cbInQue);
			//
			// Make sure we do not overrun inbuffer
			//
			if (Our_Comstat.cbInQue > max_legth) 
			{
				BytesToRead = max_legth;
			} 
			else 
			{
				BytesToRead = Our_Comstat.cbInQue;
			}
			
			//
			// Read the data, (NULL) no Overlapped I/O
			//
			ret_bool = ReadFile (fd, in_buf, BytesToRead, bytesread, NULL);

			if(!ret_bool && GetLastError() != ERROR_IO_PENDING)
			{
				printf("Errore nella ReadFile GetLastError() %d\n", GetLastError());

				ret1 = 1;
				
				return ret1;
			}

			fprintf(stderr,"ReadFile bytesread = %d\n", *bytesread);
			fflush(stderr);
		} 
		else 
		{
			*bytesread = 0;
		}
		
		t2 = timeGetTime();

	}while ((*bytesread <= 0) && (t2 - t1 < read_timeout));

	return 0;

#else //__unix__
	//TODO : Add __unix__ serial implementation
        
        struct timeval t1, t2;
        int BytesToRead;
        
        //
        // Sanity check on the Handle
        //
        if ((&fd == NULL) || (fd == INVALID_HANDLE_VALUE) || !isatty((int)fd)) 
        {
                return 1;
        } 
        
               	
        //
        // Check the Input buffer
        //

        gettimeofday(&t1, NULL);
        
        do
        {
                Sleep(1);
                  
                ioctl((int) fd, FIONREAD, &BytesToRead);

                if (BytesToRead >0)
                {
					//
					// Make sure we do not overrun inbuffer
					//
					if (BytesToRead > max_legth) 
					{
						BytesToRead = max_legth;
					} 

					 *bytesread = read((int) fd, in_buf, BytesToRead);
             
					 if (*bytesread == -1) {
							fprintf(stderr,"\nError while reading from serial port.: errno %i\n", errno);
							fflush(stderr);

							return 1;
					 }
             
					 printf("read bytesread = %d\n", *bytesread);
	
                }
                else
                {
                        *bytesread = 0;
                }
		
                gettimeofday(&t2, NULL);

        }while ((*bytesread <= 0) && (t2.tv_sec - t1.tv_sec < 5));

        return 0;
#endif //__unix__
}

unsigned int write_to_serial(HANDLE	fd, unsigned char	*buffDati, unsigned int n_byte_to_write, int rtsOnTime, int rtsOffTime)
{

#ifdef WIN32

	unsigned int n_written = 0;
	int i_rtsSendTime;
	double rtsSendTime = 0;
	double time_per_char = 11.0*1200.0/((double)l_baud_rate);
	//unsigned int t_start_1 = 0;
	//unsigned int t_start_2 = 0;
	//unsigned int t_stop = 0;

	if(fd == NULL)
		return 0;

	PurgeComm(fd, PURGE_TXCLEAR | PURGE_TXABORT);
		
	if(!EscapeCommFunction(fd, SETRTS))
	{
		DWORD error = GetLastError();
		fprintf(stderr,"Set RTS failed with error = %d\n", error);
		fflush(stderr);
	}
	else
	{
		//fprintf(stderr,"rtsOnTime in ms = %d\n", rtsOnTime);
		//fflush(stderr);
		Sleep(rtsOnTime); //in ms
		//WaitForSingleObject(wait_event, rtsOnTime);
	}
	
	//t_start_2 = timeGetTime();

	if(!WriteFile(fd, buffDati, n_byte_to_write, (unsigned long *)&n_written, NULL))
	{
		//t_stop = timeGetTime();
		return 0;
	}

	//t_stop = timeGetTime();
	
	if(rtsOnTime || rtsOffTime)
	{
		rtsSendTime = time_per_char*(double)n_written;

		//fprintf(stderr,"double rtsSendTime in ms = %lf\n", rtsSendTime);
		//fflush(stderr);
		i_rtsSendTime = (int)ceil(rtsSendTime);
		
		//fprintf(stderr,"rtsSendTime in ms = %d\n", i_rtsSendTime);
		//fflush(stderr);

		Sleep(i_rtsSendTime); //in ms
		//WaitForSingleObject(wait_event, (int)rtsSendTime);
	}

	//fprintf(stderr,"rtsOffTime in ms = %d\n", rtsOffTime);
	//fflush(stderr);

	Sleep(rtsOffTime); //in ms
	//WaitForSingleObject(wait_event, rtsOffTime);

	if(!EscapeCommFunction(fd, CLRRTS))
	{
		DWORD error = GetLastError();
		fprintf(stderr,"Set RTS failed with error = %d\n", error);
		fflush(stderr);
	}
	
	return n_written;

#else //__unix__
	unsigned int n_written = 0;

                  if(fd == NULL)
					return 0;

				  tcflush(fd, TCIOFLUSH );

                  n_written = write((int)fd, buffDati,  n_byte_to_write);
                  
                  if (n_written == -1) {
                        fprintf(stderr,"\nError while writing serial port: errno %i\n", errno);
                        fflush(stderr);

                        return 0;
                  }

	return n_written;
#endif //__unix__
}

void close_port(int fd)
{
#ifdef WIN32
	if(fd)
	{
		PurgeComm((HANDLE)fd, PURGE_RXCLEAR | PURGE_RXABORT);
		PurgeComm((HANDLE)fd, PURGE_TXCLEAR | PURGE_TXABORT);
		CloseHandle((HANDLE)fd);
	}

#else //__unix__

		if (fd) 
		{
			tcflush(fd, TCIOFLUSH );
            close(fd);
		}

#endif //__unix__
}