/******************************************************************
 *  $Id: nanohttp-client.h,v 1.24 2006/04/26 17:30:29 m0gg Exp $
 *
 * CSOAP Project:  A http client/server library in C
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
#ifndef NANO_HTTP_CLIENT_H
#define NANO_HTTP_CLIENT_H

#include <nanohttp/nanohttp-common.h>
#include <nanohttp/nanohttp-socket.h>
#include <nanohttp/nanohttp-response.h>
#include <nanohttp/nanohttp-stream.h>

typedef struct httpc_conn
{
  hsocket_t sock;
  hpair_t *header;
  hurl_t url;
  http_version_t version;
  /* 
     -1 : last dime package 0 : no dime connection >0 : dime package number */
  int _dime_package_nr;
  long _dime_sent_bytes;
  int errcode;
  char errmsg[150];
  http_output_stream_t *out;
  int id;                       /* uniq id */
} httpc_conn_t;


#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------
 HTTP CLIENT MODULE RELATED FUNCTIONS
 ---------------------------------------------------------------*/

/**
 *
 * Initializes the httpc_* module. This is called from
 * soap_client_init_args().
 *
 * @param argc		Argument count.
 * @param argv		Argument vector.
 *
 * @return H_OK on succes or a herror_t struct on failure.
 *
 * @see httpc_destroy, herror_t, soap_client_init_args
 */
herror_t httpc_init(int argc, char *argv[]);

/**
 *
 * Destroys the httpc_* module
 *
 * @see httpc_init
 *
 */
void httpc_destroy(void);

/**
 *
 * Creates a new connection.
 *
 */
httpc_conn_t *httpc_new(void);

/**
 *
 * Release a connection
 * (use httpc_close_free() instead)
 *
 * @see httpc_close_free
 *
 */
void httpc_free(httpc_conn_t * conn);

/**
 *
 * Close and release a connection
 *
 * @see httpc_close, httpc_free
 *
 */
void httpc_close_free(httpc_conn_t * conn);

/**
 *
 * Sets header element (key,value) pair.
 *
 * @return 0 on success or failure (yeah!), 1 if a (key,value) pair was replaced.
 *
 * @see httpc_add_header, httpc_add_headers
 *
 */
int httpc_set_header(httpc_conn_t * conn, const char *key, const char *value);

/**
 *
 * Adds a header element (key, value) pair.
 *
 * @return 0 on success, -1 on failure.
 *
 * @see httpc_set_header, httpc_add_headers
 *
 */
int httpc_add_header(httpc_conn_t *conn, const char *key, const char *value);

/**
 *
 * Adds a list of (key, value) pairs.
 *
 * @see httpc_set_header, httpc_add_header
 *
 */
void httpc_add_headers(httpc_conn_t *conn, const hpair_t *values);

/**
 *
 * Sets a HEADER_AUTHORIZATION header.
 *
 * @param conn		The HTTP connection.
 * @param user		The username.
 * @param password	The password.
 *
 * @see httpc_set_header, HEADER_AUTHORIZATION
 *
 */
int httpc_set_basic_authorization(httpc_conn_t *conn, const char *user, const char *password);

/**
 *
 * Sets a HEADER_PROXY_AUTHORIZATION header.
 *
 * @param conn		The HTTP connection.
 * @param user		The username.
 * @param password	The password.
 *
 * @see httpc_set_header, HEADER_PROXY_AUTHORIZATION
 */
int httpc_set_basic_proxy_authorization(httpc_conn_t *conn, const char *user, const char *password);

/**
  Invoke a "GET" method request and receive the response
*/
herror_t httpc_get(httpc_conn_t * conn, hresponse_t ** out, const char *urlstr);

/**
  Start a "POST" method request
  Returns: HSOCKET_OK  or error flag
*/
herror_t httpc_post_begin(httpc_conn_t * conn, const char *url);

/**
  End a "POST" method and receive the response.
  You MUST call httpc_post_end() before!
*/
herror_t httpc_post_end(httpc_conn_t * conn, hresponse_t ** out);


/* --------------------------------------------------------------
 DIME RELATED FUNCTIONS
 ---------------------------------------------------------------*/

/*
  DIME support httpc_dime_* function set
*/
/*
int httpc_dime_begin(httpc_conn_t *conn, const char *url);
int httpc_dime_next(httpc_conn_t* conn, long content_length, 
                    const char *content_type, const char *id,
                    const char *dime_options, int last);
hresponse_t* httpc_dime_end(httpc_conn_t *conn);
*/

/* --------------------------------------------------------------
 MIME RELATED FUNCTIONS
 ---------------------------------------------------------------*/
/*
  MIME support httpc_mime_* function set
*/

/**
  Begin MIME multipart/related POST request
  Returns: HSOCKET_OK  or error flag
*/
herror_t httpc_mime_begin(httpc_conn_t * conn, const char *url,
                          const char *related_start,
                          const char *related_start_info,
                          const char *related_type);

/**
  Send boundary and part header and continue 
  with next part
*/
herror_t httpc_mime_next(httpc_conn_t * conn,
                         const char *content_id,
                         const char *content_type,
                         const char *transfer_encoding);

/**
  Finish MIME request and get the response
*/
herror_t httpc_mime_end(httpc_conn_t * conn, hresponse_t ** out);

/**
  Send boundary and part header and continue 
  with next part
*/

herror_t httpc_mime_send_file(httpc_conn_t * conn,
                              const char *content_id,
                              const char *content_type,
                              const char *transfer_encoding,
                              const char *filename);

#ifdef __cplusplus
}
#endif

#endif
