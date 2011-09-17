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

#include "Protocol_IEC101.h"
#include "iec_def.h"
#include <math.h>
#include "Global.h"

CProtocol_IEC101::CProtocol_IEC101()
{
}

CProtocol_IEC101::CProtocol_IEC101(ST_Policy_Info *pPolicy)
{
	iCOTAddrNum  = pPolicy->protocol_info.pro_iec101.nCOTAddrNum;
    iCOAAddrNum  = pPolicy->protocol_info.pro_iec101.nCOAAddrNum;
    iINFOAddrNum = pPolicy->protocol_info.pro_iec101.nINFOAddrNum;
}

/*
bool CProtocol_IEC101::CheckCRC(CByteArray *m_RecvBufArray)
{
	bool bReturn = false;
	int nCount = m_RecvBufArray->GetSize();
	BYTE m_ByteRecvIndex = 0x00;
	int i = 0;
	Log("begin to deal");
	while (i<nCount) 
	{
		m_ByteRecvIndex = m_RecvBufArray->GetAt(i);
		if(m_ByteRecvIndex==LPCI_SYN)
		{
			if(m_RecvBufArray->GetAt(i+4)!=LPCI_ETX)
				return FALSE;
			if(m_RecvBufArray->GetAt(i+3)!=Sum(&m_ByteRecvBuf[1],2))
				return FALSE;
		}
	
		else if(m_ByteRecvBuf[0]==LPCI_STX)   
		{
			int nLen=m_ByteRecvBuf[1];   
			if(m_ByteRecvBuf[nLen+5]!=LPCI_ETX)
				return FALSE;
			if(m_ByteRecvBuf[nLen+4]!=Sum(&m_ByteRecvBuf[4],nLen))
				return FALSE;
		}
		else if(m_ByteRecvBuf[0]==LPCI_NUL)
			return TRUE;
		else
			return FALSE;
		return TRUE;
	}
	return bReturn;
}
*****************************************************/


bool CProtocol_IEC101::CheckCRC(BYTE *m_ByteRecvBuf)
{
	if(m_ByteRecvBuf[0]==LPCI_SYN)
	{
		if(m_ByteRecvBuf[4]!=LPCI_ETX)
			return FALSE;
		if(m_ByteRecvBuf[3]!=Sum(&m_ByteRecvBuf[1],2))
			return FALSE;
	}

	else if(m_ByteRecvBuf[0]==LPCI_STX)   
	{
		int nLen=m_ByteRecvBuf[1];   
		if(m_ByteRecvBuf[nLen+5]!=LPCI_ETX)   
			return FALSE;
		if(m_ByteRecvBuf[nLen+4]!=Sum(&m_ByteRecvBuf[4],nLen))
			return FALSE;
	}
	else if(m_ByteRecvBuf[0]==LPCI_NUL)
		return TRUE;
	else
		return FALSE;
//	return TRUE;
}

BYTE CProtocol_IEC101::Sum(BYTE *buf, int len)
{
	BYTE sum=0;
	for(int i=0;i<len;i++)
		sum+=buf[i];
	return sum;
}


void CProtocol_IEC101::DealErrorRecvArray(ST_Policy_Info *pPolicy,CByteArray *m_RecvErrorArray)
{
 // Log("deal error recv buf");
	sprintf(pPolicy->CharLog,"");
	m_pShowDlg->show_policy_recv_stat("error",pPolicy->CharLog);
m_RecvErrorArray->RemoveAll();
}

