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
#include "Global.h"
#include "SerialPort.h"


ST_COM_INFO GenG_var_com_info;

CPtrArray GenG_var_policy_array;
CPtrArray GenG_var_RecvPoint_array;

int GenG_var_Policy_Num;

void MessageBinDebug(char *buf,int len)
{
	CString str_temp = "";
  CString str_show ="";
  
  for(int i=0;i<len;i++)
  {
	  str_temp.Format("%02X ",(BYTE)buf[i]);
	  str_show+=str_temp;
  }
  AfxMessageBox(str_show);
  
}


CMutex mutex_Com_recv_array;
CByteArray Com_recv_array;

/////////////////////////////////////////////////////////////////////////////
// CExByteArray

CExByteArray::CExByteArray()
{
	m_pData = NULL;
	m_Size = 0;
}

CExByteArray::~CExByteArray()
{
	if(m_pData)	
	{
		delete m_pData;
		m_pData = NULL;
	}
	m_Size = 0;
}

BOOL CExByteArray::Add(BYTE* pBuf, int count)
{
	if(count <= 0) return FALSE;

	int m_NewSize = m_Size + count;
	BYTE* pNew = new BYTE[m_NewSize];
	
	if(m_pData)
	{		
		memcpy(pNew, m_pData, m_Size);
		delete m_pData;
	}
	memcpy(pNew+m_Size, pBuf, count);
	m_pData = pNew;
	m_Size = m_NewSize;

	return TRUE;
}

BOOL CExByteArray::InsertAt(int pos, BYTE* pBuf, int count)
{
	if(count <= 0 || pos > m_Size) return FALSE;

	int m_NewSize = m_Size + count;
	BYTE* pNew = new BYTE[m_NewSize];

	if(m_pData)
	{
		if(pos)
			memcpy(pNew, m_pData, pos);
		memcpy(pNew+pos, pBuf, count);
		if(pos < m_Size)
		{
			memcpy(pNew+pos+count, m_pData+pos, m_Size-pos);
		}
		delete m_pData;
	}
	else
	{
		memcpy(pNew, pBuf, count);
	}
	m_pData = pNew;
	m_Size = m_NewSize;

	return TRUE;
}

BOOL CExByteArray::GetAt(BYTE&b, int pos)
{
	if(pos < 0 || pos >= m_Size) return FALSE;

	b = *(m_pData+pos);
	return TRUE;
}

int CExByteArray::GetBuf(int pos, BYTE* pBuf, int count)
{
	if(count <= 0 || pos < 0 || pos >= m_Size) return 0;

	int nLeft = m_Size - pos;
	int nReturn = nLeft > count ? count : nLeft;

	memcpy(pBuf, m_pData+pos, nReturn);
	return nReturn;
}

BOOL CExByteArray::RemoveAt(int pos)
{
	if(pos < 0 || pos >= m_Size) return FALSE;
	if(m_Size == 1)
	{
		delete m_pData;
		m_pData = NULL;
		m_Size = 0;
		return TRUE;
	}

	m_Size--;
	BYTE* pNew = new BYTE[m_Size];
	memcpy(pNew, m_pData, pos);
	memcpy(pNew+pos, m_pData+pos+1,m_Size-pos);
	delete m_pData;
	m_pData = pNew;
	
	return TRUE;
}

int CExByteArray::Remove(int pos, int count)
{
	if(pos < 0 || count <= 0 || !m_Size) return 0;

	int nLeft = m_Size - pos;
	int nReturn = nLeft > count ? count : nLeft;

	int nNewSize = m_Size - nReturn;
	if(nNewSize)
	{
		BYTE* pNew = new BYTE[nNewSize];
		if(pos) 
			memcpy(pNew, m_pData, pos);
		if(count < nLeft)
			memcpy(pNew+pos, m_pData+pos+count, nLeft-count);
		delete m_pData;
		m_pData = pNew;
		m_Size = nNewSize;
	}
	else
	{
		delete m_pData;
		m_pData = NULL;
		m_Size = 0;
	}	

	return nReturn;
}

void CExByteArray::RemoveAll()
{
	if(m_pData)
	{
		delete m_pData;
		m_pData = NULL;
	}
	m_Size = 0;
}
