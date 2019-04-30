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
#include <errno.h>
#include "queue.h"
#include "clear_crc_eight.h"
#include "iec103.h"
#include "iecserial.h"
#include "iec103_types.h"
#include "itrace.h"
#include <signal.h>
#include <time.h>
#include "link_unbalance_master.h"
#include "serial.h"
#include "itrace.h"
#ifdef WIN32
#include <mmsystem.h>
#endif

static u_char OutputBuffertype;													
static int OutputBufferlength;												
static u_char OutputBuffer[IEC103_BUF_LEN];												
static int Trpflag;													
static int TrpMax;													
static int Trpcount;													
static u_short slave_addr;													
static u_char slave_addr_size;												
///////////////////////// 
static u_char rxDIR,txDIR;													
static u_char rxPRM,txPRM;													
static u_char FCB,ACD,FCBtoggle;
static u_char FCV,DFC;														
static u_char rxfunc,txfunc;												
static u_char frame_error;													
//////////////////////// 
static int link_state;															
static u_char rxbuffer[IEC103_BUF_LEN];										
static u_char txbuffer[IEC103_BUF_LEN];										
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
    IT_IT("SetlinkAddr");

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

    IT_EXIT;
	
	return (slave_addr_size);   
}   

static u_short GetlinkAddr(u_char *buf)   
{   
    int sl_ad_size=slave_addr_size;   
	int ret=0;
    IT_IT("GetlinkAddr");

	if (sl_ad_size==1)   
		ret=buf[0];
	else if(sl_ad_size==2)   
		ret = *((u_short*)buf);

    IT_EXIT;

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

    IT_IT("make_shortframe");

	txbuf[0] = 0x10;   
	//temp|=(txDIR<<7);   
	temp |= txPRM<<6;   
	temp |= FCB<<5;   
	temp |= FCV<<4;   
	temp |= txfunc;   
	txbuf[1] = temp;   
	LA = SetlinkAddr(&txbuf[2],slave_addr);   
	slsize = GetSizeoflinkaddr();   
	txbuf[2+LA] = calc_chs(&txbuf[1],slsize+1);   
	txbuf[3+LA] = 0x16;

    IT_EXIT;

	return(LA+4);   
}   

int CloseLink()   
{   
    IT_IT("CloseLink");

	state_proc_flag=0;

    IT_EXIT;

	return 1;   
}   

