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

#ifndef WIN32
#include "portable.h"
#include <unistd.h>
#include <netinet/in.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <assert.h>
#include <errno.h>
#include "queue.h"
#include "fifoc.h"
#include "clear_crc_eight.h"
#include "iec101.h"
#include "iecserial.h"
#include "iec101_types.h"
#include "itrace.h"
#include <signal.h>
#include <time.h>
#include "link_unbalance_slave.h"
#include "serial.h"
#ifdef WIN32
#include <mmsystem.h>
#endif

static u_char OutputBuffertype;													
static int OutputBufferlength;												
static u_char OutputBuffer[IEC101_BUF_LEN];												
static int Trpflag;													
static int TrpMax;													
static int Trpcount;													
static u_short slave_addr;													
static u_char slave_addr_size;												
///////////////////////// 
static u_char rxDIR,txDIR;													
static u_char rxPRM,txPRM;													
static u_char FCB,ACD;
static u_char FCV,DFC;														
static u_char rxfunc,txfunc;												
static u_char frame_error;													
//////////////////////// 
static int link_state;															
static u_char rxbuffer[IEC101_BUF_LEN];										
static u_char txbuffer[IEC101_BUF_LEN];										
static int rxlength;														
static int txlength;														
static int state_proc_flag;
static struct iecserial *global_s;
static int polling_time;

static int RxOneFrame(u_char *rxbuf,int maxbuflen);
static void DecodeFrame(u_char *rxbuf,int rxlen);
static void process_incoming_data(u_char *rxbuf,int rxlen);
static int TxOneFrame(u_char *txbuf,int txlen);
static int EncodeFrameShort(u_char *txbuf);
static int EncodeFrameLong(u_char *destbuf,u_char *inputbuf,int inputlen);
static u_short GetlinkAddr(u_char *buf);
static int SetlinkAddr(u_char *buf,u_short addr);
static int GetSizeoflinkaddr(void);
static void DetectHead(u_char *Rxbuf,int* Rxlen);
static u_char calc_chs(u_char *buffer, int length);
static int make_shortframe(u_char *txbuf);
static int make_longframe(u_char *txbuf,BYTE *inputbuf,int inputlen);

static int SetlinkAddr(u_char* buf,u_short addr)   
{   
	if(addr == 0xFFFF)   
	{   
		if(slave_addr_size==1)   
			buf[0] = 0xFF;   
		if(slave_addr_size==2)   
			*((u_short*)buf) = 0xFFFF;
	}   
	else   
	{   
		if(slave_addr_size == 1)
			buf[0]=(unsigned char)addr;   
		if(slave_addr_size == 2)
			((u_short*)(buf))[0]=addr;   
	}
	
	return (slave_addr_size);   
}   

static u_short GetlinkAddr(u_char *buf)   
{   
	int sl_ad_size=slave_addr_size;   
	int ret=0;

	if (sl_ad_size==1)   
		ret=buf[0];
	else if(sl_ad_size==2)   
		ret = *((u_short*)buf);
	return ret;   
}   

static int GetSizeoflinkaddr(void)
{   
	return(slave_addr_size);   
}   

static int make_shortframe(u_char *txbuf)   
{
	int LA;   
	int slsize;

	u_char temp=0;   
	txbuf[0]=0x10;   
	//temp|=(txDIR<<7);   
	temp|=txPRM<<6;   
	temp|=ACD<<5;   
	temp|=DFC<<4;   
	temp|=txfunc;   
	txbuf[1]=temp;   
	LA=SetlinkAddr(&txbuf[2],slave_addr);   
	slsize=GetSizeoflinkaddr();   
	txbuf[2+LA]=calc_chs(&txbuf[1],slsize+1);   
	txbuf[3+LA]=0x16;   

	return(LA+4);   
}   

int CloseLink()   
{   
	state_proc_flag=0;
	return 1;   
}   