void CProtocol_IEC101::DealNormalRecvBuf(ST_Policy_Info *pPolicy,BYTE *m_ByteRecvBuf)
{
//	Log("deal normal recv buf");
	switch(pPolicy->protocol_info.pro_iec101.stationtype) 
	{
	case 0:
		{
			if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_NUL)))
			{

			}
			else if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_SYN)))
			{
				int ADDR=m_ByteRecvBuf[2];

				if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x0B)
				{
					sprintf(pPolicy->CharLog,"", pPolicy->protocol_info.pro_iec101.local_stationid,pPolicy->bLink_ResetSuccess);

					m_pShowDlg->show_policy_recv_stat("解析",pPolicy->CharLog);

					pPolicy->iLink_Stat = 1;
				}

				if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x00)
				{
					pPolicy->bLink_ResetSuccess = 1;
					sprintf(pPolicy->CharLog,"",
						pPolicy->protocol_info.pro_iec101.local_stationid,pPolicy->bLink_ResetSuccess);
					m_pShowDlg->show_policy_recv_stat("解析",pPolicy->CharLog);
				}
				if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x01)
				{
					pPolicy->bLink_ResetSuccess = 0;
					sprintf(pPolicy->CharLog,"",
						pPolicy->protocol_info.pro_iec101.local_stationid,ADDR,pPolicy->bLink_ResetSuccess);
					m_pShowDlg->show_policy_recv_stat("解析",pPolicy->CharLog);
				}
				if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x09)
				{
					
					
				}
			}
			else if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_STX)))
			{
				int ADDR=m_ByteRecvBuf[5];
				switch (m_ByteRecvBuf[6])
				{
				case M_SP_NA:
					break;
				case M_SP_TA:
					break;
				case M_ME_NC:
					//  M_ME_NC_ASDU13();
					break;
					
				default:
					break;
				}
			}
		}
		break;
	case 1:
		{
			if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_NUL)))
			{
			
			}
			else if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_SYN)))
			{
               			int ADDR=m_ByteRecvBuf[2];

				if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x09 && m_ByteRecvBuf[1]&0x40==(BYTE)0x40)
				{
					sprintf(pPolicy->CharLog,"",	pPolicy->protocol_info.pro_iec101.local_stationid);

					m_pShowDlg->show_policy_recv_stat("解析",pPolicy->CharLog);

					pPolicy->iLink_Stat=1;	

					Slave_Resp_Master_Link_ACK(m_ByteSendBuf,1024,pPolicy->protocol_info.pro_iec101.local_stationid);
				}

				if ((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x09 && m_ByteRecvBuf[1]&0x40==(BYTE)0x60)
				{
					sprintf(pPolicy->CharLog,"",pPolicy->protocol_info.pro_iec101.Remote_stationid);
					pPolicy->iLink_Stat=1;	
					ACD=0x20;
					Slave_Resp_Master_Link_ACK(m_ByteSendBuf,1024,pPolicy->protocol_info.pro_iec101.local_stationid);
				}
			}
			else if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_STX)))
			{
			}
				int ADDR=m_ByteRecvBuf[2];
		}
		break;
	default:
		break;
	}


}


 void CProtocol_IEC101::DealNormalRecvBuf_feedback(ST_Policy_Info *pPolicy,BYTE *m_ByteRecvBuf,CExByteArray &ary)
 {
	 switch(pPolicy->protocol_info.pro_iec101.stationtype) 
	 {
	 case 0:
		 {
			 if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_NUL)))
			 {

			 }
			 else if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_SYN)))
			 {
				 int ADDR=m_ByteRecvBuf[2];
				 if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x0B)
				 {
					 sprintf(pPolicy->CharLog,"", pPolicy->protocol_info.pro_iec101.local_stationid,pPolicy->bLink_ResetSuccess);
					 m_pShowDlg->show_policy_recv_stat("解析",pPolicy->CharLog);
					 pPolicy->iLink_Stat = 1;
				 }

				 if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x00)
				 {
					 pPolicy->bLink_ResetSuccess = 1;
					 sprintf(pPolicy->CharLog,"", pPolicy->protocol_info.pro_iec101.local_stationid,pPolicy->bLink_ResetSuccess);

					 m_pShowDlg->show_policy_recv_stat("解析",pPolicy->CharLog);
				 }

				 if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x01)
				 {
					 pPolicy->bLink_ResetSuccess = 0;
					 sprintf(pPolicy->CharLog,"", pPolicy->protocol_info.pro_iec101.local_stationid,ADDR,pPolicy->bLink_ResetSuccess);

					 m_pShowDlg->show_policy_recv_stat("",pPolicy->CharLog);
				 }

				 if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x09)//无所召唤的数据
				 {
					 
					 
				 }
			 }
			 else if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_STX)))
			 {
				 int ADDR=m_ByteRecvBuf[5];

				 switch (m_ByteRecvBuf[6])
				 {
				 case M_SP_NA:
					 break;
				 case M_SP_TA:
					 break;
				 case M_ME_NC:
					 //  M_ME_NC_ASDU13();
					 break;
					 
				 default:
					 break;
				 }
			 }
		 }  
		 break;
	 case 1:
		 {
			 if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_NUL)))
			 {
		
			 }
			 else if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_SYN)))//固定帧长度
			 {
				 int ADDR=m_ByteRecvBuf[2];

				 if((BYTE)(m_ByteRecvBuf[1]& 0x0f)==(BYTE)0x09 && m_ByteRecvBuf[1]&0x40==(BYTE)0x40)

				 {
					 sprintf(pPolicy->CharLog,"", pPolicy->protocol_info.pro_iec101.local_stationid);

					 m_pShowDlg->show_policy_recv_stat("解析",pPolicy->CharLog);

					 pPolicy->iLink_Stat=1;

				         int iSendBytes = Slave_Resp_Master_Link_ACK(m_ByteSendBuf,1024,pPolicy->protocol_info.pro_iec101.local_stationid);

					ary.Add(m_ByteSendBuf,iSendBytes);	 
				 }
			 }
			 else if(((BYTE)(m_ByteRecvBuf[0]))==((BYTE)(LPCI_STX)))
			 {
			 }

			 int ADDR=m_ByteRecvBuf[2];
		 }
		 break;
	 default:
		 break;
	 }
 }

