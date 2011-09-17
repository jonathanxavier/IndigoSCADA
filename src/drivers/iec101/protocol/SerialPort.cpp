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

#include "SerialPort.h"
#include "Global.h"

CSerialPort::CSerialPort(ST_COM_INFO st_cominfo)
{
		com_info = st_cominfo;
		m_hEventKill = CreateEvent(NULL, TRUE, FALSE, NULL);
		bConn_com = 0;
		iErr_com =0;
	
		memset(&m_osRead,0,sizeof(OVERLAPPED));
		memset(&m_osWrite,0,sizeof(OVERLAPPED));
		memset(&m_osEvent,0,sizeof(OVERLAPPED));
		
		m_osEvent.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);	

		//-----------file transfer
		bStartSendFile = FALSE;
}

BOOL CSerialPort::OpenComm()
{
	m_nCommState=COM_FREE;
	
	char       szPort[10];		
	memset(szPort,0,10);
	sprintf(szPort,"COM%d",com_info.bPort);
	
	m_hComm = CreateFile(szPort,GENERIC_READ|GENERIC_WRITE,0,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,NULL);

	if(!m_hComm)
	{  
		char *ch_info = new char[1024];
		memset(ch_info,0,1024);
		sprintf(ch_info,"");
    	return false;
	}

	if(m_hComm!=INVALID_HANDLE_VALUE)
	{
		bConn_com=true;
		char *ch_info = new char[24];
		memset(ch_info,0,24);
		sprintf(ch_info,"");
	//	AddToRunInfo(ch_info);
		SetupComm(m_hComm,DFLT_SEND_BUF_LEN,DFLT_RECV_BUF_LEN);
		
		DCB dcb;
		GetCommState(m_hComm,&dcb);
		
		dcb.BaudRate=com_info.dwBaudRate;
		dcb.fBinary = true;
		dcb.fParity = FALSE;
		dcb.ByteSize = 8;
		dcb.Parity = com_info.bParity;
		dcb.StopBits = ONESTOPBIT;
		dcb.fOutxCtsFlow = false;
		dcb.fInX = false;
		dcb.fOutX = false;
		bool fRetVal = SetCommState(m_hComm,&dcb);
		if(!fRetVal)
		{
			int errcode = GetLastError();
			CloseHandle(m_hComm);
			m_hComm=INVALID_HANDLE_VALUE;
			//	DisplayMessage(0,"Setup Comm State Error!");
			return FALSE;
		}
		PurgeComm(m_hComm,PURGE_RXCLEAR);
		PurgeComm(m_hComm,PURGE_TXCLEAR);

		COMMTIMEOUTS wcommtimeouts;
		GetCommTimeouts(m_hComm,&wcommtimeouts);
		wcommtimeouts.ReadIntervalTimeout =50;
		wcommtimeouts.ReadTotalTimeoutConstant= 150;
		wcommtimeouts.ReadTotalTimeoutMultiplier =10;
		SetCommTimeouts(m_hComm,&wcommtimeouts);
		
		if(!SetCommMask(m_hComm,EV_RXCHAR|EV_TXEMPTY|EV_DSR))  
		{
			DWORD dw = GetLastError();
			char szbuf[80];
			sprintf(szbuf,"",dw);
		}
	}
	else 
		bConn_com=false;
	return bConn_com;
}

BOOL CSerialPort::CloseComm()
{
	CloseHandle(m_hComm);
	char *ch_info = new char[1024];
	memset(ch_info,0,1024);
	sprintf(ch_info,"");
//	AddToRunInfo(ch_info);
	return TRUE;
}


DWORD CSerialPort::ReadComm(char * buf,int len,DWORD &dwErr)
{
	int result = 0;
	DWORD wCount = len;
	DWORD  dwInBytes = len;
	int fReadStat;
	fReadStat=ReadFile(m_hComm,(LPVOID)buf,len,&dwInBytes,&m_osRead); 
   	if(!fReadStat)
	{
		dwErr =GetLastError();
		switch(dwErr) {
		case ERROR_INVALID_HANDLE: 
			break;
		case ERROR_IO_PENDING:
			while(!GetOverlappedResult(m_hComm,&m_osRead,&dwInBytes,TRUE))
			{
				if(GetLastError()==ERROR_IO_INCOMPLETE) 
					continue;
			}
			break;
		default:
			break;
		}
	}
	
	return dwInBytes;
}