static void DecodeFrame(u_char *rxbuf,int rxlen)   
{   
	int tmp=0,tmp2=0,fl=0,tmp3=0,tmp4=0;
	BYTE cb=0;
	   
	int c =rxlen;   
	DetectHead(rxbuf, &c);   
    rxlen = c; 

	if(rxlen)   
	{   
		frame_error=0;

		if (rxbuf[0]==0xE5)   
		{   
			ACD=0;   
			DFC=0;   
			if (txfunc==0xA) rxfunc=9;   
			else if (txfunc==0xB) rxfunc=9;   
			else rxfunc=0;   
		}
		else if (rxbuf[0]==0x10)   
		{   
			tmp=GetSizeoflinkaddr();   
			if (rxbuf[tmp+2]==calc_chs(&rxbuf[1],tmp+1))   
			{   
				tmp2=GetlinkAddr(&rxbuf[2]);   
				if (tmp2==slave_addr) fl=1;   
				if (fl)   
				{   
					cb=rxbuf[1];   
					FCB=(cb>>5)&1;   
					FCV=(cb>>4)&1;   
					rxDIR=(cb>>7)&1;   
					rxPRM=(cb>>6)&1;   
					rxfunc=cb&0xF;   
				}   
			}   
			else frame_error=1;   
		}   
		else if ((rxbuf[0]==0x68)&&(rxbuf[rxbuf[1]+5]==0x16))   
		{   
			tmp3=GetSizeoflinkaddr();   
			if (rxbuf[rxbuf[1]+4]==calc_chs(&rxbuf[4],rxbuf[1]))   
			{   
				tmp4=GetlinkAddr(&rxbuf[5]);   
				if (tmp4==slave_addr) fl=1;   
				if (fl)   
				{   
					cb=rxbuf[4];   
					FCB=(cb>>5)&1;   
					FCV=(cb>>4)&1;   
					rxDIR=(cb>>7)&1;   
					rxPRM=(cb>>6)&1;   
					rxfunc=cb&0xF;   
				}   
			} else frame_error=1;   
		}
		else
		{
			frame_error = 1;   
		}
	}
	else
    {
        frame_error = 1;   
    }
}   

static unsigned char calc_chs(unsigned char* buffer, int length)   
{   
	int i=0;   
	unsigned char cs=0;   
	while(i<length)   
		cs+=buffer[i++];   
	return cs;   
}   

static int TxOneFrame(u_char *txbuf,int txlen)   
{   
	int i, n;

	if (txlen>0)   
	{   
		n = write_to_serial((HANDLE)(global_s->fd), txbuf, txlen);

		for(i = 0;i < n; i++)
		{
			unsigned char c = *((unsigned char*)txbuf + i);
			fprintf(stderr,"tx ---> 0x%02x\n", c);
			fflush(stderr);
			//IT_COMMENT1("tx ---> 0x%02x", c);
		}
	}
	
	return txlen;   
}   

static void DetectHead(u_char *Rxbuf,int *Rxlen)   
{   
    //For noisy communication channels
	int i,tmp;   
	int tmp2 = 0;   
    u_char parc_rx[IEC101_BUF_LEN];

    IT_IT("DetectHead");

	tmp = GetSizeoflinkaddr() + 3;
    
	for(i = 0; i < *Rxlen; i++)
	{   
		if((Rxbuf[i]==0x68)&&(Rxbuf[i+3]==0x68)&&(Rxbuf[i+1]==Rxbuf[i+2]))   
		{   
			memcpy(parc_rx, Rxbuf + i, *Rxlen-i);   
            memcpy(&Rxbuf[0], parc_rx, *Rxlen-i);   
			*Rxlen-=i;   
			tmp2=1;
			break;
		}   
		else if((Rxbuf[i]==0x10)&&(Rxbuf[i+tmp]==0x16))   
		{   
			memcpy(parc_rx, Rxbuf + i, *Rxlen-i);   
            memcpy(&Rxbuf[0], parc_rx, *Rxlen-i);   
			*Rxlen-=i;   
			tmp2=1;   
			break;
		}
		else if(Rxbuf[i]==0xE5)   
		{   
			memcpy(parc_rx, Rxbuf + i, *Rxlen-i);   
            memcpy(&Rxbuf[0], parc_rx, *Rxlen-i);   
			*Rxlen-=i;   
			tmp2=1;   
			//break; If 0xE5 it is noise in front of 0x10 or 0x68 packet,
			//in the next steps of this loop 0xE5 is removed,
			//so remove break;
		}
	}   

	if(!tmp2) *Rxlen=0;   

    IT_EXIT;
}

