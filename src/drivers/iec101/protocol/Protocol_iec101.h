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

#ifndef PROTOCOL_IEC101_H
#define PROTOCOL_IEC101_H

#include "Protocol_base.h"
#include "iec_def.h"
#include "Global.h"

class CProtocol_IEC101:public CProtocol_Base
{

//attribute
public:

   BYTE DIR,RES,PRM,FCB,FCV,FUNC,ACD,DFC;
   int nByteLength,iProtocolLen;
   int iLocal_Slave_Stationid;
   int iRemote_Slave_Stationid;
   int iCOTAddrNum;
   int iCOAAddrNum;
   int iINFOAddrNum;
   BYTE m_ByteSendBuf[1024];
public:
	void M_ME_NC_ASDU13(BYTE *m_ByteRecvBuf);
	CProtocol_IEC101(ST_Policy_Info *pPolicy);
	void M_SP_TA_ASDU2(BYTE *m_ByteRecvBuf);  
	void M_SP_NA_ASDU1(ST_Policy_Info *pPolicy,BYTE *m_ByteRecvBuf);
	int Master_Call_Link_Stat(BYTE *SendBuf,int buflen,int slave_sid);
	int Slave_Resp_Master_Link_ACK(BYTE *m_ByteSendBuf,int buflen,int slave_sid);
    
	int Master_Reset_Link(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen);
    int Slave_Resp_Master_Reset_Link(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen,int iResetStat);

	int Master_Beckon_Data(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen);

	int Master_Send_ASDU36_SinglePoint(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen,int slave_sid);

	int Master_Send_ASDU36_MultyPoint(ST_Policy_Info *pPolicy,BYTE *m_ByteSendBuf,int buflen,int iPointNum);
	
	BYTE Sum(BYTE *buf, int len);
    CProtocol_IEC101();
//	virtual bool CheckCRC(CByteArray *m_RecvBufArray);
	virtual bool CheckCRC(BYTE *m_ByteRecvBuf);
	virtual void DealErrorRecvArray(ST_Policy_Info *pPolicy,CByteArray *m_RecvErrorArray);
	virtual void DealNormalRecvBuf(ST_Policy_Info *pPolicy,BYTE *m_ByteRecvBuf);
	virtual void DealNormalRecvBuf_feedback(ST_Policy_Info *pPolicy,BYTE *m_ByteRecvBuf,CExByteArray &ary);
protected:
private:
};


#endif