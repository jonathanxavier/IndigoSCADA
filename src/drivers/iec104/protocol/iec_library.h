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
#ifndef __IECSOCK_H
#define __IECSOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_W	8
#define DEFAULT_K	12

#define DEFAULT_T0	30
#define DEFAULT_T1	15
#define DEFAULT_T2	10
#define DEFAULT_T3	20

#define IEC_LENGTH_MAX	253 //ASDU bytes + 4 control field octets
#define IEC_LENGHT_MIN	4 //4 control fields octets

#define t0_timer_stop(__s)	evtimer_del(&(__s)->t0_timer)
#define	t1_timer_stop(__s)	evtimer_del(&(__s)->t1_timer)
#define t2_timer_stop(__s)	evtimer_del(&(__s)->t2_timer)
#define t3_timer_stop(__s)	evtimer_del(&(__s)->t3_timer)

#define t0_timer_pending(__s)	evtimer_pending(&(__s)->t0_timer, NULL)
#define t1_timer_pending(__s)	evtimer_pending(&(__s)->t1_timer, NULL)
#define t2_timer_pending(__s)	evtimer_pending(&(__s)->t2_timer, NULL)
#define t3_timer_pending(__s)	evtimer_pending(&(__s)->t3_timer, NULL)

enum frame_type {
	FRAME_TYPE_I,
	FRAME_TYPE_S,
	FRAME_TYPE_U
};

enum uframe_func {
	STARTDTACT,
	STARTDTCON,
	STOPDTACT,
	STOPDTCON,
	TESTFRACT,
	TESTFRCON
};

#define IEC104_BUF_LEN		255 //cfr. pag. 23 APDU max = 255
#define	IEC_PORT_DEFAULT	2404 
#define IEC104_CTRL_LEN		4
#define IEC104_ASDU_MAX		249 //cfr. pag. 23 ASDU max = 249

enum {
	IEC_SLAVE,
	IEC_MASTER
};

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack(1)             /* Byte Alignment   */
#endif

struct iec_i {
	u_int	ft:1;
	u_int	ns:15;
	u_int	res:1;
	u_int	nr:15;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

struct iec_s {
	u_int	ft:1;
	u_int	res1:15;
	u_int	res2:1;
	u_int	nr:15;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

struct iec_u {
	u_char	ft:2;
	u_char	start_act:1;
	u_char	start_con:1;
	u_char	stop_act:1;
	u_char	stop_con:1;
	u_char	test_act:1;
	u_char	test_con:1;
	u_char	res1;
	u_short	res2;
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

struct iechdr {
		u_char	start;
		u_char	length;
		union  {	/* g++ doesn't allow anonymous unions with static data members */
			u_char	raw[1];
			struct iec_i ic;
			struct iec_s sc;
			struct iec_u uc;
		};
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

struct iec_buf {
	TAILQ_ENTRY(iec_buf) head;
	u_char	data_len;	/* actual ASDU length */
	struct iechdr h;
	u_char	data[1];
#ifndef WIN32
} __attribute__((__packed__));
#else
};
#endif

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack()              /* Byte Alignment   */
#endif

struct iecsock;

struct iechooks {
	void (*connect_indication)(struct iecsock *s);
	void (*activation_indication)(struct iecsock *s);
	void (*deactivation_indication)(struct iecsock *s);
	void (*disconnect_indication)(struct iecsock *s, short reason);
	void (*data_indication)(struct iecsock *s, struct iec_buf *b);
	void (*transmit_wakeup)(struct iecsock *s);
};

extern struct iechooks default_hooks;

TAILQ_HEAD(iec_buf_queue, iec_buf);

struct iecsock {
	int		sock;		/* socket descriptor */
	u_char		buf[IEC104_BUF_LEN];
	u_char		len;
	u_char		left;
	u_char		type;
	u_char		stopdt:1;	/* monitor direction 0=active 1=inactive */
	u_char		testfr:1;	/* test function 1=active 0=inactive */
	u_short		w, k;
	u_short		v_ack, vr, vs, v_ack_rem;
	u_short		t0, t1, t2, t3;
	struct event	t0_timer;
	struct event	t1_timer;
	struct event	t2_timer;
	struct event	t3_timer;
	struct sockaddr_in addr;	/* socket address */
	struct bufferevent *io;
	struct iec_buf_queue write_q;	/* write queue */
	struct iec_buf_queue ackw_q;	/* acknowledge wait queue */