int CProtocol_IEC101::Master_Call_Link_Stat(BYTE *SendBuf,int buflen,int slave_sid)
{
   int len = 5;
   memset(SendBuf,5,0);

   DIR = DIR_MASTER_TO_SLAVE;
   PRM = PRM_MASTER;
   FCB = FCB_MASTER_SAMESEND;
   //FCV = FCV_VALID;
   FCV = FCV_INVALID;

   FUNC = FUNC_CALL_LINK_STAT;
  // FUNC = 0x09;

   SendBuf[0] = LPCI_SYN;
   SendBuf[1] = DIR | PRM | FCB | FCV | FUNC;
   //--0 1 0 0 
   SendBuf[2] = slave_sid;
   SendBuf[3] = Sum(&SendBuf[1],2);
   SendBuf[4] = LPCI_ETX;
  // MessageBinDebug((char*)SendBuf,len);
   	m_pShowDlg->show_policy_send_stat("send","");
   return len;
}


int CProtocol_IEC101::Master_Reset_Link(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen)
{
	int len = 5;
	memset(m_ByteSendBuf,5,0);

	DIR = DIR_MASTER_TO_SLAVE;
	PRM = PRM_MASTER;
	FCB = FCB_MASTER_SAMESEND;
	//FCV = FCV_VALID;
	FCV = FCV_INVALID;
	
	FUNC = FUNC_RESET_LINK;
	// FUNC = 0x00 ;

	m_ByteSendBuf[0] = LPCI_SYN;
	m_ByteSendBuf[1] = DIR | PRM | FCB | FCV | FUNC;
	//--0 1 0 0 
	m_ByteSendBuf[2] = pPolicy->protocol_info.pro_iec101.Remote_stationid;
	m_ByteSendBuf[3] = Sum(&m_ByteSendBuf[1],2);
	m_ByteSendBuf[4] = LPCI_ETX;
	// MessageBinDebug((char*)SendBuf,len);
   	m_pShowDlg->show_policy_send_stat("send","");
	return len;
}


