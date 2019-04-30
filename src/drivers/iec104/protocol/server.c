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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>
#else
//#include <assert.h>
#include <errno.h>
#include <winsock2.h>
#endif
#include "event.h"
#ifndef WIN32
#include <sys/queue.h>
#endif

#include <compat/sys/queue.h>
#include "fifoc.h"
#include "clear_crc_eight.h"
#include "iec104.h"
#include "iec_library.h"
#include "iec104_types.h"
#include "itrace.h"
#include <signal.h>
#include <time.h>

//#define BACKLOG	1
#define TRUE 1

struct iechooks default_hooks;

void timer_send_frame(struct iecsock *s, void *arg)
{
	struct timeval tv;
	
	IT_IT("timer_send_frame");

	process_timer_send_frame(s,arg);
	//Run queues
	iecsock_run_send_queues(s);

	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	iecsock_user_timer_start(s, &tv);

	IT_EXIT;
}

void disconnect_hook(struct iecsock *s, short reason)
{	
	IT_IT("disconnect_hook");
	//fprintf(stderr, "%s: what=0x%02x\n", "disconnect_hook", reason);
	//fflush(stderr);

    fprintf(stderr, "Disconnect\n");
	fflush(stderr);

	reset_state_machines();

//	clearing_ftwd();

#ifdef REDUNDANT_SYSTEM
	#ifdef WIN32
	ExitProcess(0); //apa+++ on 06-12-2011 To force disconnection of master side on redundant system
	#elif __unix__
	exit(0);
	#endif
#endif

	IT_EXIT;
	return;
}

void data_received_hook(struct iecsock *s, struct iec_buf *b)
{
	IT_IT("data_received_hook");

	process_data_received_hook(s, b);

	IT_EXIT;
}

void activation_hook(struct iecsock *s)
{
	struct timeval tv;
	
	IT_IT("activation_hook");
	
	//fprintf(stderr, "%s: Success 0x%lu\n", "activation_hook", (unsigned long) s);
	//fflush(stderr);
	
	tv.tv_sec  = 1;
	tv.tv_usec = 0;

	iecsock_user_timer_set(s, timer_send_frame, NULL);
	iecsock_user_timer_start(s, &tv);
	IT_EXIT;
}

void connect_hook(struct iecsock *s)
{
	IT_IT("connect_hook");
	//fprintf(stderr, "%s: Success 0x%lu\n", "connect_hook", (unsigned long) s);
	//fflush(stderr);

    fprintf(stderr, "Connect\n");
	fflush(stderr);

	set_link_state();

	IT_EXIT;
}