static void DecodeFrame(u_char *rxbuf,int rxlen)   
{   
    int tmp=0,tmp2=0,fl=0,tmp3=0,tmp4=0;
	BYTE cb=0;
	int c =rxlen;

    IT_IT("DecodeFrame");
   	   
	DetectHead(rxbuf, &c);   
    rxlen = c;

	if(rxlen)   
	{   
		frame_error=0;   

		if(rxbuf[0]==0xE5)   
		{   
			//fprintf(stderr, "rxbuf[0]==0xE5");
			//fflush(stderr);

			ACD=0;   
			DFC=0;   
			if (txfunc==0xA) rxfunc=9;   
			else if (txfunc==0xB) rxfunc=9;   
			else rxfunc=0;   
		}
		else if (rxbuf[0]==0x10)   
		{   
			//fprintf(stderr, "rxbuf[0]==0x10");
			//fflush(stderr);

			tmp=GetSizeoflinkaddr();   

			if (rxbuf[tmp+2] == calc_chs(&rxbuf[1],tmp+1))   
			{   
				tmp2=GetlinkAddr(&rxbuf[2]);   

				if (tmp2==slave_addr) fl=1;   

				if (fl)   
				{   
					cb=rxbuf[1];   
					ACD=(cb>>5)&1;   
					DFC=(cb>>4)&1;   
					rxDIR=(cb>>7)&1;   
					rxPRM=(cb>>6)&1;   
					rxfunc=cb&0xF;   
				}   
			}   
			else
            {
                frame_error = 1;   
				fprintf(stderr, "Frame error\n");
				fflush(stderr);

                IT_COMMENT("Frame error 1");
            }
		}   
		else if ((rxbuf[0]==0x68)&&(rxbuf[rxbuf[1]+5]==0x16))   
		{   
			//fprintf(stderr, "rxbuf[0]==0x68");
			//fflush(stderr);

			tmp3=GetSizeoflinkaddr();
            
            IT_COMMENT1("rxbuf[rxbuf[1]+4]= 0x%x", rxbuf[rxbuf[1]+4]);
            IT_COMMENT1("chs = 0x%x ", calc_chs(&rxbuf[4],rxbuf[1]));

			if (rxbuf[rxbuf[1]+4] == calc_chs(&rxbuf[4],rxbuf[1]))   
			{   
				tmp4=GetlinkAddr(&rxbuf[5]);   

				if (tmp4==slave_addr) fl=1;   

				if (fl)   
				{   
					cb=rxbuf[4];   
					ACD=(cb>>5)&1;   
					DFC=(cb>>4)&1;   
					rxDIR=(cb>>7)&1;   
					rxPRM=(cb>>6)&1;   
					rxfunc=cb&0xF;   

					//fprintf(stderr, "rxfunc = %d", rxfunc);
					//fflush(stderr);
				}
			} 
            else
            {
                frame_error = 1;
				fprintf(stderr, "Frame error\n");
				fflush(stderr);
                IT_COMMENT("Frame error 2");
            }
		}
        else
        {
            frame_error = 1;   
			fprintf(stderr, "Frame error\n");
			fflush(stderr);
            IT_COMMENT("Frame error 3");
        }
	}
	else
    {
        frame_error = 1;
		fprintf(stderr, "Frame error\n");
		fflush(stderr);
        IT_COMMENT("Frame error 4");
    }
    
    IT_EXIT;
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

    IT_IT("TxOneFrame");

	if (txlen>0)   
	{   
		n = write_to_serial((HANDLE)(global_s->fd), txbuf, txlen, gl_rtsOnTime, gl_rtsOffTime);

		for(i = 0;i < n; i++)
		{
			unsigned char c = *((unsigned char*)txbuf + i);
			fprintf(stderr,"tx ---> 0x%02x\n", c);
			fflush(stderr);
			IT_COMMENT1("tx ---> 0x%02x", c);
		}
	}
	
    IT_EXIT;

	return txlen;   
}   

static void DetectHead(u_char *Rxbuf,int *Rxlen)   
{   
    //For noisy communication channels
	int i,tmp;   
	int tmp2 = 0;   
    u_char parc_rx[IEC103_BUF_LEN];

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

    IT_IT("EncodeFrameLong");

	if(inputlen > 0)
	{
		tmp = make_longframe(destbuf,inputbuf,inputlen);
	}

	inputlen = 0;  

    IT_EXIT;

	return tmp;   
}   

static void STAM_LINKRESET_moveto()   
{   
    IT_IT("STAM_LINKRESET_moveto");

	link_state = STAM_WAITFORREQ;

	set_link_state(link_state);

    IT_EXIT;
}   

static void STAM_SLAVELINKAVAIL_moveto()   
{   
	struct iechdr *h;
	struct iec_buf *b;
	int data_len;

    IT_IT("STAM_SLAVELINKAVAIL_moveto");

	if(!(TAILQ_EMPTY(&global_s->write_q))) 
	{
	    b = TAILQ_FIRST(&global_s->write_q);
	    TAILQ_REMOVE(&global_s->write_q, b, head);

		h = &b->h;

		data_len = h->length01;

		memcpy(OutputBuffer, &(b->data), data_len);
		OutputBufferlength = data_len;   
		OutputBuffertype = 2;    //SENDCONFIRM
		//OutputBuffertype = 1;  //SENDNOREPLY

		if(OutputBuffertype == 1)   //SENDNOREPLY
		 {   
			 txfunc=4;   
			 FCV=0;   
			 txlength = EncodeFrameLong(txbuffer,OutputBuffer,OutputBufferlength);   
			 FCV=1;   
			 OutputBufferlength=0;   
			 TxOneFrame(txbuffer,txlength);   
		 }   
		 else if(OutputBuffertype == 2)  //SENDCONFIRM
		 {   
			 Trpcount=0;   
			 txfunc=3;   
			 FCB^=1;   
			 txlength = EncodeFrameLong(txbuffer,OutputBuffer,OutputBufferlength);   
			 OutputBufferlength=0;   
			 link_state = STAM_SENDCON;

			 set_link_state(link_state);
		 }
	}
	else   
	{   
		 Trpcount=0;

		 if(ACD == 1) txfunc=0xA; else txfunc = 0xB;

		 FCB^=1;   
		 txlength = EncodeFrameShort(txbuffer);   
		 link_state = STAM_EXREQRES;

		 set_link_state(link_state);
	}

    IT_EXIT;
}
   