int CProtocol_IEC101::Slave_Resp_Master_Link_ACK(BYTE *m_ByteSendBuf,int buflen,int slave_sid)
{
   int len = 5;
    memset(m_ByteSendBuf,buflen,0);
    m_ByteSendBuf[buflen] = '\0';

	DIR = DIR_SLAVE_TO_MASTER;
	PRM = PRM_SLAVE;
	FCB = FCB_MASTER_SAMESEND;
	//FCV = FCV_VALID;
	FCV = FCV_INVALID;

	FUNC = FUNC_ACK_LINK_STAT;
	
   m_ByteSendBuf[0] = LPCI_SYN;
   m_ByteSendBuf[1] = DIR | PRM | FCB | FCV | FUNC;
   m_ByteSendBuf[2] = slave_sid;
   m_ByteSendBuf[3] = Sum(&m_ByteSendBuf[1],2);
   m_ByteSendBuf[4] = LPCI_ETX;
   // MessageBinDebug((char*)m_ByteSendBuf,len);
   return len;
}
 int CProtocol_IEC101::Slave_Resp_Master_Reset_Link(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen,int iResetStat)
 {
	 int iReturnBufLen = 0 ;
	 ZeroMemory(m_ByteSendBuf,buflen);

	 DIR = DIR_SLAVE_TO_MASTER;
	 PRM = PRM_SLAVE;
	 FCB = FCB_MASTER_SAMESEND;
	 //FCV = FCV_VALID;
	 FCV = FCV_INVALID;
	 
	 FUNC = FUNC_RESET_LINK|iResetStat;
	 // FUNC = 0x00 ;

	 m_ByteSendBuf[0] = LPCI_SYN;
	 m_ByteSendBuf[1] = DIR | PRM | FCB | FCV | FUNC;
	 m_ByteSendBuf[2] = pPolicy->protocol_info.pro_iec101.local_stationid;
	 m_ByteSendBuf[3] = Sum(&m_ByteSendBuf[1],2);
	 m_ByteSendBuf[4] = LPCI_ETX;
	 // MessageBinDebug((char*)SendBuf,len);
	 m_pShowDlg->show_policy_send_stat("send","");
     iReturnBufLen = 5;
	 return iReturnBufLen;

 }

int CProtocol_IEC101::Master_Beckon_Data(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen)
{
	int iReturnBufLen = 0 ;
	ZeroMemory(m_ByteSendBuf,buflen);
	
	DIR = DIR_MASTER_TO_SLAVE;
	PRM = PRM_MASTER;
	FCB = FCB_MASTER_SAMESEND;
	FCV = FCV_VALID;
	
	FUNC = FUNC_SEND_NEED_RESP;

	m_ByteSendBuf[0]=LPCI_STX;
	m_ByteSendBuf[3]=LPCI_STX;
	m_ByteSendBuf[4] = DIR | PRM | FCB | FCV | FUNC;
	m_ByteSendBuf[5] = BYTE(pPolicy->protocol_info.pro_iec101.Remote_stationid);
	//m_ByteSendBuf[6] = TYPE_ALL_BECKON;
	m_ByteSendBuf[6] = C_IC_NA;
	m_ByteSendBuf[7] = SQ_NO_INFOOBJECT|VSQ_OEN_POINT;

	WORD wCOT = COT_BECKON_ACTIVE;
	nByteLength=8;
	iProtocolLen = pPolicy->protocol_info.pro_iec101.nCOTAddrNum;
	memcpy(&m_ByteSendBuf[nByteLength],&wCOT,iProtocolLen);
	nByteLength+=iProtocolLen;

	iRemote_Slave_Stationid = pPolicy->protocol_info.pro_iec101.Remote_stationid;
	iProtocolLen = pPolicy->protocol_info.pro_iec101.nCOAAddrNum;
	memcpy(&m_ByteSendBuf[nByteLength],&iRemote_Slave_Stationid,iProtocolLen);
	nByteLength += iProtocolLen;
	
	DWORD wAddress = 0;
	iProtocolLen =  pPolicy->protocol_info.pro_iec101.iInfoAddrNum_Beckon;
	memcpy(&m_ByteSendBuf[nByteLength],&wAddress,iProtocolLen);
	nByteLength += iProtocolLen;

	m_ByteSendBuf[nByteLength++] = 0x14;
	m_ByteSendBuf[nByteLength++] = Sum(&m_ByteSendBuf[4],nByteLength-4);
	m_ByteSendBuf[nByteLength++] = LPCI_ETX;

	m_ByteSendBuf[1]=nByteLength-6;
	m_ByteSendBuf[2]=nByteLength-6;
	
	iReturnBufLen = nByteLength;
	m_pShowDlg->show_policy_send_stat("send","");
	return iReturnBufLen;
}