static int EncodeFrameLong(u_char *destbuf, u_char *inputbuf, int inputlen)   
{   
	int tmp = 0;
	if(inputlen > 0)
	{
		tmp = make_longframe(destbuf,inputbuf,inputlen);
	}

	inputlen = 0;  
	return tmp;   
}   


static void STAM_MASTERLINKAVAIL_moveto()   
{   
	struct iechdr *h;
	struct iec_buf *b;
	int data_len;

	if(!(TAILQ_EMPTY(&global_s->high_priority_q))) 
	{
		//First class data available
		if(ACD == 0)
		{
			 Trpcount=0;

			 txfunc=0x09; //RESPOND 	NACK : Requested data not available

			 ACD = 1; //Tell master we have first class user data
			 DFC=0;
			 txlength = EncodeFrameShort(txbuffer);   
			 link_state = STAM_EXREQRES;
			 
			 set_link_state(link_state);

			 TxOneFrame(txbuffer,txlength);   
		}
		else if(ACD == 1)
		{
			b = TAILQ_FIRST(&global_s->high_priority_q);
			TAILQ_REMOVE(&global_s->high_priority_q, b, head);

			h = &b->h;

			data_len = h->length01;

			memcpy(OutputBuffer, &(b->data), data_len);
			OutputBufferlength = data_len;   

			Trpcount=0;   
			txfunc=8; //RESPOND 	User data
					
			DFC=0;
			
			txlength = EncodeFrameLong(txbuffer,OutputBuffer,OutputBufferlength);   
			OutputBufferlength=0;   
			
			link_state = STAM_EXREQRES;
		
			set_link_state(link_state);

			TxOneFrame(txbuffer,txlength);   
		}
	}
	else if(!(TAILQ_EMPTY(&global_s->spontaneous_q))) 
	{
	    b = TAILQ_FIRST(&global_s->spontaneous_q);
	    TAILQ_REMOVE(&global_s->spontaneous_q, b, head);

		h = &b->h;

		data_len = h->length01;

		memcpy(OutputBuffer, &(b->data), data_len);
		OutputBufferlength = data_len;   

		Trpcount=0;   
		txfunc=8; //RESPOND 	User data
				
		ACD = 0; //Second class data
		DFC=0;
		
		txlength = EncodeFrameLong(txbuffer,OutputBuffer,OutputBufferlength);   
		OutputBufferlength=0;   
		
		link_state = STAM_EXREQRES;
	
		set_link_state(link_state);

		TxOneFrame(txbuffer,txlength);   
	}
	else   
	{   
		 //No user data available

		 Trpcount=0;

		 txfunc=0x09; //RESPOND 	NACK : Requested data not available

		 ACD = 0;
		 DFC=0;
		 txlength = EncodeFrameShort(txbuffer);   
		 link_state = STAM_EXREQRES;
		 
		 set_link_state(link_state);

		 TxOneFrame(txbuffer,txlength);   
	}
}


