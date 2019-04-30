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
#ifndef __IECSERIAL_H
#define __IECSERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define IEC103_BUF_LEN		255
#define IEC103_ASDU_MAX		249 //IEC103_BUF_LEN - (4 octetcs for header(0x68 + length+ length + 0x68) + 1 octect for checksum + 1 octect for end character (0x16))

enum {
	IEC_SLAVE,
	IEC_MASTER
};

#if defined( _MSC_VER)          /* Microsoft C */
    #pragma pack(1)             /* Byte Alignment   */
#endif

struct iechdr {
        u_char	start01;
        u_char	length01;
        u_char	length02;
        u_char	start02;
        u_char	control_field;
        u_char	link_addr;
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

TAILQ_HEAD(iec_buf_queue, iec_buf);

struct iecserial {
	int		fd;		/* serial file descriptor */
	u_char		type;
	struct iec_buf_queue write_q;	/* write queue */
	struct iec_buf_queue high_priority_q;
	struct iec_buf_queue spontaneous_q;
};

void iecserial_prepare_iframe(struct iec_buf *buf);
int iecserial_connect(char* port_name, int baud_rate);
void iecserial_flush_queues(struct iecserial *s);

#ifdef __cplusplus
}
#endif

#endif