int CProtocol_IEC101::Master_Send_ASDU36_SinglePoint(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen,int slave_sid)
{
	int len = 0;
   	ZeroMemory(m_ByteSendBuf,buflen);

	DIR = DIR_MASTER_TO_SLAVE;
	PRM = PRM_MASTER;
	FCB = FCB_MASTER_SAMESEND;
	//FCV = FCV_VALID;
	FCV = FCV_INVALID;  

	FUNC = FUNC_SEND_NO_RESP;

	m_ByteSendBuf[0]=LPCI_STX;
	m_ByteSendBuf[3]=LPCI_STX;
	m_ByteSendBuf[4] = DIR | PRM | FCB | FCV | FUNC;
	m_ByteSendBuf[5] = BYTE(slave_sid);
	m_ByteSendBuf[6] = 52;
	m_ByteSendBuf[7] = VSQ_SEND_SINGLE_POINT;

	WORD wCOT = COT_MASTER_ACTIVE;
	nByteLength = 8;
	iProtocolLen = pPolicy->protocol_info.pro_iec101.nCOTAddrNum;
   	memcpy(&m_ByteSendBuf[nByteLength],&wCOT,iProtocolLen);
	nByteLength += iProtocolLen;

	iLocal_Slave_Stationid = pPolicy->SID;
	iProtocolLen = pPolicy->protocol_info.pro_iec101.nCOAAddrNum;
	memcpy(&m_ByteSendBuf[nByteLength],&iLocal_Slave_Stationid,iProtocolLen);
	nByteLength += iProtocolLen;

	int iPointAddress = 1;

//	DWORD wAddress = iPointAddress + 0x4001;
	DWORD wAddress = iPointAddress + INFO_AI_S;
	iProtocolLen =  pPolicy->protocol_info.pro_iec101.nINFOAddrNum;
	memcpy(&m_ByteSendBuf[nByteLength],&wAddress,iProtocolLen);
	nByteLength += iProtocolLen;
    
	//--first data_value
   float fValue = 1234.12;
   if (pPolicy->SystemType ==0)
   {
	   *(float*)(m_ByteSendBuf+nByteLength) = fValue;
	   nByteLength+=4;
   }
   if (pPolicy->SystemType == 1)
   {
	   unsigned char bufchar[4];
	   *(float*)bufchar = fValue;
	   m_ByteSendBuf[nByteLength++]=bufchar[3];
	   m_ByteSendBuf[nByteLength++]=bufchar[2];
	   m_ByteSendBuf[nByteLength++]=bufchar[1];
	   m_ByteSendBuf[nByteLength++]=bufchar[0];
   }
 

    m_ByteSendBuf[nByteLength++]=0X00;

	COleDateTime  tDateTime = COleDateTime::GetCurrentTime();
	int nWeekday=tDateTime.GetDayOfWeek();
	m_ByteSendBuf[nByteLength++]=(tDateTime.GetSecond()*1000) % 256;
	m_ByteSendBuf[nByteLength++]=(tDateTime.GetSecond()*1000) / 256;
	m_ByteSendBuf[nByteLength]=(BYTE)tDateTime.GetMinute();
//	if(!m_bIsValidTime) m_ByteSendBuf[nByteLength]|=0X80;
	nByteLength++;
	m_ByteSendBuf[nByteLength++]=(BYTE)tDateTime.GetHour();
	m_ByteSendBuf[nByteLength]=(BYTE)tDateTime.GetDay();
	m_ByteSendBuf[nByteLength]|=(nWeekday<<5);
	nByteLength++;
	m_ByteSendBuf[nByteLength++]=(BYTE)tDateTime.GetMonth();
	m_ByteSendBuf[nByteLength++]=(BYTE)(tDateTime.GetYear() % 100);
 


	m_ByteSendBuf[nByteLength++]=Sum(&m_ByteSendBuf[4],nByteLength-4);
	m_ByteSendBuf[nByteLength++]=LPCI_ETX;
	//m_ByteSendBuf[1]=k*14+10;
	m_ByteSendBuf[1]=nByteLength-6;
	m_ByteSendBuf[2]=nByteLength-6;
    len = nByteLength;
//	MessageBinDebug((char*)m_ByteSendBuf,len);
	m_pShowDlg->show_policy_send_stat("send","send one point");
	return len;
}

