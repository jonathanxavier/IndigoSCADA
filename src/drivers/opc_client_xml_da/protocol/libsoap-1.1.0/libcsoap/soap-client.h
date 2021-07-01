/******************************************************************
 *  $Id: soap-client.h,v 1.13 2006/03/06 13:37:38 m0gg Exp $
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
 * Email: ayaz@jprogrammer.net
 ******************************************************************/
#ifndef cSOAP_CLIENT_H
#define cSOAP_CLIENT_H

#include <libcsoap/soap-env.h>
#include <libcsoap/soap-ctx.h>

#define SOAP_ERROR_CLIENT_INIT 5001

#ifdef __cplusplus
extern "C" {
#endif

/**
	Initializes the client side soap engine
*/
herror_t soap_client_init_args(int argc, char *argv[]);


/**
	Destroy the soap client module
*/
void soap_client_destroy();


/**
   Establish connection to the soap server and send 
   the given envelope. 

   @param env envelope to send
   @param response  the result envelope
   @param url url to the soap server
   @soap_action value for "SoapAction:" in the 
    HTTP request header.

    @returns H_OK if success 
 */
herror_t soap_client_invoke(SoapCtx * ctx, SoapCtx ** response,
                            const char *url, const char *soap_action);



/**
	Sets the underlaying socket to use while connecting
	into block mode or not block mode.
	The default mode is always non-blocking mode.

  @param block 1 to creat blocked sockets, 0 to create non 
	blocking sockets.
*/
void soap_client_block_socket(int block);
int soap_client_get_blockmode();

#ifdef __cplusplus
}
#endif

#endif