static void STAM_EXREQRES_moveto()   
{   
    unsigned int tmp;   

    IT_IT("STAM_EXREQRES_moveto");

	TxOneFrame(txbuffer,txlength);   
	rxlength = RxOneFrame(rxbuffer,IEC103_BUF_LEN);   
	Trpcount++;   

	if((!rxlength)&&(Trpcount>TrpMax))  
	{   
		 fprintf(stderr,"Slave no response\n");
		 fflush(stderr);
         IT_COMMENT("Slave no response");

		 link_state = STAM_WAITFORREQ;

		 set_link_state(link_state);
	}   
	else   
	{   
		 DecodeFrame(rxbuffer,rxlength);

		 if((frame_error)&&(Trpcount>TrpMax))    
		 {   
			 fprintf(stderr,"Slave no correct response\n");
			 fflush(stderr);
             IT_COMMENT("Slave no correct response");

			 link_state = STAM_WAITFORREQ;

			 set_link_state(link_state);
		 }   
		 else   
		 {   
			 if(rxlength>0)   
			 {   
				 if(!frame_error)   
				 {   
					tmp = rxfunc - 8;

					if(tmp <= 7)
					{
						switch(tmp)   
						{   
							case 0: process_incoming_data(rxbuffer,rxlength); link_state = STAM_SLAVELINKAVAIL; set_link_state(link_state); break;
							case 1: fprintf(stderr,"No data\n"); fflush(stderr); link_state = STAM_SLAVELINKAVAIL; set_link_state(link_state); break;
							case 2:   
							case 3:   
							case 4:    
							case 5:  break;   
							case 6:    
							case 7: fprintf(stderr,"Err status\n"); fflush(stderr); link_state = STAM_SLAVELINKAVAIL; set_link_state(link_state); break;
						}
					}
				 }   
                 else
                 {
                    IT_COMMENT("Frame error");
                 }
			 }   
		 }   
	}   

    IT_EXIT;
}   

static void STAM_EXREQLINK_moveto()   
{   
    IT_IT("STAM_EXREQLINK_moveto");
    //Request status of link

	FCV=0;   
	txfunc=9;   
	FCB=0;   
	txlength = EncodeFrameShort(txbuffer);   
	FCV=1;   
	TxOneFrame(txbuffer,txlength);   
	rxlength = RxOneFrame(rxbuffer,IEC103_BUF_LEN);   
	Trpcount++;
	
	if((rxlength)&&(Trpcount<=TrpMax))   
	{   
		 DecodeFrame(rxbuffer,rxlength);

		 if(!frame_error)   
		 {   
			 if((rxfunc!=14)&&(rxfunc!=15))
			 {
				 if((rxfunc==11)&&(!DFC))   
				 {   
					 Trpcount = 0;   
					 link_state = STAM_EXRESETLINK;

					 set_link_state(link_state);
				 }   
			 }
		 }   
	}   
	else   
	{   
		 fprintf(stderr,"Error status\n");
         fflush(stderr);
         IT_COMMENT("Error status 1");

		 link_state = STAM_WAITFORREQ;

		 set_link_state(link_state);
	}   

    IT_EXIT;
}   