static void STAM_EXREQRES_moveto()   
{   
	rxlength = RxOneFrame(rxbuffer,IEC101_BUF_LEN);   
	Trpcount++;   

	//NOTE: Smaller is the pollling time of the master
	//and greater must be the MULTIPLIER, otherwise
	//the slave thinks the master is no more polling
	#define MULTIPLIER 10

	if((!rxlength)&&(Trpcount>TrpMax*MULTIPLIER))  
	{   
		 fprintf(stderr,"Master no response\n");
		 fflush(stderr);
		 link_state = STAM_WAITREQSTATUS;

		 set_link_state(link_state);
		 return;
	}   

	if(rxlength)
	{   
		DecodeFrame(rxbuffer,rxlength);

		 if((frame_error)&&(Trpcount>TrpMax))    
		 {   
			 fprintf(stderr,"Master no correct response\n");
			 fflush(stderr);
			 link_state = STAM_WAITREQSTATUS;

			 set_link_state(link_state);
		 }   
		 else   
		 {   
			 if(rxlength>0)   
			 {   
				 if(!frame_error)   
				 {   
					 if((rxfunc==0xA || rxfunc==0xB)&&(FCV))
					 {
						//The master is polling for data
						//Master Request/ RESPOND expected
						//0xA Request user data class 1
						//0xB Request user data class 2
						 DFC=0;
						 
						link_state = STAM_MASTERLINKAVAIL; set_link_state(link_state);
					 }

					 if(rxfunc==9&&(!FCV))
					 {
						 //Master Request/ RESPOND expected
						 //Receiving Request status of link
						 ////////////so send Status of link/////////
						 txPRM=0;
						 //FCV=0;   
						 txfunc = 0x0B;//RESPOND 	Status of link or access demand   
						 ACD = 0;
						 DFC=0;
						 txlength = EncodeFrameShort(txbuffer);   
						 TxOneFrame(txbuffer,txlength);   
						 ////////////////////////////////////////

						 link_state = STAM_STATUSOFLINK;
						 set_link_state(link_state);
					 }

					 if(rxfunc==0x03 &&(FCV))
					 {
						 //Master Send User data/ CONFIRM expected
						 //so send Positive ACK
						 txPRM=0;
						 txfunc=0x00; //CONFIRM 	ACK : Positive acknowledgement  
						 //ACD = 1;
						 txlength = EncodeFrameShort(txbuffer);   
						 TxOneFrame(txbuffer,txlength);   

						 link_state = STAM_PROCESS_INCOMING_DATA; set_link_state(link_state);
					 }
				 }   
			 }   
		 } 
	}   
	else
	{
		link_state = STAM_EXREQRES;
		 
		set_link_state(link_state);
	}
}   


static void STAM_WAITREQSTATUS_moveto()
{   
    IT_IT("STAM_WAITREQSTATUS_moveto");

	rxlength = RxOneFrame(rxbuffer,IEC101_BUF_LEN);   
	
	if(rxlength)   
	{   
		 DecodeFrame(rxbuffer,rxlength);

		 if(!frame_error)   
		 {   
			 if((rxfunc==9)&&(rxPRM)&&(!FCV))   
			 {   
				 //Master Request/ RESPOND expected
				 //Receiving Request Staus
				 ////////////so send Status of link/////////
				 txPRM=0;
				 //FCV=0;   
				 txfunc=0x0B;  //RESPOND 	Status of link or access demand 
				 ACD =0;
				 txlength = EncodeFrameShort(txbuffer);   
				 TxOneFrame(txbuffer,txlength);   
				 ////////////////////////////////////////

				 link_state = STAM_STATUSOFLINK;

				 set_link_state(link_state);
			 }   
		 }   
	}   

    IT_EXIT;
}   