	struct iec_buf_queue high_priority_q;
	struct iec_buf_queue spontaneous_q;

	struct iechooks hooks;
	struct event user;
	void	(*usercb)(struct iecsock *s, void *arg);
	void	*userarg;
	u_long recv_cnt, xmit_cnt;
};

struct iecsock_options {
	u_short		w;
	u_short		k;
	u_short		t0;
	u_short		t1;
	u_short		t2;
	u_short		t3;
};

void iecsock_prepare_iframe(struct iec_buf *buf);
void iecsock_run_write_queue(struct iecsock *s);
void iecsock_run_send_queues(struct iecsock *s);

int iecsock_listen(struct sockaddr_in *addr, int backlog);
int iecsock_connect(struct sockaddr_in *addr);
size_t iecsock_can_queue(struct iecsock *s);
void iecsock_set_options(struct iecsock *s, struct iecsock_options *opt);
void iecsock_set_hooks(struct iecsock *s, struct iechooks *hooks);
void iecsock_close(struct iecsock *s);
void iecsock_user_timer_set(struct iecsock *s, 
	void (*cb)(struct iecsock *s, void *arg), void *arg);
void iecsock_user_timer_start(struct iecsock *s, struct timeval *tv);
void iecsock_user_timer_stop(struct iecsock *s);
void iecsock_flush_queues(struct iecsock *s);

int initialize_win32_socket(void);
void deinitialize_win32_socket(void);

void timer_send_frame(struct iecsock *s, void *arg);
void disconnect_hook(struct iecsock *s, short reason);
void data_received_hook(struct iecsock *s, struct iec_buf *b);
void activation_hook(struct iecsock *s);
void connect_hook(struct iecsock *s);

#ifdef __cplusplus
}
#endif

#endif

/*
CHANNEL SETTINGS
Protocol Options

t0 - Timeout of Connection establishment(s) - Maximum waiting time, in seconds, for a client TCP/IP
connection establishment into the LISTENING port. After this time, this driver actively closes TCP/IP
socket and restart it to LISTENING state. Allowed values lies between 1 and 255.
t1 - Timeout of send or test APDUs(s) - Maximum acceptable time, in seconds, for the slave to send
regular or test APDUs after receiving the START DT sending confirmation. Allowed values lie between 1
and 255.
t2 - Timeout for ack in case of no data(s)- Maximum waiting time, in seconds, for a pending
acknowledgement before sending an acknowledgement for the last received message. A message with
the sequence number of last received. Values from 1 to 255 are allowed. Besides, t2 must be inferior to
t1.
t3 - Timeout for send test frames(s) - Maximum waiting time, in seconds, for the arrival of any
information (in case of a TCP-IP connection already established) before sending a TEST-FR. The Values
are allowed are from 1 to 255.

w - Send ack after received w I format APDUs - Number of information messages sent spontaneously to
client until it sends an "acknowledgment" with the sequence number of last message it received.
Allowed values lies between 1 and 32767.
k - Messages received to send state variable - Maximum allowed number of pending
acknowledgements before this slave stops sending new messages. The IEC standard recommends that
w is, at most, two thirds of the k value. Values allowed are between 1 and 32767

Comments:

T0 must always be higher than T3, these two times supervise the communication status when there is no data, 
if T3 is higher a disconnection is generated due to inactivity in the channel.

T1 must always be higher than T2, these two times monitor the communication status when there is fluid traffic, 
if T2 is lower it can generate disconnection due to lack of event confirmation.

K must always be greater than W, these two parameters ensure the performance of the channel in conditions of 
massive traffic. If K is lower than W, no disconnections will occur, however, for some moments the channel will 
be inactive (around T2) despite having data to send.

Generally when transmitting events on high latency channels, such as radio or satellite, disconnections may occur. 
In this case, T1 can be increased to extend the time for receiving the confirmation of sent events; 
and T2 can be reduced to confirm sequences faster.

*/