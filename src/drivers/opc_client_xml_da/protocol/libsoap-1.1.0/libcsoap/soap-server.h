/******************************************************************
 *  $Id: soap-server.h,v 1.10 2006/03/29 08:35:56 m0gg Exp $
 *
 * CSOAP Project:  A SOAP client/server library in C
 * Copyright (C) 2003  Ferhat Ayaz
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 * 
 * Email: ferhatayaz@yahoo.com
 ******************************************************************/
#ifndef cSOAP_SERVER_H
#define cSOAP_SERVER_H

#include <libcsoap/soap-env.h>
#include <libcsoap/soap-router.h>
#include <libcsoap/soap-ctx.h>

typedef struct _SoapRouterNode
{
  char *context;
  SoapRouter *router;
  struct _SoapRouterNode *next;

} SoapRouterNode;

#ifdef __cplusplus
extern "C" {
#endif

/**
   Initializes the soap server with commandline arguments.

   <TABLE border=1>
   <TR><TH>Argument</TH><TH>Description</TH></TR>
   <TR><TD>-NHTTPport [port]</TD><TD>Port to listen (default: 10000)</TD></TR>
   <TR><TD>-NHTTPmaxconn [num]</TD><TD>Maximum thread connections</TD></TR>
   <TR><TD>-NHTTPlog [logfilename]</TD><TD>logfile</TD></TR>
   </TABLE>

   @param argc commandline arg count
   @param argv commandline arg vector

   @returns 1 if success, 0 otherwise
 */
herror_t soap_server_init_args(int argc, char *argv[]);


/**
   Register a router to the soap server. 

   <P>http://<I>host</I>:<I>port</I>/<B>[context]</B>


   @param router The router to register
   @param context the url context 
   @returns 1 if success, 0 otherwise
   
   @see soap_router_new
   @see soap_router_register_service

 */
int soap_server_register_router(SoapRouter * router, const char *context);

SoapRouter *soap_server_find_router(const char *context);

SoapRouterNode * soap_server_get_routers(void);

/**
   Enters the server loop and starts to listen to 
   http requests.
 */
herror_t soap_server_run();

int soap_server_get_port(void);

/**
   Frees the soap server.
 */
void soap_server_destroy();

#ifdef __cplusplus
}
#endif

#endif
