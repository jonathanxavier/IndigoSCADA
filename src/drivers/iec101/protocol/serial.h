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

#ifndef __SERIAL_H
#define __SERIAL_H

#if defined( __cplusplus) && defined(WIN32)
extern "C" {
#endif

extern int open_port(char* port_name, int baud_rate);
extern int read_from_serial(HANDLE fd, unsigned char* in_buf, unsigned long *bytesread, unsigned max_legth, int read_timeout);
extern unsigned int write_to_serial(HANDLE	port, unsigned char	*buffDati, unsigned int n_byte_to_write);
extern void close_port(int fd);

extern HANDLE          fd; //serial handle

#if defined( __cplusplus) && defined(WIN32)
}
#endif

#endif