DWORD CSerialPort::WriteComm(char * msg,int len)
{
	int result = 0;
	DWORD wCount = len;
	result=WriteFile(m_hComm,msg,len,&wCount,&m_osWrite);
	if(!result)
		if (GetLastError()==ERROR_IO_PENDING)
		{
			while(!GetOverlappedResult(m_hComm,&m_osWrite,&wCount,TRUE))
			{
				if(GetLastError()==ERROR_IO_INCOMPLETE) 
					continue;
			}
		}
		
		return wCount;
		
}

bool CSerialPort::single_event_recv_step()
{
	
	WaitCommEvent(m_hComm,
		&dwEventMask,
		&m_osEvent
		);
	
	dwRes=::WaitForSingleObject(m_osEvent.hEvent,100);
	
	switch(dwRes)
	{
	case WAIT_OBJECT_0:
		{ 
			switch(dwEventMask)
			{
			case EV_RXCHAR:
				OnRecv();
				break;
			case EV_DSR:
			//	Log("data-set-ready line state changed");
				break;
			case EV_TXEMPTY:
				break;
			default:
				break;
			}
			
		}
		break;
	case 258:    //---------------------------The wait operation timed out.
		//	AfxMessageBox("258");
		break;;
	default:
		break;
	}
	
	
	return true;
}

int CSerialPort::single_event_recv_step(char *RecvBuf,CByteArray *RecvBufArray)
{
	int iReturn = 0;

	WaitCommEvent(m_hComm,      
		&dwEventMask,       
		&m_osEvent
		);
	
	dwRes=::WaitForSingleObject(m_osEvent.hEvent,100);          
	
	switch(dwRes)
	{
	case WAIT_OBJECT_0:
		{ 
			switch(dwEventMask)
			{
			case EV_RXCHAR:
				iReturn = OnRecv(RecvBuf,RecvBufArray);
				break;
			case EV_DSR:
				//Log("data-set-ready line state changed");
				break;
			case EV_TXEMPTY:

				break;
			default:
				break;
			}
			
		}
		break;
	case 258:    //---------------------------The wait operation timed out.
		//	AfxMessageBox("258");
		break;
	default:
		break;
	}
	
	
	return iReturn;
}



bool CSerialPort::single_send_step()
{
	int result =0;
	int len = 0;
	char msg[] = "hello";
	len = strlen(msg);
	result = WriteComm(msg,len);
	
	if (result!=len)
	{         
		iErr_com = 101;
		bConn_com = 0;
	}
	return true;
}

bool CSerialPort::OnRecv()
{
   	DWORD     dwLength = 0;
	COMSTAT   ComStat ;
	DWORD dwErrorFlags;
   	ClearCommError(m_hComm,&dwErrorFlags,&ComStat);	
	dwLength =ComStat.cbInQue;
	DWORD dwErr =0;
	
   int numread = ReadComm((char *)m_ByteRecvBuf,dwLength,dwErr);
	if ( numread!= dwLength) 
	{
		if (dwErr ==ERROR_INVALID_HANDLE) 
		{
			bConn_com = false;
			char *ch_info = new char[1024];
			memset(ch_info,0,1024);
			sprintf(ch_info,":%d<%d",numread,dwLength);
		//	AddToRunInfo(ch_info);		 
		}
	}
	else
	{
		CSingleLock singlelock(&mutex_Com_recv_array);	
		singlelock.Lock();
		for (int iTemp=0;iTemp<dwLength;iTemp++)	
			Com_recv_array.Add(m_ByteRecvBuf[iTemp]);

		singlelock.Unlock();
	
	}	
	
	return true;
}

int CSerialPort::OnRecv(char *RecvBuf,CByteArray *RecvBufArray)
{
   	DWORD     dwLength = 0;
	COMSTAT   ComStat ;
	DWORD dwErrorFlags;
   	ClearCommError(m_hComm,&dwErrorFlags,&ComStat);	
	dwLength =ComStat.cbInQue;
	DWORD dwErr =0;
	
	int numread = ReadComm((char *)RecvBuf,dwLength,dwErr);
	if ( numread!= dwLength) 
	{
		if (dwErr ==ERROR_INVALID_HANDLE) 
		{
			bConn_com = false;
			char *ch_info = new char[1024];
			memset(ch_info,0,1024);
			sprintf(ch_info,"",numread,dwLength);
			//	AddToRunInfo(ch_info);		 
		}
	}
	else
	{
	//	CSingleLock singlelock(&mutex_recvbuf_array);	
	//	singlelock.Lock();
	/*	for (int iTemp=0;iTemp<dwLength;iTemp++)	
			RecvBufArray->Add(RecvBuf[iTemp]);	
	*/
	//	singlelock.Unlock();
		
	}	
	
	return numread;
}
