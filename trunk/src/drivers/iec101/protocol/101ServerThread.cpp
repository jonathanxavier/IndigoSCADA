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

#include "IEC101.h"
#include "101ServerThread.h"

C101ServerThread::C101ServerThread()
{
}

C101ServerThread::C101ServerThread(ST_Policy_Info *pPolicy)
{
	m_hEventKill = CreateEvent(NULL, TRUE, FALSE, NULL);
	tLastBeckonTime = time(NULL);
	tLastTransferDataTime = time(NULL);
    
	policy_info = *pPolicy;
	policy_info.Policyid = pPolicy->Policyid;
    sprintf(policy_info.szLocalAppName,"%s",pPolicy->szLocalAppName);
    sprintf(policy_info.szRemoteAppName,"%s",pPolicy->szRemoteAppName);
	// policy_info.channel_info = pPolicy->channel_info;
	//policy_info.protocol_info = pPolicy->protocol_info;
    int baud = pPolicy->channel_info.con.con_serialport.baud;
    com_info.dwBaudRate = pPolicy->channel_info.con.con_serialport.baud;
	com_info.bPort =  pPolicy->channel_info.con.con_serialport.port;
	com_info.bParity = pPolicy->channel_info.con.con_serialport.CheakType;
	m_RecvBufArray = new CByteArray;
	m_RecvBufArray->RemoveAll();
	m_RecvErrorArray = new CByteArray;
	m_RecvErrorArray->RemoveAll();
	pserialport = new CSerialPort(com_info);
	bool ret = pserialport->OpenComm();
	if (ret==0) {
		// MessageBinDebug("open serialport fail");
		Log("open serialport fail");
		m_bReset = 1;
	}else m_bReset = 0;
	
    pProtocol_101 = new CProtocol_IEC101();
}

C101ServerThread::~C101ServerThread()
{
}

BOOL C101ServerThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int C101ServerThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// C101ServerThread message handlers

int C101ServerThread::Run()
{
	bool ret = false;
	int failnum = 0;
	CString str_show = "";
	while (WaitForSingleObject(m_hEventKill,200)!= WAIT_OBJECT_0) 
	{
     Log("begin server thread");
	 str_show.Format("%d:%s--%s",policy_info.Policyid,policy_info.szLocalAppName,policy_info.szRemoteAppName);
	 //	Log(str_show);
	 while(m_bReset ==true) 
	 {
		 pserialport->CloseComm();
		 ret = pserialport->OpenComm();
		 
		 if (ret == false) 
		 {
             m_bReset=true;
			 failnum++;
		 }				
		 else m_bReset = false;
		 
		 if (failnum>3) {
			 //	Log("open serialport fail after 3 times");
			 break;
		 }
	 }
     
	 
	 Single_Recv_Step();
	 Single_Send_Step();
	}
	return 1;
}


void C101ServerThread::Single_Send_Step()
{

}
void C101ServerThread::Single_Recv_Step()
{
}