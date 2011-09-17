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

#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#define DFLT_RECV_BUF_LEN 4096
#define DFLT_SEND_BUF_LEN 4096

#define RCVBUFSIZE          1024
#define MSGBUFSIZE          1024
#define MAX_COMBUF_LEN      256
#define MaxListDsp          20

#define COM_FREE 0
#define COM_READ 1
#define COM_WRITE 2
#define COM_PROCESS 3

typedef struct{	
	BYTE          bPort;
	BOOL          fConnected,fXonXoff;
	BYTE          bByteSize,bFlowCtrl,bParity,bStopBits;
	DWORD         dwBaudRate;
	OVERLAPPED    osWrite,osRead;
}ST_COM_INFO;

class CSerialPort
{
public:
	CSerialPort(ST_COM_INFO );
	bool bStartSendFile;
// Attributes
public:
	HANDLE	m_hEventKill;
	HANDLE			m_hComm;
	OVERLAPPED     m_osRead,m_osWrite,m_osEvent;
	int m_nCommState,m_nAlive,m_nState;
	ST_COM_INFO  com_info;
	bool bConn_com;
	int  iErr_com;
	DWORD dwEventMask,dwRes;
	char m_ByteRecvBuf[4096];
	// Operations
	public:
   	BOOL OpenComm();
	BOOL CloseComm();

	DWORD WriteComm(char * buf,int len);
	DWORD ReadComm(char * buf,int len,DWORD &dwErr);

	bool single_send_step();
	bool single_event_recv_step();
	bool OnRecv();

	int single_event_recv_step(char *RecvBuf,CByteArray *RecvBufArray);
	int OnRecv(char *RecvBuf,CByteArray *RecvBufArray);
	CMutex mutex_recvbuf_array;

	protected:
	private:
};

#endif