static void STAM_EXRESETLINK_moveto()   
{   
    IT_IT("STAM_EXRESETLINK_moveto");

	FCB=0;
	txfunc=0;
	FCV=0;
	txlength = EncodeFrameShort(txbuffer);
	FCV=1;
	TxOneFrame(txbuffer,txlength);
	rxlength = RxOneFrame(rxbuffer,IEC103_BUF_LEN);

	Trpcount++;

	if((!rxlength)||(Trpcount>TrpMax))   
	{   
		fprintf(stderr,"error status\n");
		fflush(stderr);

        IT_COMMENT("Error status 2");

		link_state = STAM_WAITFORREQ;

		set_link_state(link_state);
	}   
	else   
	{   
		DecodeFrame(rxbuffer,rxlength);

		if(!frame_error)
		{
			if ((rxfunc!=14)&&(rxfunc!=15)&&(rxfunc!=1)&&(!rxfunc))   
			{   
				Trpcount=0;
                
				fprintf(stderr,"Slave!\n");
				fflush(stderr);
                IT_COMMENT("Slave!");

				link_state = STAM_SLAVELINKAVAIL;

				set_link_state(link_state); //Tell application layer we have the link
			}
		}
	}   

    IT_EXIT;
}   

static void STAM_WAITFORREQ_moveto()   
{   
    IT_IT("STAM_WAITFORREQ_moveto");

	Trpflag=1;   
	Trpcount=0;
	link_state = STAM_EXREQLINK;

	set_link_state(link_state);

    IT_EXIT;
}   

static void STAM_SENDCON_moveto()   
{   
    int tmp;

    IT_IT("STAM_SENDCON_moveto");

	TxOneFrame(txbuffer,txlength);   
	rxlength=RxOneFrame(rxbuffer,IEC103_BUF_LEN);   
	Trpcount++;

	if((!rxlength)&&(Trpcount>TrpMax))   
	{   
		 fprintf(stderr,"Slave no resp\n");
		 fflush(stderr);
		 link_state = STAM_WAITFORREQ;

		 set_link_state(link_state);
	}   
	else   
	{   
		 DecodeFrame(rxbuffer,rxlength);

		 if((frame_error)&&(Trpcount > TrpMax))
		 {   
			fprintf(stderr,"Slave no resp\n");
			fflush(stderr);
			link_state = STAM_WAITFORREQ;

			set_link_state(link_state);
		 }   
		 else if(!rxlength) return;   
		 else if(!frame_error)   
		 {   
			 tmp = rxfunc;

			 if(tmp <= 15)
			 {
				switch(tmp)   
				{   
					case 0: link_state = STAM_SLAVELINKAVAIL; set_link_state(link_state); break;   
					case 1: fprintf(stderr,"Data refuse to receive\n"); fflush(stderr); link_state = STAM_SLAVELINKAVAIL; set_link_state(link_state); break;
					case 2:   
					case 3:   
					case 4: break;   
					case 5:   
					case 6: fprintf(stderr,"Data refuse to receive\n"); fflush(stderr); link_state = STAM_SLAVELINKAVAIL; set_link_state(link_state); break;
					default:;   
				}   
			 }
		 }   
	}
    
    IT_EXIT;
}   

static int RxOneFrame(u_char *rxbuf, int maxbuflen)   
{   
    int n, n_tot, length, off, i;
	unsigned char parc_buf[IEC103_BUF_LEN];
	int t1, t2;

    IT_IT("RxOneFrame");

	length = 0;
	n_tot = 0;
	n = 0;
	off = 0;

	memset(rxbuf, 0x00, maxbuflen);

	t1 = timeGetTime();

	do
	{
		off = off + n;

		read_from_serial((HANDLE)(global_s->fd), parc_buf, (unsigned long*) &n, IEC103_BUF_LEN, gl_read_timeout_ms);

		for(i = 0;i < n; i++)
		{
			unsigned char c = *((unsigned char*)parc_buf + i);
			fprintf(stderr,"rx <--- 0x%02x\n", c);
			fflush(stderr);
			IT_COMMENT1("rx <--- 0x%02x", c);
		}

		if(n > 0)
		{
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
		}
		else
		{
			fprintf(stderr,"Read %d bytes\n", n);
			fflush(stderr);
		}

		t2 = timeGetTime();
	}
	while((n_tot < length) && (t2 - t1 < gl_read_timeout_ms));

    IT_EXIT;

	return n_tot;
}

