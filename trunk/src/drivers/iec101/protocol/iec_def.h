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

#ifndef IEC_DEF_H
#define IEC_DEF_H

#include <string>
using namespace std;

#define GENG_mac_APP_NAME_LEN 20
#define GENG_mac_IPaddr_LEN 16

#define CHANNEL_SerialPort 0
#define CHANNEL_TcpServer 1
#define CHANNEL_TcpClient 2

#define PROTOCOL_IEC101 101
#define PROTOCOL_IEC104 104
#define PROTOCOL_IECTASE2 502

#define IEC101_MASTER 0
#define IEC101_SLAVE 1

/*==== IEC870-5-101 Link Protocol Control Information =====*/

#define LPCI_SYN            0x10
#define LPCI_STX            0x68
#define LPCI_ETX            0x16
#define LPCI_FCB            0x20
#define LPCI_FCV            0x10
#define LPCI_ACD            0x20
#define LPCI_SEQ            0x80
#define LPCI_NUL            0xe5

#define DIR_MASTER_TO_SLAVE 0x00
#define DIR_SLAVE_TO_MASTER 0x80

#define PRM_MASTER 0x40
#define PRM_SLAVE 0x00

#define FCB_MASTER_SAMESEND 0x00

#define FCV_VALID 0x10
#define FCV_INVALID 0x00
#define FUNC_RESET_LINK 0x00
#define FUNC_CALL_LINK_STAT 0x09
#define FUNC_ACK_LINK_STAT 0x0B
#define FUNC_SEND_NO_RESP 0x04
#define FUNC_SEND_NEED_RESP 0x03

#define TYPE_EXTEND_REAL_WATERLINE 52
#define TYPE_ALL_BECKON 100

#define M_SP_NA                1  
#define M_SP_TA                2  
#define M_ME_NC				   13   

#define C_IC_NA              100
#define C_CI_NA              101
#define C_RD_NA              102
#define C_CS_NA              103

#define SQ_HAVE_INFOOBJECT 1
#define SQ_NO_INFOOBJECT 0

#define VSQ_OEN_POINT 0X01
#define VSQ_SEND_SINGLE_POINT 0X81
#define VSQ_SEND_MULTY_POINT 0X8N

#define COT_MASTER_ACTIVE 3
#define COT_BECKON_ACTIVE 6

typedef struct  {
    int port;
	int baud;
	int	StartNum;
	int DataNum;
	int CheakType;
	int EndNum;
}Channel_serialport_info;

typedef struct  {
    int ilocal_port;
   char szlocal_ip[GENG_mac_IPaddr_LEN];
}Channel_tcpserver_info;

typedef struct  {
    int ilocal_port;
	char szlocal_ip[GENG_mac_IPaddr_LEN];
	int iRemote_port;
	char szRemote_ip[GENG_mac_IPaddr_LEN];

}Channel_tcpclient_info;

typedef struct  {
	int type;     
	union{
		Channel_serialport_info    con_serialport;
		Channel_tcpserver_info     con_tcpserver;
		Channel_tcpclient_info     con_tcpclient;
	}con;
}Channel_Info;


typedef struct  {
	int stationtype;
	int local_stationid;
	int Remote_stationid;
	int nCOAAddrNum;
	int nCOTAddrNum;
	int nINFOAddrNum;
	int iInfoAddrNum_Beckon;
}Protocol_iec101_info;

typedef struct  {
	int stationtype;
	int nCOAAddrNum;
	int nCOTAddrNum;
	int nINFOAddrNum;
}Protocol_iec104_info;

typedef struct  {
	int type;
	union{
		Protocol_iec101_info pro_iec101;
		Protocol_iec104_info pro_iec104;
	};
}Protocol_Info;

typedef struct              
{
	int ChannelSetID;		
	int iLink_Stat;     //---connect success = 1
	int iLink_Alive;    
	bool bLink_ResetSuccess;
	int Local_sid;
	int	SID;		
	int ChannelType;		//1:UDP 2:ClientSocket 3:ServerSocket
	int ChannelPortRecv;		
	int ChannelPortSend;		
	char DestIP[30];				
	char LocalIP[30];				
   
	int  Policyid;
	char    szLocalAppName[GENG_mac_APP_NAME_LEN];
	char    szRemoteAppName[GENG_mac_APP_NAME_LEN];
	Channel_Info channel_info;
      Protocol_Info protocol_info;

	int TimeW;				
	int SystemType;			//0 windows 1 unix
      int LocalSystemType;			//0 windows 1 unix
	DATE OldTime;

	int nSetTime;
	char CharLog[1024];
}ST_Policy_Info;

#endif