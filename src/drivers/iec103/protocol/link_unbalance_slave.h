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

#ifndef _LINK_UNBALANCE_SLAVE 
#define _LINK_UNBALANCE_SLAVE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#define STAM_MASTERLINKAVAIL 1 
#define STAM_EXREQRES		2 
#define STAM_WAITREQSTATUS  3
#define STAM_STATUSOFLINK   4
#define STAM_POSITIVEACK    5
#define STAM_PROCESS_INCOMING_DATA 6

#define BROADCAST_ADDR  0xffff 

#ifdef __cplusplus
extern "C" {
#endif

int InitLink(struct iecserial *s); 
int CloseLink(); 
void State_process(); 

#ifdef __cplusplus
}
#endif

#endif 


