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

#ifndef GLOBAL_H
#define GLOBAL_H
#include "SerialPort.h"

#define RCVBUFSIZE          1024
#define MSGBUFSIZE          1024

extern int GenG_var_Policy_Num;

void MessageBinDebug(char *buf,int len);

extern CMutex mutex_Com_recv_array;
extern CByteArray Com_recv_array;
extern CPtrArray GenG_var_policy_array;
extern CPtrArray GenG_var_RecvPoint_array;


typedef struct 
{
	int iSID	;
	int iaddress;
	int iSenid;
	char SenName[60];
	int iTimeW;
	int	iDataType;
	int	iDateTypeForm;
	float fValue;
	double DataTime;
	double Time;
	double OldTime;
	float fMaxProValue;
	float fMinProValue;
}RecvPoint;

class CExByteArray
{
public:
	CExByteArray();
	~CExByteArray();
public:
	BOOL  Add(BYTE* pBuf, int count);
	BOOL  InsertAt(int pos, BYTE* pBuf, int count);
	int   GetSize(){ return m_Size; }
	BOOL  GetAt(BYTE&b, int pos);
	int   GetBuf(int pos, BYTE* pBuf, int count);
	BYTE* GetData(){ return m_pData; }
	BOOL  RemoveAt(int pos);
	int   Remove(int pos, int count);
	void  RemoveAll();
	
private:
	BYTE* m_pData;
	int   m_Size;
};

#endif