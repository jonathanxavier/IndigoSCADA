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

#include "101ClientThread.h"

C101ClientThread::C101ClientThread()
{
}

 C101ClientThread::C101ClientThread(ST_Policy_Info *pPolicy)
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
	m_ByteSendArray = new CByteArray();
	pserialport = new CSerialPort(com_info);
	  bool ret = pserialport->OpenComm();
	  if (ret==0) {
		 // MessageBinDebug("open serialport fail");
		  Log("open serialport fail");
		  m_bReset = 1;
	  }else m_bReset = 0;

    pProtocol_101 = new CProtocol_IEC101();
	  
 }

C101ClientThread::~C101ClientThread()
{
}

BOOL C101ClientThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int C101ClientThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// C101ClientThread message handlers
int C101ClientThread::Run()
{
   CString str_show = "";
   bool ret = false;
   int failnum = 0;
	while (WaitForSingleObject(m_hEventKill,200)!= WAIT_OBJECT_0) 
	{
	    str_show.Format("%d:%s--%s",policy_info.Policyid,policy_info.szLocalAppName,policy_info.szRemoteAppName);

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
			
			if (failnum>3) 
			{
			//	Log("open serialport fail after 3 times");
				break;
			}
		}
     
		  CExByteArray tempGram;
		  tempGram.RemoveAll();
		  Single_Recv_Step(tempGram);
		  Single_Send_Step(tempGram);
		  tempGram.RemoveAll();
     
	
	}
	return 1;
}

int C101ClientThread::GetObjectNum()
{
//	return iThreadNum;
	return 1;
}
CWinThread * C101ClientThread::CreateNewThread(ST_Policy_Info *pPolicy)
 {
	 C101ClientThread *pThread=new C101ClientThread(pPolicy);//,pDoc);
	 pThread->m_pThreadParams=NULL;
	 if(!pThread->CreateThread(CREATE_SUSPENDED))
	 {
		 delete pThread;
		 pThread=NULL;
		 return NULL;
	 }
	 VERIFY(pThread->SetThreadPriority(THREAD_PRIORITY_NORMAL));
	 pThread->ResumeThread();
	 return pThread;
 }

void C101ClientThread::DealNormalRecvBuf()
{

}

void C101ClientThread::DealErrorRecvArray()
{

}

void C101ClientThread::Single_Recv_Step(CExByteArray &ary)
{
	
	int iRecvNum = 0;
    bool ret = false;
	DWORD dwError = 0;

	if (m_bReset == false)
	iRecvNum = pserialport->single_event_recv_step((char*)m_ByteRecvBuf,m_RecvBufArray);
//	iRecvNum = pserialport->ReadComm((char*)m_ByteRecvBuf,m_RecvBufArray,dwError);
	
	CString str_temp = "";
	CString str_show = "";
//	if (m_RecvBufArray->GetSize()>0) 
	if (iRecvNum>0)
	{
       for(int i=0;i<iRecvNum;i++)
	   {
		   str_temp.Format("%02X ",m_ByteRecvBuf[i]);
		   str_show += str_temp;
	   }
		m_pShowDlg->show_policy_recv_stat("com_recv",str_show);

		ret = pProtocol_101->CheckCRC(m_ByteRecvBuf);
		if (ret==false) 
		{
			for(int i=0;i<iRecvNum;i++)
				m_RecvErrorArray->Add(m_ByteRecvBuf[i]);
		}
		else 
			pProtocol_101->DealNormalRecvBuf_feedback(&policy_info,m_ByteRecvBuf,ary);
				
	}

	
	if (m_RecvErrorArray->GetSize()>0) 
	{
		pProtocol_101->DealErrorRecvArray(&policy_info,m_RecvErrorArray);
	}

}

void C101ClientThread::Single_Send_Step(CExByteArray &ary)
{
  	int len = 0;
	
	if (policy_info.iLink_Stat ==0&&(tLastTransferDataTime+5<time(NULL))) 
	{
		len = pProtocol_101->Master_Call_Link_Stat(m_ByteSendBuf,RCVBUFSIZE,2);
		pserialport->WriteComm((char*)m_ByteSendBuf,len);
		tLastTransferDataTime = time(NULL);
	}
   if (policy_info.iLink_Stat == 1&&(tLastTransferDataTime+5<time(NULL)) )
   {
     len = pProtocol_101->Master_Beckon_Data(&policy_info,m_ByteSendBuf,RCVBUFSIZE);
	pserialport->WriteComm((char*)m_ByteSendBuf,len);
	tLastTransferDataTime = time(NULL);
	
   }
   len = ary.GetSize();
   if (len>0)
   {
	   BYTE *m_Byte = new BYTE[len];
	   ary.GetBuf(0,m_Byte,len);
     pserialport->WriteComm((char*)m_Byte,len);
	 delete m_Byte;
   }
  
   
/*

   len = pProtocol_101->Slave_Resp_Master_Reset_Link(&policy_info,m_ByteSendBuf,RCVBUFSIZE);
    pserialport->WriteComm((char*)m_ByteSendBuf,len);
	
    len = pProtocol_101->Master_Reset_Link(&policy_info,m_ByteSendBuf,RCVBUFSIZE);
    //----	m_pShowDlg->show_policy_send_stat("send","");
     pserialport->WriteComm((char*)m_ByteSendBuf,len);

	len = pProtocol_101->Slave_Resp_Master_Link_ACK(m_ByteSendBuf,RCVBUFSIZE,2);
	pserialport->WriteComm((char*)m_ByteSendBuf,len);

   len = pProtocol_101->Master_Send_ASDU36_SinglePoint(&policy_info,m_ByteSendBuf,RCVBUFSIZE,3);
	pserialport->WriteComm((char*)m_ByteSendBuf,len);

	len = pProtocol_101->Master_Send_ASDU36_MultyPoint(&policy_info,m_ByteSendBuf,RCVBUFSIZE,12);
	pserialport->WriteComm((char*)m_ByteSendBuf,len);

  len = pProtocol_101->Master_Beckon_Data(&policy_info,m_ByteSendBuf,RCVBUFSIZE);
  pserialport->WriteComm((char*)m_ByteSendBuf,len);
*/	

}
