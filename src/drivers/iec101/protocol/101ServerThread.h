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

#ifndef 101ServerThread
#define 101ServerThread

#include "Global.h"
#include "SerialPort.h"
#include "iec_def.h"
#include "Protocol_IEC101.h"

class C101ServerThread : public QThread
{
	protected:
	C101ServerThread();           // protected constructor used by dynamic creation
  	C101ServerThread(ST_Policy_Info *pPolicy);   
	// Attributes
	public:
	HANDLE	m_hEventKill;
	bool m_bReset;
	
	BYTE m_ByteSendBuf[RCVBUFSIZE];
	BYTE m_ByteRecvBuf[RCVBUFSIZE];
	CByteArray *m_RecvBufArray;
	CByteArray *m_RecvErrorArray;
	ST_COM_INFO com_info;
	CSerialPort *pserialport;
	CProtocol_IEC101 *pProtocol_101;
	ST_Policy_Info policy_info;
	time_t tLastBeckonTime;
	time_t tLastTransferDataTime;
	// Operations
	public:
	void Single_Send_Step();
	void Single_Recv_Step();

	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();

	// Implementation
	protected:
	virtual ~C101ServerThread();
};


#endif