static void STAM_STATUSOFLINK_moveto()
{
	IT_IT("STAM_STATUSOFLINK_moveto");
    
	rxlength = RxOneFrame(rxbuffer,IEC101_BUF_LEN);   
		
	if(rxlength)   
	{   
		 DecodeFrame(rxbuffer,rxlength);

		 if(!frame_error)   
		 {   
			 if((rxfunc==0)&&(rxPRM)&&(!FCV))   
			 {   
				 //Master Send/ CONFIRM expected
				 //Receiving Reset remote Link
				 //so send Positive ACK
				 txPRM=0;
				 txfunc=0x00; //CONFIRM 	ACK : Positive acknowledgement  
				 ACD = 1;
				 txlength = EncodeFrameShort(txbuffer);   
				 TxOneFrame(txbuffer,txlength);   

				 link_state = STAM_POSITIVEACK;

				 set_link_state(link_state);
			 }
			 
			 if((rxfunc==9)&&(rxPRM)&&(!FCV))   
			 {   
				 //Master Request/ RESPOND expected
				 //Receiving Request Staus
				 ////////////so send Status of link/////////
				 txPRM=0;
				 //FCV=0;   
				 txfunc=0x0B; //RESPOND 	Status of link or access demand  
				 ACD =0;
				 txlength = EncodeFrameShort(txbuffer);   
				 TxOneFrame(txbuffer,txlength);   
				 ////////////////////////////////////////

				 link_state = STAM_STATUSOFLINK;

				 set_link_state(link_state);
			 }
		 }   
	}
	
	IT_EXIT;
}

static STAM_PROCESS_INCOMING_DATA_moveto()
{
	DFC=0;  //more messages are accepted
	process_incoming_data(rxbuffer,rxlength);

	link_state = STAM_EXREQRES; set_link_state(link_state);
}

static void STAM_POSITIVEACK_moveto()
{
	IT_IT("STAM_POSITIVEACK_moveto");

	rxlength = RxOneFrame(rxbuffer,IEC101_BUF_LEN);   
		
	if(rxlength)   
	{   
		 DecodeFrame(rxbuffer,rxlength);

		 if(!frame_error)   
		 {   
			 if((rxfunc == 0xA || rxfunc == 0xB)&&(rxPRM))   
			 {   
				 fprintf(stderr,"Master!\n");
				 fflush(stderr);
				 link_state = STAM_MASTERLINKAVAIL;

				 set_link_state(link_state);
			 }
			 
			 if((rxfunc==9)&&(rxPRM)&&(!FCV))   
			 {   
				 //Master Request/ RESPOND expected
				 //Receiving Request Staus
				 ////////////so send Status of link/////////
				 txPRM=0;
				 //FCV=0;   
				 txfunc=0x0B; //RESPOND 	Status of link or access demand  
				 ACD =0;
				 txlength = EncodeFrameShort(txbuffer);   
				 TxOneFrame(txbuffer,txlength);   
				 ////////////////////////////////////////

				 link_state = STAM_STATUSOFLINK;

				 set_link_state(link_state);
			 }
		 }   
	}   

	IT_EXIT;
}

static int RxOneFrame(u_char *rxbuf, int maxbuflen)   
{   
	int n, n_tot, length, off, i;
	unsigned char parc_buf[IEC101_BUF_LEN];
	int t1, t2;

	length = 0;
	n_tot = 0;
	n = 0;
	off = 0;

	memset(rxbuf, 0x00, maxbuflen);

	t1 = timeGetTime();

	do
	{
		off = off + n;

		read_from_serial((HANDLE)(global_s->fd), parc_buf, (unsigned long*) &n, IEC101_BUF_LEN, gl_read_timeout_ms);

		for(i = 0;i < n; i++)
		{
			unsigned char c = *((unsigned char*)parc_buf + i);
			fprintf(stderr,"rx <--- 0x%02x\n", c);
			fflush(stderr);
			//IT_COMMENT1("rx <--- 0x%02x", c);
		}

		memcpy(rxbuf + off, parc_buf, n);

		for(i = 0; i < n; i++)
		{
			if(rxbuf[i] == 0x68)
			{
				if(i+1 < n)
				{
					//Second octect + 4 octetcs for header(0x68 + length+ length + 0x68) + 1 octect for checksum + 1 octect for end character (0x16)
					length = rxbuf[i+1] + 4 + 2;
				}
				else
				{
					length = 4+2;
				}
				break;
			}
			else if(rxbuf[i] == 0x10)
			{
				length = 5;
				break;
			}
			else if(rxbuf[i] == 0xE5)
			{
				length = 1;
				break;
			}
		}

		n_tot = n_tot + n;

		t2 = timeGetTime();
	}
	while((n_tot < length) && (t2 - t1 < gl_read_timeout_ms));

	return n_tot;
}