int CProtocol_IEC101::Master_Send_ASDU36_MultyPoint(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen,int iPointNum)
{
  int iReturn = 0;
  ZeroMemory(m_ByteSendBuf,buflen);
  DIR = DIR_MASTER_TO_SLAVE;
  PRM = PRM_MASTER;
  FCB = FCB_MASTER_SAMESEND;
  //FCV = FCV_VALID;
  FCV = FCV_INVALID;
  
  FUNC = FUNC_SEND_NO_RESP;
  
  m_ByteSendBuf[0]=LPCI_STX;
  m_ByteSendBuf[3]=LPCI_STX;
  m_ByteSendBuf[4] = DIR | PRM | FCB | FCV | FUNC;
  m_ByteSendBuf[5] = pPolicy->SID;
  m_ByteSendBuf[6] = 52;
  m_ByteSendBuf[7] = VSQ_SEND_SINGLE_POINT | iPointNum;

  WORD wCOT = COT_MASTER_ACTIVE;
  nByteLength = 8;
  iProtocolLen = pPolicy->protocol_info.pro_iec101.nCOTAddrNum;
  memcpy(&m_ByteSendBuf[nByteLength],&wCOT,iProtocolLen);
  nByteLength += iProtocolLen;
  

  iLocal_Slave_Stationid = pPolicy->SID;
  iProtocolLen = pPolicy->protocol_info.pro_iec101.nCOAAddrNum;
  memcpy(&m_ByteSendBuf[nByteLength],&iLocal_Slave_Stationid,iProtocolLen);
  nByteLength += iProtocolLen;
  
  int iPointAddress = 1;
  DWORD wAddress = iPointAddress + INFO_AI_S;
  iProtocolLen =  pPolicy->protocol_info.pro_iec101.nINFOAddrNum;
  float fValue = 1234.12;

  int iPointIndex = 1;
  for(iPointIndex=1;iPointIndex<=iPointNum;iPointIndex++)
  {
    wAddress = iPointIndex + INFO_AI_S;
	memcpy(&m_ByteSendBuf[nByteLength],&wAddress,iProtocolLen);
	nByteLength += iProtocolLen;
	
	if (pPolicy->SystemType ==0)
	{
		*(float*)(m_ByteSendBuf+nByteLength) = fValue+iPointIndex;
		nByteLength+=4;
	}
	if (pPolicy->SystemType == 1)
	{
		unsigned char bufchar[4];
		*(float*)bufchar = fValue+iPointIndex;
		m_ByteSendBuf[nByteLength++]=bufchar[3];
		m_ByteSendBuf[nByteLength++]=bufchar[2];
		m_ByteSendBuf[nByteLength++]=bufchar[1];
		m_ByteSendBuf[nByteLength++]=bufchar[0];
	}
   	

    m_ByteSendBuf[nByteLength++]=0X00;

	COleDateTime  tDateTime = COleDateTime::GetCurrentTime();
	int nWeekday=tDateTime.GetDayOfWeek();
	m_ByteSendBuf[nByteLength++]=(tDateTime.GetSecond()*1000) % 256;
	m_ByteSendBuf[nByteLength++]=(tDateTime.GetSecond()*1000) / 256;
	m_ByteSendBuf[nByteLength]=(BYTE)tDateTime.GetMinute();
	//	if(!m_bIsValidTime) m_ByteSendBuf[nByteLength]|=0X80;
	nByteLength++;
	m_ByteSendBuf[nByteLength++]=(BYTE)tDateTime.GetHour();
	m_ByteSendBuf[nByteLength]=(BYTE)tDateTime.GetDay();
	m_ByteSendBuf[nByteLength]|=(nWeekday<<5);
	nByteLength++;
	m_ByteSendBuf[nByteLength++]=(BYTE)tDateTime.GetMonth();
	m_ByteSendBuf[nByteLength++]=(BYTE)(tDateTime.GetYear() % 100);
	 }
  m_ByteSendBuf[nByteLength++]=Sum(&m_ByteSendBuf[4],nByteLength-4);
  m_ByteSendBuf[nByteLength++]=LPCI_ETX;
  //m_ByteSendBuf[1]=k*14+10;
  m_ByteSendBuf[1]=nByteLength-6;
  m_ByteSendBuf[2]=nByteLength-6;
  iReturn = nByteLength;
  m_pShowDlg->show_policy_send_stat("send","send multy point");

	return iReturn;
}


