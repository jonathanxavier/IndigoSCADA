/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2009 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
//#include <assert.h>
#ifndef WIN32
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <arpa/inet.h>
#include <portable.h>

#define CLOSE_SOCKET close
#else

#define EINPROGRESS             WSAEINPROGRESS
#define CLOSE_SOCKET closesocket

#include <winsock2.h>
#include <WS2tcpip.h>

#endif

#include "event.h"

#include <queue.h>
#include "iec_library.h"

static enum frame_type frame_type(struct iechdr *h)
{
	if (!(h->raw[0] & 0x1))
		return FRAME_TYPE_I;
	else if (!(h->raw[0] & 0x2))
		return FRAME_TYPE_S;
	else
		return FRAME_TYPE_U; 
}

static enum uframe_func uframe_func(struct iechdr *h)
{
	if (h->raw[0] & 0x4)
		return STARTDTACT;
	else if (h->raw[0] & 0x8)
		return STARTDTCON;
	else if (h->raw[0] & 0x10)
		return STOPDTACT;
	else if (h->raw[0] & 0x20)
		return STOPDTCON;
	else if (h->raw[0] & 0x40)
		return TESTFRACT;
	else
		return TESTFRCON;
}

static char * uframe_func_to_string(enum uframe_func func)
{
	switch (func) {
	case STARTDTACT:
		return (char*)"STARTDTACT";
	case STARTDTCON:
		return (char*)"STARTDTCON";
	case STOPDTACT:
		return (char*)"STOPDTACT";
	case STOPDTCON:
		return (char*)"STOPDTCON";
	case TESTFRACT:
		return (char*)"TESTFRACT";
	case TESTFRCON:
		return (char*)"TESTFRCON";
	default:
		return (char*)"UNKNOWN";
	}
}

static char * frame_to_string(struct iechdr *h)
{
	switch (frame_type(h)) {
	case FRAME_TYPE_I:
		return (char*)"I";
	case FRAME_TYPE_S:
		return (char*)"S";
	case FRAME_TYPE_U:
		return (char*)"U";
	default:
		return (char*)"0";
	}
}

#include "fifoc.h"
#include "clear_crc_eight.h"
#include "iec104.h"

#define MODNAME "iecsock"

#ifndef WIN32