void State_process()   
{   
    IT_IT("State_process");

	while(state_proc_flag)
	{   
		switch(link_state)
		{   
			case STAM_LINKRESET: STAM_LINKRESET_moveto(); break;   
			case STAM_WAITFORREQ: STAM_WAITFORREQ_moveto(); break;   
			case STAM_EXREQLINK: STAM_EXREQLINK_moveto(); break;   
			case STAM_EXRESETLINK: STAM_EXRESETLINK_moveto(); break;   
			case STAM_SLAVELINKAVAIL: STAM_SLAVELINKAVAIL_moveto(); break;   
			case STAM_EXREQRES: STAM_EXREQRES_moveto(); break;   
			case STAM_SENDCON: STAM_SENDCON_moveto(); break;   
			default:;   
		}

#ifdef CHECK_TIMEOUT_WITH_PARENT		
		if(check_timeout_with_parent(polling_time))
		{
			state_proc_flag = 0;
		}
#endif

		Sleep(polling_time);

		process_timer_send_frame(global_s, NULL);
		iec103_run_send_queues(global_s);
	}

    IT_EXIT;
}   

static int make_longframe(u_char *txbuf, BYTE *inputbuf, int inputlen)   
{   
	u_char *tmptxbuf=txbuf;   
	int tmplen=0,len=0;   
	u_char cb=0,CS=0;   
	int sl_ad_size=0;   

    IT_IT("make_longframe");

	(*tmptxbuf++)=0x68;   
	tmplen=inputlen+GetSizeoflinkaddr()+1;   
	(*tmptxbuf++)=tmplen;   
	(*tmptxbuf++)=tmplen;   
	(*tmptxbuf++)=0x68;   
	//cb|=(txDIR<<7);   
	cb|=txPRM<<6;   
	cb|=FCB<<5;   
	cb|=FCV<<4;   
	cb|=txfunc;   
	(*tmptxbuf++)=cb;

	if (txfunc==4)
		sl_ad_size=SetlinkAddr(tmptxbuf,-1);
	else
		sl_ad_size=SetlinkAddr(tmptxbuf,slave_addr);

	tmptxbuf+=sl_ad_size;   
	memcpy(tmptxbuf,inputbuf,inputlen);   
	tmptxbuf+=inputlen;   
	CS=calc_chs(&txbuf[4],tmplen);   
	(*tmptxbuf++)=CS;   
	(*tmptxbuf++)=0x16;   

	len=tmptxbuf-txbuf;   

    IT_EXIT;

	return len;   
}   

static void process_incoming_data(u_char *rxbuf, int rxlen)   
{   
    //int i;
	struct iec_buf *b;

    IT_IT("process_incoming_data");
	
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

    IT_EXIT;
}   

static int EncodeFrameShort(u_char *txbuf)   
{   
    IT_IT("EncodeFrameShort");

    IT_EXIT;

	return(make_shortframe(txbuf));   
}   

int InitLink(struct iecserial *s)
{   
    IT_IT("InitLink");

	OutputBufferlength = 0;   
	slave_addr_size = LINK_103_ADDRLEN;   
	slave_addr = (unsigned short)gl_link_address;
	state_proc_flag = 1;   
	link_state = STAM_LINKRESET;   
    //link_state = STAM_EXRESETLINK;
	rxDIR = 0;   
	txDIR = 0;   
	rxPRM = 0;   
	txPRM = 1;   
	FCB=0;   
	ACD=0;   
	FCBtoggle=0;   
	FCV=1;   
	DFC=0;   
	txfunc = 0;
	rxfunc = 0;
	frame_error = 0;
	global_s = s;
	TrpMax = 3;   
	polling_time = gl_master_polling_time_in_milliseconds;
	reset_state_machines();

    IT_EXIT;

	return 1;   
}   

void set_link_state(int state)
{
    IT_IT("set_link_state");

	switch(state)
	{
		case STAM_SLAVELINKAVAIL: state_iec_103_link = LNK_CONNECTED; break;
		case STAM_WAITFORREQ: state_iec_103_link = LNK_IDLE; reset_state_machines(); break;
		default: break;
	}

    IT_EXIT;
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
	s->type = IEC_MASTER;
	
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

		for(i = 0; i < IEC_103_MAX_EVENTS; i++)
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

	for(i = 0; i < IEC_103_MAX_EVENTS; i++)
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

