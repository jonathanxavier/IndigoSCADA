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

#ifndef 101ClientThread
#define 101ClientThread

#include "Global.h"
#include "SerialPort.h"
#include "iec_def.h"
#include "Protocol_IEC101.h"

typedef struct
{
	int            ID;
	COleDateTime  TIME;
	float          VALUE;
	int            RTUID;
	int            RTUSEN;
	int            DCHAR;	
}RLData;

class C101ClientThread : public QThread
{
	protected:
  	C101ClientThread(); 
	public:
	C101ClientThread(ST_Policy_Info *pPolicy); 
	// Attributes
	public:
	
	static int iThreadNum;
    HANDLE	m_hEventKill;
	bool m_bReset;

	BYTE m_ByteSendBuf[RCVBUFSIZE];
	BYTE m_ByteRecvBuf[RCVBUFSIZE];
	CByteArray *m_RecvBufArray;
	CByteArray *m_RecvErrorArray;
	CExByteArray *m_ByteSendExArray;
	CByteArray *m_ByteSendArray;
	ST_COM_INFO com_info;
    CSerialPort *pserialport;
	CProtocol_IEC101 *pProtocol_101;
	ST_Policy_Info policy_info;
	time_t tLastBeckonTime;
	time_t tLastTransferDataTime;
	// Operations
	public:
	void Single_Send_Step(CExByteArray &ary);
	void Single_Recv_Step(CExByteArray &ary);
	void DealErrorRecvArray();
	void DealNormalRecvBuf();
	static CWinThread *CreateNewThread(ST_Policy_Info *pPolicy);
	static int GetObjectNum();
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();
	// Implementation
	protected:
	virtual ~C101ClientThread();
	private:
};

/////////////////////////////////////////////////////////////////////////////

#endif