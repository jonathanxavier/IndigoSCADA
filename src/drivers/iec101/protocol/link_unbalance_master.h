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

#ifndef _LINK_UNBALANCE_MASTER 
#define _LINK_UNBALANCE_MASTER

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif
 
#define STAM_LINKRESET		1 
#define STAM_WAITFORREQ		2 
#define STAM_EXREQLINK		3 
#define STAM_EXRESETLINK	4 
#define STAM_SLAVELINKAVAIL 5 
#define STAM_EXREQRES		6 
#define STAM_SENDCON		7 
 
#define SENDNOREPLY	1 
#define SENDCONFIRM 2 

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