void State_process()   
{   
	while(state_proc_flag)
	{   
		switch(link_state)
		{   
			case STAM_MASTERLINKAVAIL: STAM_MASTERLINKAVAIL_moveto(); break;   
			case STAM_EXREQRES: STAM_EXREQRES_moveto(); break;   
			case STAM_WAITREQSTATUS: STAM_WAITREQSTATUS_moveto(); break;
            case STAM_STATUSOFLINK: STAM_STATUSOFLINK_moveto(); break;
            case STAM_POSITIVEACK: STAM_POSITIVEACK_moveto(); break;
			case STAM_PROCESS_INCOMING_DATA: STAM_PROCESS_INCOMING_DATA_moveto(); break;
			default:;
		}

//#ifdef CHECK_TIMEOUT_WITH_PARENT        
//		if(check_timeout_with_parent(polling_time))
//		{
//			state_proc_flag = 0;
//		}
//#endif

		Sleep(polling_time);

		process_timer_send_frame(global_s, NULL);
		iec101_run_send_queues(global_s);
	}
}   

static int make_longframe(u_char *txbuf, BYTE *inputbuf, int inputlen)   
{   
	u_char *tmptxbuf=txbuf;   
	int tmplen=0,len=0;   
	u_char cb=0,CS=0;   
	int sl_ad_size=0;   
	(*tmptxbuf++)=0x68;   
	tmplen=inputlen+GetSizeoflinkaddr()+1;   
	(*tmptxbuf++)=tmplen;   
	(*tmptxbuf++)=tmplen;   
	(*tmptxbuf++)=0x68;   
	//cb|=(txDIR<<7);   
	cb|=txPRM<<6;   
	cb|=ACD<<5;   
	cb|=DFC<<4;   
	cb|=txfunc;   
	(*tmptxbuf++)=cb;
	
	sl_ad_size=SetlinkAddr(tmptxbuf,slave_addr);

	tmptxbuf+=sl_ad_size;   
	memcpy(tmptxbuf,inputbuf,inputlen);   
	tmptxbuf+=inputlen;   
	CS=calc_chs(&txbuf[4],tmplen);   
	(*tmptxbuf++)=CS;   
	(*tmptxbuf++)=0x16;   

	len=tmptxbuf-txbuf;   

	return len;   
}   

static void process_incoming_data(u_char *rxbuf, int rxlen)   
{   
	//int i;
	struct iec_buf *b;
	
	if(rxlen > 0)    
	{   
		get_iec_buf(&b);
		
		//for(i = 0; i < rxlen; i++)
		//{
		//	unsigned char c = *((unsigned char*)rxbuf + i);
		//	fprintf(stderr,"%02x", c);
		//	fflush(stderr);
		//}

		fprintf(stderr,"\n");
		fflush(stderr);

		b->data_len = rxbuf[1] - 2; //minus control field and link address
		memcpy(&(b->h), rxbuf, rxlen);

		process_data_received_hook(global_s, b);

		free_iec_buf(b);
	}
}   

static int EncodeFrameShort(u_char *txbuf)   
{   
	return(make_shortframe(txbuf));   
}   

int InitLink(struct iecserial *s)
{   
	OutputBufferlength = 0;   
	slave_addr_size = LINK_ADDRLEN;   
	slave_addr = gl_link_address;
	state_proc_flag = 1;   
	link_state = STAM_WAITREQSTATUS;
	rxDIR = 0;   
	txDIR = 0;   
	rxPRM = 0;   
	txPRM = 0;   
	FCB=0;   
	ACD=0;   
	FCV=0;   
	DFC=0;   
	txfunc = 0;
	rxfunc = 0;
	frame_error = 0;
	global_s = s;
	TrpMax = 3;   
	polling_time = gl_slave_polling_time_in_milliseconds;
	reset_state_machines();

	return 1;   
}   