#define debug(format, arg...)						\
fprintf(stderr, "%s: " format ": %s\n", MODNAME, ##arg, strerror(errno)); \
fflush(stderr);

#define proto_debug(format, arg...)					\
fprintf(stderr, "%s: " format "\n", MODNAME, ##arg); \
fflush(stderr);

#endif //WIN32

#include "itrace.h"

static void iecsock_uframe_send(struct iecsock *s, enum uframe_func func)
{
	struct iechdr h;

	IT_IT("iecsock_uframe_send");
	
	memset(&h, 0, sizeof(struct iechdr));
	
	#ifndef WIN32
	proto_debug("TX U %s stopdt:%i", uframe_func_to_string(func), s->stopdt);
	#else
	fprintf(stderr, "TX U %s stopdt:%i\n", uframe_func_to_string(func), s->stopdt);
	fflush(stderr);	
	#endif

	IT_COMMENT2("TX U %s stopdt:%i", uframe_func_to_string(func), s->stopdt);


	h.start = 0x68;
	h.length = sizeof(struct iec_u);
	h.uc.ft = 3;
	if (func == STARTDTACT)
		h.uc.start_act = 1;
	else if (func == STARTDTCON)
		h.uc.start_con = 1;
	else if (func == STOPDTACT)
		h.uc.stop_act = 1;
	else if (func == STOPDTCON)
		h.uc.stop_con = 1;
	else if (func == TESTFRACT)
		h.uc.test_act = 1;
	else if (func == TESTFRCON)
		h.uc.test_con = 1;
	
	IT_COMMENT1("sizeof(struct iechdr) = %d", sizeof(struct iechdr));

	bufferevent_write(s->io, &h, sizeof(struct iechdr));
	s->xmit_cnt++;

	IT_EXIT;
}

static void iecsock_sframe_send(struct iecsock *s)
{
	struct iechdr h;
	IT_IT("iecsock_sframe_send");
	
	memset(&h, 0, sizeof(h));
	
	#ifndef WIN32
	proto_debug("TX S V(r)=%i", s->vr);
	#else
	fprintf(stderr, "TX S V(r)=%i\n", s->vr);
	fflush(stderr);
	#endif
	IT_COMMENT1("TX S V(r)=%i", s->vr);

	h.start = 0x68;
	h.length = sizeof(struct iec_s);
	h.sc.ft = 1;
	h.sc.nr = s->vr;	
	bufferevent_write(s->io, &h, sizeof(h));
	s->xmit_cnt++;

	IT_EXIT;
}

void iecsock_prepare_iframe(struct iec_buf *buf)
{
	struct iechdr *h;

	IT_IT("iecsock_prepare_iframe");
	
	h = &buf->h;
	h->start = 0x68;
	//IT_COMMENT4("buf->data_len = %d, sizeof(struct iec_i) = %d, sizeof(struct iec_u) = %d, sizeof(struct iec_s) = %d", buf->data_len, sizeof(struct iec_i), sizeof(struct iec_u), sizeof(struct iec_s));
	IT_COMMENT1("buf->data_len = %d", buf->data_len);
	h->length = buf->data_len + sizeof(struct iec_i);
	h->ic.ft = 0;

	IT_EXIT;
}

static void t1_timer_run(int nofd, short what, void *arg)
{
	struct iecsock *s = (struct iecsock *) arg;

	IT_IT("t1_timer_run");

	s->io->errorcb(s->io, what, s);

	IT_EXIT;
}

static void t1_timer_start(struct iecsock *s)
{
	struct timeval tv;

	IT_IT("t1_timer_start");

	tv.tv_usec = 0;
	tv.tv_sec = s->t1;
	evtimer_add(&s->t1_timer, &tv);

	IT_EXIT;
}

static void t2_timer_run(int nofd, short what, void *arg)
{
	struct iecsock *s = (struct iecsock *) arg;

	IT_IT("t2_timer_run");

	iecsock_sframe_send(s);
	s->v_ack_rem = s->vr;

	IT_EXIT;
}

static void t2_timer_start(struct iecsock *s)
{
	struct timeval tv;

	IT_IT("t2_timer_start");

	tv.tv_usec = 0;
	tv.tv_sec = s->t2;
	evtimer_add(&s->t2_timer, &tv);

	IT_EXIT;
}

static void t3_timer_start(struct iecsock *s)
{
	struct timeval tv;

	IT_IT("t3_timer_start");

	tv.tv_usec = 0;
	tv.tv_sec = s->t3;
	evtimer_add(&s->t3_timer, &tv);

	IT_EXIT;
}

static void t3_timer_run(int nofd, short what, void *arg)
{
	struct iecsock *s = (struct iecsock *) arg;

	IT_IT("t3_timer_run");

	t1_timer_start(s);
	s->testfr = 1;
	iecsock_uframe_send(s, TESTFRACT);

	IT_EXIT;
}

static void t0_timer_start(struct iecsock *s)
{
	struct timeval tv;

	IT_IT("t0_timer_start");

	tv.tv_usec = 0;
	tv.tv_sec = s->t0;
	evtimer_add(&s->t0_timer, &tv);  //T0 TIMER START

	IT_EXIT;
}

static void t0_timer_run(int nofd, short what, void *arg)
{
	struct iecsock *s = (struct iecsock *) arg;
	IT_IT("t0_timer_run");

	if(s->type == IEC_MASTER)
	{
		iecsock_connect(&s->addr);
		free(s);
	}
	else if(s->type == IEC_SLAVE)
	{
		s->io->errorcb(s->io, what, s); //For the SLAVE it is like t1_timer_run
	}

	IT_EXIT;
}

static void flush_queue(struct iec_buf_queue *q)
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

		for(i = 0; i < IEC_104_MAX_EVENTS; i++)
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

void iecsock_flush_queues(struct iecsock *s)
{
	IT_IT("iecsock_flush_queues");

	flush_queue(&s->write_q);
	flush_queue(&s->ackw_q);
	flush_queue(&s->high_priority_q);
	flush_queue(&s->spontaneous_q); 

	IT_EXIT;
}

extern int state_of_slave;

//apa 13-11-2010 
//Non accumulare nel buffer SO_SNDBUF del socket un numero superiore a 1460 bytes
//poiche e' possibile che il kernel mi tronca il buffer a 1460, tagliando
//i pacchetti iec contenuti

#define DEFAULT_SOCK_SNDBUF_SIZE 1460

static int bytes_sent;

void iecsock_run_send_queues(struct iecsock *s)
{
	struct iec_buf *n1;

	IT_IT("iecsock_run_send_queues");

	bytes_sent = 0;

    while(!TAILQ_EMPTY(&s->high_priority_q)) 
	{
	    n1 = TAILQ_FIRST(&s->high_priority_q);

		if((bytes_sent + n1->h.length + 2) > DEFAULT_SOCK_SNDBUF_SIZE)
		{
			IT_EXIT;
			return;
		}
	    
		if(iecsock_can_queue(s))
		{
			TAILQ_REMOVE(&s->high_priority_q, n1, head);

			//fprintf(stderr, "n1->data_len = %d\n", n1->data_len);
			//fflush(stderr);
			//fprintf(stderr, "n1->h.length  = %d\n", n1->h.length);
			//fflush(stderr);

			bytes_sent += n1->h.length + 2;
						
			TAILQ_INSERT_TAIL(&s->write_q, n1, head);
			iecsock_run_write_queue(s);
		}
		else
		{
			IT_EXIT;
			return;
		}
    }

	if(s->type == IEC_SLAVE)
	{
		//if(state_of_slave == SLAVE_SENDING_DATA) //the SLAVE is allowed to send spontaneous data
		{
		    while(!TAILQ_EMPTY(&s->spontaneous_q)) 
			{
				n1 = TAILQ_FIRST(&s->spontaneous_q);

				if((bytes_sent + n1->h.length + 2) > DEFAULT_SOCK_SNDBUF_SIZE)
				{
					IT_EXIT;
					return;
				}
				
				if(iecsock_can_queue(s))
				{
					TAILQ_REMOVE(&s->spontaneous_q, n1, head);

					//fprintf(stderr, "n1->data_len = %d\n", n1->data_len);
					//fflush(stderr);
					//fprintf(stderr, "n1->h.length  = %d\n", n1->h.length);
					//fflush(stderr);

					bytes_sent += n1->h.length + 2;
					
					TAILQ_INSERT_TAIL(&s->write_q, n1, head);
					iecsock_run_write_queue(s);
				}
				else
				{
					IT_EXIT;
					return;
				}
			}
		}
	}
	else if(s->type == IEC_MASTER)
	{
		while(!TAILQ_EMPTY(&s->spontaneous_q)) 
		{
			n1 = TAILQ_FIRST(&s->spontaneous_q);

			if((bytes_sent + n1->h.length + 2) > DEFAULT_SOCK_SNDBUF_SIZE)
			{
				IT_EXIT;
				return;
			}
			
			if(iecsock_can_queue(s))
			{
				TAILQ_REMOVE(&s->spontaneous_q, n1, head);

				//fprintf(stderr, "n1->data_len = %d\n", n1->data_len);
				//fflush(stderr);
				//fprintf(stderr, "n1->h.length  = %d\n", n1->h.length);
				//fflush(stderr);

				bytes_sent += n1->h.length + 2;
				
				TAILQ_INSERT_TAIL(&s->write_q, n1, head);
				iecsock_run_write_queue(s);
			}
			else
			{
				IT_EXIT;
				return;
			}
		}
	}

	IT_EXIT;
}

void iecsock_run_write_queue(struct iecsock *s)
{
	struct iechdr *h;
	struct iec_buf *b;

	IT_IT("iecsock_run_write_queue");
	
	if (s->type == IEC_SLAVE && s->stopdt)
	{	
		IT_EXIT;
		return;
	}

	// TailQ Deletion.
    while(!(TAILQ_EMPTY(&s->write_q)) && (s->vs != (s->v_ack + s->k) % 32768)) 
	{
	    b = TAILQ_FIRST(&s->write_q);
	    TAILQ_REMOVE(&s->write_q, b, head);

		h = &b->h;
		h->ic.nr = s->vr;
		h->ic.ns = s->vs;
		
		#ifndef WIN32
		proto_debug("TX I Ack=%d V(s)=%d V(r)=%d", s->v_ack, h->ic.ns, h->ic.nr);
		#endif

		IT_COMMENT3("TX I Ack=%d V(s)=%d V(r)=%d", s->v_ack, h->ic.ns, h->ic.nr);

		fprintf(stderr,"TX I Ack=%d V(s)=%d V(r)=%d\n", s->v_ack, h->ic.ns, h->ic.nr);
		fflush(stderr);

		if(t1_timer_pending(s))
		{
			t1_timer_stop(s);
		}

		t1_timer_start(s);
		
		bufferevent_write(s->io, h, h->length + 2);
		TAILQ_INSERT_TAIL(&s->ackw_q, b, head);

		IT_COMMENT1("Insert in ackw_q b = %x", b);
		
		s->vs = (s->vs + 1) % 32768;
		s->xmit_cnt++;
	}
	
	if (s->vs == (s->v_ack + s->k) % 32768)
	{
		#ifndef WIN32
		proto_debug("reached k, no frames will be sent");
		#endif
		IT_COMMENT("reached k, no frames will be sent");
	}

	IT_EXIT;
}

static void iecsock_run_ackw_queue(struct iecsock *s, unsigned short nr)
{
	struct iec_buf *b;
	int i;

	IT_IT("iecsock_run_ackw_queue");
	
	#ifndef WIN32
	proto_debug("received ack for V(s)=%d", nr);
	#else
	fprintf(stderr, "received ack for V(s)=%d\n", nr);
	fflush(stderr);
	#endif
	IT_COMMENT1("received ack for V(s)=%d", nr);


	//questo for (;;) deve rimuovere fino a b->h.ic.ns escluso

	//TailQ Deletion.
     while(!TAILQ_EMPTY(&s->ackw_q)) 
	 {
		b = TAILQ_FIRST(&s->ackw_q);

		if(b->h.ic.ns == nr) break;

		TAILQ_REMOVE(&s->ackw_q, b, head);

		IT_COMMENT1("free b->h.ic.ns = %d\n", b->h.ic.ns);

		//fprintf(stderr, "free b->h.ic.ns = %d\n", b->h.ic.ns); //apa+++ 03-11-2010
		//fflush(stderr); //apa+++ 03-11-2010

		for(i = 0; i < IEC_104_MAX_EVENTS; i++)
		{
			if(b == v_iec_buf[i].c)
			{
				v_iec_buf[i].used = 0;
				break;
			}
		}
     }

	IT_EXIT;
}

#ifndef WIN32
static inline int check_nr(struct iecsock *s, unsigned short nr)
#else
static int check_nr(struct iecsock *s, unsigned short nr)
#endif
{
	IT_IT("check_nr");
	//fprintf(stderr, "(V(r_rem) - s->v_ack + 32768) % 32768 = %d <= (s->vs - s->v_ack + 32768) % 32768 = %d\n", (nr - s->v_ack + 32768) % 32768, (s->vs - s->v_ack + 32768) % 32768);
	//fflush(stderr);
	IT_EXIT;
	return ((nr - s->v_ack + 32768) % 32768 <= (s->vs - s->v_ack + 32768) % 32768);
}

#ifndef WIN32
static inline int check_ns(struct iecsock *s, unsigned short ns)
#else
static int check_ns(struct iecsock *s, unsigned short ns)
#endif
{
	IT_IT("check_ns");
	//fprintf(stderr, "check_ns: V(s_rem) %d == V(r) = %d\n", ns, s->vr);
	//fflush(stderr);
	IT_EXIT;
	return (ns == s->vr);
}

static int iecsock_iframe_recv(struct iecsock *s, struct iec_buf *buf)
{
	struct iechdr *h;

	IT_IT("iecsock_iframe_recv");

	h = &buf->h; // equivale a &(buf->h)
	buf->data_len = h->length - 4;
	IT_COMMENT1("h->length = %d", h->length);
	
	if (!check_nr(s, (u_short)h->ic.nr))
	{
		char err_msg[100];
		iecsock_close(s);
		sprintf(err_msg, "!check_nr(s, (u_short)h->ic.nr)");
		iec_call_exit_handler(__LINE__, __FILE__, err_msg);

		IT_EXIT;
		return -1;
	}
	
	iecsock_run_ackw_queue(s, (u_short)h->ic.nr);
	
	s->v_ack = h->ic.nr;
	if (s->v_ack == s->vs) {
		t1_timer_stop(s);
		if (s->hooks.transmit_wakeup)
			s->hooks.transmit_wakeup(s);
		else if (default_hooks.transmit_wakeup)
			default_hooks.transmit_wakeup(s);
	}
	t2_timer_stop(s);
	t2_timer_start(s);
	
	if (!check_ns(s, (u_short)h->ic.ns))
	{
		

		char err_msg[100];
		iecsock_close(s);
		sprintf(err_msg, "s->vr != (u_short)h->ic.ns");
		iec_call_exit_handler(__LINE__, __FILE__, err_msg);

		IT_COMMENT2("s->vr != (u_short)h->ic.ns", s->vr, (u_short)h->ic.ns);
		IT_EXIT;
		return -1;
	}

	s->vr = (s->vr + 1) % 32768;
	if ((s->vr - s->v_ack_rem + 32768) % 32768 == s->w)
	{
		iecsock_sframe_send(s);
		s->v_ack_rem = s->vr;
	}

	if (s->hooks.data_indication)
	{
		s->hooks.data_indication(s, buf);
	}
	else if (default_hooks.data_indication)
	{
		default_hooks.data_indication(s, buf);
	}

	IT_EXIT;
	return 0;
}

static int iecsock_sframe_recv(struct iecsock *s, struct iec_buf *buf)
{
	struct iechdr *h;

	IT_IT("iecsock_sframe_recv");

	h = &buf->h;
	
	buf->data_len = h->length - 4;
		
	if (!check_nr(s, (u_short)h->ic.nr))
	{
		char err_msg[100];
		iecsock_close(s);
		sprintf(err_msg, "!check_nr(s, (u_short)h->ic.nr)");
		iec_call_exit_handler(__LINE__, __FILE__, err_msg);
		IT_EXIT;
		return -1;
	}
	
	iecsock_run_ackw_queue(s, (u_short)h->ic.nr);

	s->v_ack = h->ic.nr;
	if (s->v_ack == s->vs) {
		t1_timer_stop(s);
		if (s->hooks.transmit_wakeup)
			s->hooks.transmit_wakeup(s);
		else if (default_hooks.transmit_wakeup)
			default_hooks.transmit_wakeup(s);
	}
	
	IT_EXIT;
	return 0;
}

static int iecsock_uframe_recv(struct iecsock *s, struct iec_buf *buf)
{
	struct iechdr *h;

	IT_IT("iecsock_uframe_recv");

	h = &buf->h;

	switch(uframe_func(h)) {
	case STARTDTACT:
		if (s->type != IEC_SLAVE)
		{
			char err_msg[100];
			iecsock_close(s);
			sprintf(err_msg, "s->type != IEC_SLAVE");
			iec_call_exit_handler(__LINE__, __FILE__, err_msg);
			IT_EXIT;
			return -1;
		}
		#ifndef WIN32
		proto_debug("STARTDTACT changed stopdt to 0");
		#endif

		IT_COMMENT("STARTDTACT changed stopdt to 0");

		s->stopdt = 0;
		iecsock_uframe_send(s, STARTDTCON);
		iecsock_run_write_queue(s);
		if (s->hooks.activation_indication)
			s->hooks.activation_indication(s);
		else if (default_hooks.activation_indication)
			default_hooks.activation_indication(s);
	break;
	case STARTDTCON:
		if (s->type != IEC_MASTER)
		{
			char err_msg[100];
			iecsock_close(s);
			sprintf(err_msg, "s->type != IEC_MASTER");
			iec_call_exit_handler(__LINE__, __FILE__, err_msg);
			IT_EXIT;
			return -1;
		}
		t1_timer_stop(s);
		#ifndef WIN32
		proto_debug("STARTDTCON changed stopdt to 0");
		#endif
		IT_COMMENT("STARTDTCON changed stopdt to 0");

		s->stopdt = 0;
		if (s->hooks.activation_indication)
			s->hooks.activation_indication(s);
		else if (default_hooks.activation_indication)
			default_hooks.activation_indication(s);
	break;
	case STOPDTACT:
		if (s->type != IEC_SLAVE)
		{
			char err_msg[100];
			iecsock_close(s);
			sprintf(err_msg, "s->type != IEC_SLAVE");
			iec_call_exit_handler(__LINE__, __FILE__, err_msg);
			IT_EXIT;
			return -1;
		}
		s->stopdt = 1;
		iecsock_uframe_send(s, STOPDTCON);
		if (s->hooks.deactivation_indication)
			s->hooks.deactivation_indication(s);
		else if (default_hooks.deactivation_indication)
			default_hooks.deactivation_indication(s);
	break;
	case STOPDTCON:
		if (s->type != IEC_MASTER)
		{
			char err_msg[100];
			iecsock_close(s);
			sprintf(err_msg, "s->type != IEC_MASTER");
			iec_call_exit_handler(__LINE__, __FILE__, err_msg);
			IT_EXIT;
			return -1;
		}
		s->stopdt = 1;
		if (s->hooks.deactivation_indication)
			s->hooks.deactivation_indication(s);
		else if (default_hooks.deactivation_indication)
			default_hooks.deactivation_indication(s);
	break;
	case TESTFRACT:
		iecsock_uframe_send(s, TESTFRCON);
		/* SLAVE must not send TESTFR while recieving them from MASTER */
		if(s->type == IEC_SLAVE && !s->testfr)
		{
			t3_timer_stop(s);
			t0_timer_start(s); //apa+++ 23-11-2011
		}
	break;
	case TESTFRCON:
		if (!s->testfr)
		{
			char err_msg[100];
			iecsock_close(s);
			sprintf(err_msg, "!s->testfr");
			iec_call_exit_handler(__LINE__, __FILE__, err_msg);
			IT_EXIT;
			return -1;
		}
		t1_timer_stop(s);
		s->testfr = 0;
	break;
	}

	IT_EXIT;
	return 0;
}

static u_char receive_buffer[IEC104_BUF_LEN];

static int iecsock_frame_recv(struct iecsock *s)
{
	int ret = 0;
	struct iechdr *h;
	struct iec_buf *buf;

	IT_IT("iecsock_frame_recv");

	h = (struct iechdr *) &s->buf[0];

	buf = (struct iec_buf *)receive_buffer;
	memset(buf, 0x00, IEC104_BUF_LEN);

	//apa+++ 23-11-2011////////////////////
	if(s->type == IEC_SLAVE)
	{
		if(t0_timer_pending(s))
		{
			t0_timer_stop(s);
		}
	}
	/////end apa+++ 23-11-2011/////////////
	
	t3_timer_stop(s);
	t3_timer_start(s);
	
	memcpy(&buf->h, h, h->length + 2);
	
	switch (frame_type(h)) 
	{
		case FRAME_TYPE_I:
			if (s->type == IEC_SLAVE && s->stopdt) {
				#ifndef WIN32
				proto_debug("RX I in monitor direction not active");
				#endif
				IT_COMMENT("RX I in monitor direction not active");

				fprintf(stderr, "RX I in monitor direction not active\n");
				fflush(stderr);

				break;
			}
			#ifndef WIN32
			proto_debug("RX I len=%d V(r)=%d V(s)=%d Ack=%d V(ack_rem)=%d " 
			"N(r)=%d N(s)=%d", s->len, s->vr, s->vs, s->v_ack, s->v_ack_rem, 
			h->ic.nr, h->ic.ns);
			#endif

			IT_COMMENT7("RX I len=%d V(r)=%d V(s)=%d Ack=%d V(ack_rem)=%d " 
			"N(r)=%d N(s)=%d", s->len, s->vr, s->vs, s->v_ack, s->v_ack_rem, 
			h->ic.nr, h->ic.ns);
			
			fprintf(stderr,"RX I len=%d V(r)=%d V(s)=%d Ack=%d V(ack_rem)=%d " 
			"V(r_rem)=%d V(s_rem)=%d\n", s->len, s->vr, s->vs, s->v_ack, s->v_ack_rem, 
			h->ic.nr, h->ic.ns);
			
			fflush(stderr);

			ret = iecsock_iframe_recv(s, buf);

		break;
		case FRAME_TYPE_S:
			#ifndef WIN32
			proto_debug("RX S V(r)=%d V(s)=%d Ack=%d V(ack_rem)=%d " 
			"N(r)=%d", s->vr, s->vs, s->v_ack, s->v_ack_rem, h->sc.nr);
			#endif

			IT_COMMENT5("RX S V(r)=%d V(s)=%d Ack=%d V(ack_rem)=%d " 
			"N(r)=%d", s->vr, s->vs, s->v_ack, s->v_ack_rem, h->sc.nr);

			fprintf(stderr,"RX S V(r)=%d V(s)=%d Ack=%d V(ack_rem)=%d " 
			"V(r_rem)=%d\n", s->vr, s->vs, s->v_ack, s->v_ack_rem, h->sc.nr);

			fflush(stderr);

			ret = iecsock_sframe_recv(s, buf);

		break;
		case FRAME_TYPE_U:
			#ifndef WIN32
			proto_debug("RX U %s stopdt:%i", uframe_func_to_string(uframe_func(h)), s->stopdt);
			#endif

			IT_COMMENT2("RX U %s stopdt:%i", uframe_func_to_string(uframe_func(h)), s->stopdt);

			fprintf(stderr, "RX U %s stopdt:%i\n", uframe_func_to_string(uframe_func(h)), s->stopdt);
			fflush(stderr);

			ret = iecsock_uframe_recv(s, buf);

		break;
	}
	
	IT_EXIT;
	return ret;
}

static int iecsock_buffer_read(struct iecsock *s, int (*frame_recv)(struct iecsock *s))
{
	int ret;
	u_char	wm_read;
	struct iechdr *h;

	IT_IT("iecsock_buffer_read");
	
	if (!s->left) 
	{
		ret = bufferevent_read(s->io, s->buf, sizeof(struct iechdr));
		//assert(ret == sizeof(struct iechdr));
		if(ret != sizeof(struct iechdr))
		{
			char err_msg[100];
			iecsock_close(s);
			sprintf(err_msg, "iec 104 packet is not standard");
			
			iec_call_exit_handler(__LINE__, __FILE__, err_msg);
		}

		h = (struct iechdr *) &s->buf[0];

		if (h->start != 0x68 || h->length < IEC_LENGHT_MIN || h->length > IEC_LENGTH_MAX)
		{
			char err_msg[100];
			iecsock_close(s);
			sprintf(err_msg, "iec 104 packet is not standard");
			
			iec_call_exit_handler(__LINE__, __FILE__, err_msg);
		}

		s->left = h->length - IEC104_CTRL_LEN;
		s->len = sizeof(struct iechdr);
		wm_read = s->left;

		ret = 0;

		if (!s->left && !(ret = frame_recv(s))) 
		{
			wm_read = sizeof(struct iechdr);
			s->left = s->len = 0;
		}

		bufferevent_setwatermark(s->io, EV_READ, wm_read, 0);

		if(ret)
		{
			iecsock_close(s);
		}
	} 
	else 
	{
		ret = bufferevent_read(s->io, &s->buf[s->len], s->left);
		s->left -= ret;
		s->len += ret;

		if (s->left)
		{	
			IT_EXIT;
			return 0;
		}

		if(frame_recv(s))
		{
			char err_msg[100];
			iecsock_close(s);
			sprintf(err_msg, "frame_recv(s)");
			iec_call_exit_handler(__LINE__, __FILE__, err_msg);

			iecsock_close(s);
			ret = -1;
			IT_EXIT; 
			return ret; 
		}

		wm_read = sizeof(struct iechdr);
		s->left = s->len = 0;
		bufferevent_setwatermark(s->io, EV_READ, wm_read, 0);
	}

	IT_EXIT;
	return ret; 
}

static void bufreadcb(struct bufferevent *bufev, void *arg)
{
	struct iecsock *s = (struct iecsock *) arg;

	IT_IT("bufreadcb");
	
	while (EVBUFFER_LENGTH(s->io->input))
	{
		if(iecsock_buffer_read(s, iecsock_frame_recv) == -1) 
		{
			//exit process
			char err_msg[100];
			sprintf(err_msg, "Exit due to network error");
			iec_call_exit_handler(__LINE__,__FILE__, err_msg);
			IT_EXIT;
			return; 
		}
	}

	IT_EXIT;
	return;
}

static void bufwritecb(struct bufferevent *bufev, void *arg)
{
	IT_IT("bufwritecb");
	IT_EXIT;
	return;
}

static void buferrorcb(struct bufferevent *bufev, short what, void *arg)
{
	struct iecsock *s = (struct iecsock *) arg;

	IT_IT("buferrorcb");
	
	if (s->hooks.disconnect_indication)
		s->hooks.disconnect_indication(s, what);
	else if (default_hooks.disconnect_indication)	
		default_hooks.disconnect_indication(s, what);

	iecsock_close(s);

	IT_EXIT;

}

static void iecsock_set_defaults(struct iecsock *s)
{
	IT_IT("iecsock_set_defaults");

	s->t0	= DEFAULT_T0;
	s->t1	= DEFAULT_T1;
	s->t2	= DEFAULT_T2;
	s->t3	= DEFAULT_T3;
	s->w 	= DEFAULT_W;
	s->k	= DEFAULT_K;
	TAILQ_INIT(&s->write_q);
	TAILQ_INIT(&s->ackw_q);
	TAILQ_INIT(&s->high_priority_q);
	TAILQ_INIT(&s->spontaneous_q);

	evtimer_set(&s->t0_timer, t0_timer_run, s);
	evtimer_set(&s->t1_timer, t1_timer_run, s);
	evtimer_set(&s->t2_timer, t2_timer_run, s);
	evtimer_set(&s->t3_timer, t3_timer_run, s);

	IT_EXIT;
}

#ifndef socklen_t
#define socklen_t unsigned int
#endif

static void connect_writecb(int sock, short what, void *arg)
{
	socklen_t slen;
	int ret;
    #ifndef WIN32
	int opt;
    #else
    char opt;
    #endif
	struct iecsock *s = (struct iecsock *) arg;

	IT_IT("connect_writecb");
	
	if(what & EV_TIMEOUT) 
	{
		while(CLOSE_SOCKET(s->sock) != 0 && errno == EINTR);

		t0_timer_start(s);
		
		IT_EXIT;
		return;
	}
	
	slen = sizeof(opt);

	ret = getsockopt(s->sock, SOL_SOCKET, SO_ERROR, &opt, &slen);

	if(opt != 0) 
	{
		while(CLOSE_SOCKET(s->sock) != 0 && errno == EINTR);

		t0_timer_start(s);

		IT_EXIT;
		return;
	}
		
	t0_timer_stop(s);
	
	s->io = bufferevent_new(s->sock, bufreadcb, bufwritecb, buferrorcb, s);

	if(!s->io)
	{
		while(CLOSE_SOCKET(s->sock) != 0 && errno == EINTR);
		free(s);
	}
	
	bufferevent_setwatermark(s->io, EV_READ, sizeof(struct iechdr), 0);
	bufferevent_enable(s->io, EV_READ);
	
	s->stopdt = 1;
	iecsock_uframe_send(s, STARTDTACT);
	t1_timer_start(s);
	
	if (default_hooks.connect_indication)
		default_hooks.connect_indication(s);

	IT_EXIT;
	return;
}

#define SOCK_NO_DELAY

/**
* Applications that expect real time responses can react poorly with 
* Nagle's algorithm. Applications such as networked multiplayer video 
* games expect that actions in the game are sent immediately, while the 
* algorithm purposefully delays transmission, increasing bandwidth at the 
* expense of latency. For this reason applications with low-bandwidth 
* time-sensitive transmissions typically use TCP_NODELAY to bypass the 
* Nagle delay.
*/

#define SOCK_LINGER
#define LINGER_TIME 10

//#define SOCK_SNDBUF_SIZE 4096 //the default in Win32 is 1460 bytes

static void listen_readcb(int sock, short what, void *arg)
{
	int ret;
	#ifndef WIN32
    int opt;
	u_long sflags;
	#else
    char opt;
    #endif
	socklen_t slen;
	struct iecsock *s;
	struct event *evt = (struct event *) arg;
	#ifdef SOCK_NO_DELAY
	int enabled = 1;
	#endif
	#ifdef SOCK_LINGER
        static struct linger l = {1, LINGER_TIME};
	#endif
	#ifdef SOCK_SNDBUF_SIZE
        int size = SOCK_SNDBUF_SIZE;
	#endif


	IT_IT("listen_readcb");
	
	#ifndef REDUNDANT_SYSTEM
	event_add(evt, NULL);
	#endif
	
	s = (struct iecsock *)calloc(1, sizeof(struct iecsock));
	if (!s)
	{
		IT_EXIT;
		return;
	}
	
	iecsock_set_defaults(s);	
	s->type = IEC_SLAVE;
	s->stopdt = 1;
		
	slen = sizeof(s->addr);
	s->sock = accept(sock, (struct sockaddr *) &s->addr, &slen);
	if (s->sock == -1) {
		free(s);
		IT_EXIT;
		return;
	}


	fprintf(stderr, "Connected address %s:%d\n", inet_ntoa(s->addr.sin_addr), ntohs(s->addr.sin_port));
	fflush(stderr);

	#ifdef REDUNDANT_SYSTEM
	CLOSE_SOCKET(sock);
	#endif
	
	opt = 1;
	#ifndef WIN32
	sflags = fcntl(s->sock, F_GETFL);
	ret = fcntl(s->sock, F_SETFL, O_NONBLOCK | sflags);
	#else
	{
	unsigned long nonblocking = 1;
	ret = ioctlsocket(s->sock, FIONBIO, (unsigned long*) &nonblocking);
	}
	#endif

	if (ret == -1)
		goto error_bufev;

	ret = setsockopt(s->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (ret == -1)
		goto error_bufev;

#ifdef SOCK_LINGER
        ret = setsockopt(s->sock, SOL_SOCKET, SO_LINGER, (char*)&l, sizeof l);
#endif

	// wait for a stable status of the socket
	Sleep(10);

#ifdef SOCK_NO_DELAY
	enabled = 1;
	ret = setsockopt(s->sock, IPPROTO_TCP, TCP_NODELAY, (char*)&enabled,sizeof enabled);

	if(ret == -1)
	{
		goto error_bufev;
	}
#endif

#ifdef SOCK_SNDBUF_SIZE
        setsockopt(s->sock, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof size);
#endif
	
	s->io = bufferevent_new(s->sock, bufreadcb, bufwritecb, buferrorcb, s);
	if (!s->io)
		goto error_bufev;
	
	bufferevent_setwatermark(s->io, EV_READ, sizeof(struct iechdr), 0);
	bufferevent_enable(s->io, EV_READ);
	
	if (default_hooks.connect_indication)
		default_hooks.connect_indication(s);

	IT_EXIT;	
	return;
error_bufev:
	while (CLOSE_SOCKET(s->sock) != 0 && errno == EINTR);			
	free(s);
	IT_EXIT;
	return;
}

/**
 * User can use his own specific timer and register a callback on it's arrival.
 */
 
static void iecsock_user_timer_run(int nofd, short what, void *arg)
{
	struct iecsock *s = (struct iecsock *) arg;

	IT_IT("iecsock_user_timer_run");

	if (s->usercb)
		s->usercb(s, s->userarg);

	IT_EXIT;
}

void iecsock_user_timer_set(struct iecsock *s, 
	void (*cb)(struct iecsock *s, void *arg), void *arg)
{
	IT_IT("iecsock_user_timer_set");

	if (cb == NULL)
	{
		IT_EXIT;
		return;
	}
	s->usercb = cb;
	s->userarg = arg;
	evtimer_set(&s->user, iecsock_user_timer_run, s);

	IT_EXIT;
}

void iecsock_user_timer_start(struct iecsock *s, struct timeval *tv)
{
	IT_IT("iecsock_user_timer_start");

	if (evtimer_initialized(&s->user))
		evtimer_add(&s->user, tv);

	IT_EXIT;
}

void iecsock_user_timer_stop(struct iecsock *s)
{
	IT_IT("iecsock_user_timer_stop");

	evtimer_del(&s->user);

	IT_EXIT;
}

/**
 * iecsock_can_queue - check current window size
 * @param s : iecsock session
 * @return : number of acceptable ASDU's for transmission
 */
size_t iecsock_can_queue(struct iecsock *s)
{
	IT_IT("iecsock_can_queue");
	IT_EXIT;
	return (s->k - ((s->vs - s->v_ack + 32768) % 32768));
}

/**
 * iecsock_set_options - set IEC 870-5-104 specific options
 * @param s : iecsock session
 * @param opt: protocol specific options
 *
 * Please, refere to protocol specification!
 */
void iecsock_set_options(struct iecsock *s, struct iecsock_options *opt)
{
	IT_IT("iecsock_set_options");

	s->t0 = opt->t0;
	s->t1 = opt->t1;
	s->t2 = opt->t2;
	s->t3 = opt->t3;
	s->w = opt->w;
	s->k = opt->k;

	IT_EXIT;
}

/**
 * iecsock_set_hooks - set session specific hooks
 * @param s : iecsock session
 * @param hooks : session specific hooks to call on event
 *
 * User can provide his own specific hooks for each event he is interested in.
 * NULL-pointers in hooks structure mean that default hooks are prefered.
 *
 * connect_indication - called when link layer connection is established.
 *
 * disconnect_indication - called when link layer connection terminates.
 *
 * activation_indication - called when monitor direction activates with 
 * STARTDTACT/STARTDTCON S-frames.
 *
 * deactivation_indication - called when monitor direction deactivates 
 * with STOPDTACT/STOPDTCON S-frames.
 *
 * data_activation - called when ASDU was received, buf points to 
 * allocated structure. It is user responsibility to free allocated resources.
 *
 * transmit_wakeup - called when all frames from transmition queue were
 * sent, acknowledged and iecsock can accept more.
 */
void iecsock_set_hooks(struct iecsock *s, struct iechooks *hooks)
{
	IT_IT("iecsock_set_hooks");
	s->hooks.connect_indication = hooks->connect_indication;
	s->hooks.disconnect_indication = hooks->disconnect_indication;
	s->hooks.activation_indication = hooks->activation_indication;
	s->hooks.deactivation_indication = hooks->deactivation_indication;
	s->hooks.data_indication = hooks->data_indication;
	s->hooks.transmit_wakeup = hooks->transmit_wakeup;
	IT_EXIT;
}


/**
 * iecsock_connect - register master station connection
 * @param addr : slave station host address.
 * @return : 0 on success -1 on error
 *
 * iecsock_connect is used to register master station connection based on 
 * buffered events. Many iecsock_connect connections can be registered. 
 * iecsock_connect will try to connect to slave station host even if it 
 * refuses connection or timeout occures after DEFAULT_T3 seconds and 
 * stays persistent untill explicitely disconnected by iecsock_close().
 *
 * After all the master station connections have been registered, user 
 * calls event_dispatch() to process events.
 */
int iecsock_connect(struct sockaddr_in *addr)
{
	int ret;
	socklen_t slen;
	#ifndef WIN32
	u_long flags;
	#endif
	struct iecsock *s;
	//struct timeval tv;
	#ifdef SOCK_NO_DELAY
	int enabled = 1;
	#endif
//	#ifdef SOCK_LINGER
//        static struct linger l = {1, LINGER_TIME};
//	#endif

	IT_IT("iecsock_connect");
	
	s = (struct iecsock *)calloc(1, sizeof(struct iecsock));
	if (!s)
	{
		IT_EXIT;
		return(-1);
	}
	
	if ((s->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		free(s);
		IT_EXIT;
		return(-1);
	}
		
	iecsock_set_defaults(s);
	s->type = IEC_MASTER;
	
	if (addr) {
		s->addr.sin_family = addr->sin_family;
		s->addr.sin_port = addr->sin_port;
		s->addr.sin_addr.s_addr = addr->sin_addr.s_addr;
	} else {
		slen = sizeof(struct sockaddr_in);
		s->addr.sin_family = AF_INET;
		s->addr.sin_port = htons(IEC_PORT_DEFAULT);
		#ifndef WIN32
		inet_pton(AF_INET, "127.0.0.1", &s->addr.sin_addr);
		#else
		s->addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		#endif
	}

	ret = connect(s->sock, (struct sockaddr *) &s->addr, sizeof(s->addr));
	if (ret == -1 && errno != EINPROGRESS)
	{	
		goto error_sock;
	}

//#ifdef SOCK_LINGER
//        ret = setsockopt(s->sock, SOL_SOCKET, SO_LINGER, (char*)&l, sizeof l);
//#endif

	#ifndef WIN32
	flags = fcntl(s->sock, F_GETFL);
	ret = fcntl(s->sock, F_SETFL, O_NONBLOCK | flags);
	#else
	{
	unsigned long nonblocking = 1;
	ret = ioctlsocket(s->sock, FIONBIO, (unsigned long*) &nonblocking);
	}
	#endif

	if (ret == -1)
		goto error_sock;

	// wait for a stable status of the socket
	Sleep(10);

#ifdef SOCK_NO_DELAY
	enabled = 1;
	ret = setsockopt(s->sock, IPPROTO_TCP, TCP_NODELAY, (char*)&enabled,sizeof enabled);
	if (ret == -1)
		goto error_sock;
#endif
	
	event_set(&s->t0_timer, s->sock, EV_WRITE, connect_writecb, s);

	t0_timer_start(s);
	
	IT_EXIT;
	return(0);
error_sock:
	while (CLOSE_SOCKET(s->sock) != 0 && errno == EINTR);
	free(s);
	fprintf(stderr, "\nA wild error occurs.\n");
	fflush(stderr);
	IT_EXIT;
	return(-1);
}

/**
 * iecsock_close - close iecsock session and free occupied resources
 * @param s : iecsock session to close
 * 
 * iecsock_close MUST be called carefully! It is intended to be called only 
 * in bufferevent callbacks when nobody references iecsock session pointer.
 *
 */
void iecsock_close(struct iecsock *s)
{
	IT_IT("iecsock_close");

	//iecsock_flush_queues(s); //apa commented out on 02-11-2010

	flush_queue(&s->ackw_q); //apa added on 03-11-2010 Rimuovo solo questa coda alla disconnessione

	t0_timer_stop(s);
	t1_timer_stop(s);
	t2_timer_stop(s);
	t3_timer_stop(s);

	iecsock_user_timer_stop(s);

	bufferevent_disable(s->io, EV_READ);
	bufferevent_free(s->io);

	while (CLOSE_SOCKET(s->sock) != 0 && errno == EINTR);

	if(s->type == IEC_MASTER)
	{
		iecsock_connect(&s->addr);
	}

	free(s);

	s = NULL; 

	IT_EXIT;
}

/**
 * iecsock_listen - register slave station listener
 * @param addr : slave station address and port. If NULL provided, listens on 
 * all available addresses.
 * @param backlog : backlog socket parameter.
 * @return : 0 on success -1 on error
 *
 * iecsock_listen is used to register slave station listening process based on
 * buffered events. Each new master connection creates new socket and new 
 * buffered event. User can later register his own hooks in events he is 
 * interested.
 *
 * After all the master station connections have been registered, user 
 * calls event_dispatch() to process events.
 */
int iecsock_listen(struct sockaddr_in *addr, int backlog)
{
	int sock, ret;
	
	#ifndef WIN32
    int opt;
	u_long flags;
	#else
    char opt;
    #endif
	struct sockaddr_in sock_addr;
	struct event *evt;

	IT_IT("iecsock_listen");

	evt = (struct event *)calloc(1, sizeof(struct event));
	if (!evt)
	{
		IT_EXIT;
		return(-1);
	}
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		free(evt);
		IT_EXIT;
		return(-1);
	}
        
                
	#ifndef WIN32
	flags = fcntl(sock, F_GETFL);
	ret = fcntl(sock, F_SETFL, O_NONBLOCK | flags);
	#else
	{
	unsigned long nonblocking = 1;
	ret = ioctlsocket(sock, FIONBIO, (unsigned long*) &nonblocking);
	}
	#endif

	if (ret == -1)
		goto error_sock;
	
	opt = 0;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret == -1)
		goto error_sock;
	
	if (!addr) {
		memset(&sock_addr, 0, sizeof(sock_addr));
		sock_addr.sin_family = AF_INET;
		sock_addr.sin_port = htons(IEC_PORT_DEFAULT);
		sock_addr.sin_addr.s_addr = INADDR_ANY;
	} else
		sock_addr = *addr;
	
	ret = bind(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
	if (ret == -1)
		goto error_sock;

	ret = listen(sock, backlog);
	if (ret == -1)
		goto error_sock;
	
	event_set(evt, sock, EV_READ, listen_readcb, evt);
	event_add(evt, NULL);
	
	IT_EXIT;
	return(0);
error_sock:
	while (CLOSE_SOCKET(sock) != 0 && errno == EINTR);
	free(evt);
	IT_EXIT;
	return(-1);
}

int initialize_win32_socket(void)
{
	#ifdef WIN32
	WSADATA wsa;
	#endif
	IT_IT("initialize_win32_socket");
	#ifdef WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
	{
        /* Tell the user that we could not find a usable WinSock DLL. */
		fprintf(stderr,"Failed to initialize windows sockets: %d\n", WSAGetLastError());
		IT_EXIT;
		return -1;
    }

	if (LOBYTE (wsa.wVersion) != 2 || HIBYTE (wsa.wVersion) != 2)
    {
       WSACleanup();
	   IT_EXIT;
       return -1;
    }
	#endif

	IT_EXIT;
	return 0;
}

void deinitialize_win32_socket(void)
{
	IT_IT("deinitialize_win32_socket");
	#ifdef WIN32
    WSACleanup();
	#endif
	IT_EXIT;
}


/*
TAIL QUEUES
     A tail queue is headed by a structure defined by the TAILQ_HEAD macro.
     This structure contains a pair of pointers, one to the first element in
     the tail queue and the other to the last element in the tail queue.  The
     elements are doubly linked so that an arbitrary element can be removed
     without traversing the tail queue.  New elements can be added to the tail
     queue after an existing element, before an existing element, at the head
     of the tail queue, or at the end of the tail queue.  A TAILQ_HEAD struc-
     ture is declared as follows:

	   TAILQ_HEAD(HEADNAME, TYPE) head;

     where HEADNAME is the name of the structure to be defined, and TYPE is
     the type of the elements to be linked into the tail queue.  A pointer to
     the head of the tail queue can later be declared as:

	   struct HEADNAME *headp;

     (The names head and headp are user selectable.)

     The macro TAILQ_HEAD_INITIALIZER evaluates to an initializer for the tail
     queue head.

     The macro TAILQ_CONCAT concatenates the tail queue headed by head2 onto
     the end of the one headed by head1 removing all entries from the former.

     The macro TAILQ_EMPTY evaluates to true if there are no items on the tail
     queue.

     The macro TAILQ_ENTRY declares a structure that connects the elements in
     the tail queue.

     The macro TAILQ_FIRST returns the first item on the tail queue or NULL if
     the tail queue is empty.

     The macro TAILQ_FOREACH traverses the tail queue referenced by head in
     the forward direction, assigning each element in turn to var.  var is set
     to NULL if the loop completes normally, or if there were no elements.

     The macro TAILQ_FOREACH_REVERSE traverses the tail queue referenced by
     head in the reverse direction, assigning each element in turn to var.

     The macros TAILQ_FOREACH_SAFE and TAILQ_FOREACH_REVERSE_SAFE traverse the
     list referenced by head in the forward or reverse direction respectively,
     assigning each element in turn to var.  However, unlike their unsafe
     counterparts, TAILQ_FOREACH and TAILQ_FOREACH_REVERSE permit to both
     remove var as well as free it from within the loop safely without inter-
     fering with the traversal.

     The macro TAILQ_INIT initializes the tail queue referenced by head.

     The macro TAILQ_INSERT_HEAD inserts the new element elm at the head of
     the tail queue.

     The macro TAILQ_INSERT_TAIL inserts the new element elm at the end of the
     tail queue.

     The macro TAILQ_INSERT_AFTER inserts the new element elm after the ele-
     ment listelm.

     The macro TAILQ_INSERT_BEFORE inserts the new element elm before the ele-
     ment listelm.

     The macro TAILQ_LAST returns the last item on the tail queue.  If the
     tail queue is empty the return value is NULL.

     The macro TAILQ_NEXT returns the next item on the tail queue, or NULL if
     this item is the last.

     The macro TAILQ_PREV returns the previous item on the tail queue, or NULL
     if this item is the first.

     The macro TAILQ_REMOVE removes the element elm from the tail queue.

TAIL QUEUE EXAMPLE
     TAILQ_HEAD(tailhead, entry) head =
	 TAILQ_HEAD_INITIALIZER(head);
     struct tailhead *headp;		     // Tail queue head.
     struct entry {
	     ...
	     TAILQ_ENTRY(entry) entries;     // Tail queue.
	     ...
     } *n1, *n2, *n3, *np;

     TAILQ_INIT(&head); 		     // Initialize the queue.

     n1 = malloc(sizeof(struct entry));      // Insert at the head.
     TAILQ_INSERT_HEAD(&head, n1, entries);

     n1 = malloc(sizeof(struct entry));      // Insert at the tail.
     TAILQ_INSERT_TAIL(&head, n1, entries);

     n2 = malloc(sizeof(struct entry));      // Insert after.
     TAILQ_INSERT_AFTER(&head, n1, n2, entries);

     n3 = malloc(sizeof(struct entry));      // Insert before.
     TAILQ_INSERT_BEFORE(n2, n3, entries);

     TAILQ_REMOVE(&head, n2, entries);	     // Deletion.
     free(n2);
					     // Forward traversal.
     TAILQ_FOREACH(np, &head, entries)
	     np-> ...
					     // Safe forward traversal.
     TAILQ_FOREACH_SAFE(np, &head, entries, np_temp) {
	     np->do_stuff();
	     ...
	     TAILQ_REMOVE(&head, np, entries);
	     free(np);
     }
					     // Reverse traversal.
     TAILQ_FOREACH_REVERSE(np, &head, tailhead, entries)
	     np-> ...
					     // TailQ Deletion.
     while (!TAILQ_EMPTY(&head)) {
	     n1 = TAILQ_FIRST(&head);
	     TAILQ_REMOVE(&head, n1, entries);
	     free(n1);
     }
					     // Faster TailQ Deletion.
     n1 = TAILQ_FIRST(&head);
     while (n1 != NULL) {
	     n2 = TAILQ_NEXT(n1, entries);
	     free(n1);
	     n1 = n2;
     }
     TAILQ_INIT(&head);
*/

/*

Where T1 is the simulator
R1 is iec104slave.exe

12,277      12:01:37 PM,363  T1  Disconnected

12,278      12:01:37 PM,365  T1  Port closed

12,279      12:01:38 PM,928  T1  Port opened

12,280      12:01:38 PM,932  T1  Connected with: 172.28.20.20 Port: 2404

12,281      12:01:39 PM,787  
T1       :  StartDT: act

12,282      12:01:39 PM,787  
R1       :  StartDT: con

12,283      12:02:00 PM,149  
R1       :  TestFR: act

12,284      12:02:00 PM,150  
T1       :  TestFR: con

12,285      12:02:20 PM,182  
R1       :  TestFR: act

12,286      12:02:20 PM,183  
T1       :  TestFR: con

12,287      12:02:40 PM,213  
R1       :  TestFR: act

12,288      12:02:40 PM,215  
T1       :  TestFR: con

12,289      12:03:00 PM,245  
R1       :  TestFR: act

12,290      12:03:00 PM,247  
T1       :  TestFR: con

12,291      12:03:20 PM,277  
R1       :  TestFR: act

12,292      12:03:20 PM,278  
T1       :  TestFR: con

12,293      12:03:40 PM,309  
R1       :  TestFR: act

12,294      12:03:40 PM,310  
T1       :  TestFR: con
*/