void CProtocol_IEC101::M_SP_NA_ASDU1(ST_Policy_Info *pPolicy,BYTE *m_ByteRecvBuf)
{
  	unsigned char len=0,value=0,infonum=0,UsefulInfolen;
	WORD COTAddr =0,COAAddr = 0,INFOAddr=0 ;
	int i,iInfoBegin=0;
		
	UsefulInfolen = m_ByteRecvBuf[1]-4-iCOTAddrNum-iCOAAddrNum;

	infonum = m_ByteRecvBuf[7]& (~0x80);

	if(m_ByteRecvBuf[7]& (0x80))
	{
		if (UsefulInfolen!=infonum + iINFOAddrNum)
		{
			return;
		}
	}
	else
	{
		if (UsefulInfolen!=infonum * (iINFOAddrNum +1) )
		{
			return;
		}
	}


	for(i=0;i<iCOTAddrNum;i++)
		COTAddr=COTAddr+m_ByteRecvBuf[8+i]*pow(256,i);
	for(i=0;i<iCOAAddrNum;i++)
		COAAddr=COAAddr+m_ByteRecvBuf[8+iCOTAddrNum+i]*pow(256,i);
	
	iInfoBegin=8+iCOTAddrNum+COAAddr;

	if (m_ByteRecvBuf[7]& (0x80))
	{
		INFOAddr=0;
		for(i=0;i<iINFOAddrNum;i++)
		  INFOAddr=INFOAddr+m_ByteRecvBuf[iInfoBegin+i]*pow(256,i);
		for(i=0;i<infonum;i++)
		{
		}
	}
	else
	{
		for(i=0;i<infonum;i++)
		{
		}
	}
}

void CProtocol_IEC101::M_SP_TA_ASDU2(BYTE *m_ByteRecvBuf)
{
  
}

void CProtocol_IEC101::M_ME_NC_ASDU13(BYTE *m_ByteRecvBuf)
{  

}