void set_link_state(int state)
{
	fprintf(stderr, "state = %d\n", state);
	fflush(stderr);

	switch(state)
	{
		case STAM_MASTERLINKAVAIL: state_iec_101_link = LNK_CONNECTED; break;
		case STAM_WAITREQSTATUS: state_iec_101_link = LNK_IDLE; reset_state_machines(); break;
		default: break;
	}
}

/*
rx <--- 0x68- START FRAME  HEADER
rx <--- 0x12- LENGTH  HEADER
rx <--- 0x12- LENGTH  HEADER
rx <--- 0x68- START FRAME  HEADER
rx <--- 0x08- Control field <-------- start ASDU lenght
rx <--- 0x01- Link Address
rx <--- 0x1e-
rx <--- 0x01-
rx <--- 0x03-
rx <--- 0x01-
rx <--- 0x00-
rx <--- 0x64-
rx <--- 0x00-
rx <--- 0x00-
rx <--- 0x01-
rx <--- 0xd3-
rx <--- 0x51-
rx <--- 0x14-
rx <--- 0x14-
rx <--- 0x0c-
rx <--- 0x0c-
rx <--- 0x0b- <--------- end ASDU length
rx <--- 0x00- CHS
rx <--- 0x16- END FRAME
*/

void iecserial_set_defaults(struct iecserial *s)
{
	IT_IT("iecserial_set_defaults");

	TAILQ_INIT(&s->write_q);
	TAILQ_INIT(&s->high_priority_q);
	TAILQ_INIT(&s->spontaneous_q);
	
	IT_EXIT;
}

int iecserial_connect(char* port_name, int baud_rate)
{
	struct iecserial *s;

	IT_IT("iecserial_connect");

	if(alloc_queues())
	{
		IT_EXIT;
		return(-1);
	}
	
	s = (struct iecserial *)calloc(1, sizeof(struct iecserial));
	if (!s)
	{
		IT_EXIT;
		return(-1);
	}

	InitLink(s);
	
	if ((s->fd = open_port(port_name, baud_rate)) == -1) {
		free(s);
		IT_EXIT;
		return(-1);
	}
			
	iecserial_set_defaults(s);
	s->type = IEC_SLAVE;
	
	IT_EXIT;
	return(0);
}

void iecserial_prepare_iframe(struct iec_buf *buf)
{
	struct iechdr *h;

	IT_IT("iecserial_prepare_iframe");
	
	h = &buf->h;
	h->start01 = 0x68;
	h->start02 = 0x68;
	h->length01 = buf->data_len;
	h->length02 = buf->data_len;

	IT_EXIT;
}

void flush_queue(struct iec_buf_queue *q)
{
	struct iec_buf *n1;
	int i;

	IT_IT("flush_queue");

	// TailQ Deletion.
    while(!TAILQ_EMPTY(q)) 
	{
	    n1 = TAILQ_FIRST(q);
	    TAILQ_REMOVE(q, n1, head);

	    IT_COMMENT1("flush free element= %x", n1);

		for(i = 0; i < IEC_101_MAX_EVENTS; i++)
		{
			if(n1 == v_iec_buf[i].c)
			{
				v_iec_buf[i].used = 0;
				break;
			}
		}
     }

	IT_EXIT;
}

void free_iec_buf(struct iec_buf *b)
{
	int i;

	IT_IT("free_iec_buf");

	for(i = 0; i < IEC_101_MAX_EVENTS; i++)
	{
		if(b == v_iec_buf[i].c)
		{
			v_iec_buf[i].used = 0;
			break;
		}
	}

	IT_EXIT;
}

void iecserial_flush_queues(struct iecserial *s)
{
	IT_IT("iecserial_flush_queues");

	flush_queue(&s->write_q);
	flush_queue(&s->high_priority_q);
	flush_queue(&s->spontaneous_q); 

	IT_EXIT;
}
