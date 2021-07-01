/******************************************************************
 *  $Id: nanohttp-server.h,v 1.21 2006/05/31 19:39:34 mrcsys Exp $
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
#ifndef NANO_HTTP_SERVER_H
#define NANO_HTTP_SERVER_H


#include <nanohttp/nanohttp-common.h>
#include <nanohttp/nanohttp-socket.h>
#include <nanohttp/nanohttp-request.h>
#include <nanohttp/nanohttp-stream.h>



typedef struct httpd_conn
{
  hsocket_t *sock;
  char content_type[25];
  http_output_stream_t *out;
  hpair_t *header;
}
httpd_conn_t;


/*
  Service callback
 */
typedef void (*httpd_service) (httpd_conn_t *, hrequest_t *);
typedef int (*httpd_auth) (hrequest_t * req, const char *user,
                           const char *password);

/*
 * Service representation object
 */
typedef struct tag_hservice
{
  char ctx[255];
  httpd_service func;
  httpd_auth auth;
  struct tag_hservice *next;
}
hservice_t;


#ifdef __cplusplus
extern "C"
{
#endif


/*
  Begin  httpd_* function set
 */
  herror_t httpd_init(int argc, char *argv[]);
  void httpd_destroy(void);

  herror_t httpd_run(void);

  int httpd_register(const char *ctx, httpd_service service);
  int httpd_register_secure(const char *ctx, httpd_service service,
                            httpd_auth auth);

  int httpd_register_default(const char *ctx, httpd_service service);
  int httpd_register_default_secure(const char *ctx, httpd_service service,
                                    httpd_auth auth);

  int httpd_get_port(void);
  int httpd_get_timeout(void);
  void httpd_set_timeout(int t);

  const char *httpd_get_protocol(void);
  int httpd_get_conncount(void);

  hservice_t *httpd_services(void);

  herror_t httpd_send_header(httpd_conn_t * res, int code, const char *text);

  int httpd_set_header(httpd_conn_t * conn, const char *key,
                       const char *value);
  void httpd_set_headers(httpd_conn_t * conn, hpair_t * header);

  int httpd_add_header(httpd_conn_t * conn, const char *key,
                       const char *value);
  void httpd_add_headers(httpd_conn_t * conn, const hpair_t * values);

/*
unsigned char *httpd_get_postdata(httpd_conn_t *conn, 
			 hrequest_t *req, long *received, long max);
*/
/* --------------------------------------------------------------
 MIME RELATED FUNCTIONS
 ---------------------------------------------------------------*/
/*
  MIME support httpd_mime_* function set
*/

/**
  Begin MIME multipart/related POST 
  Returns: HSOCKET_OK  or error flag
*/
  herror_t httpd_mime_send_header(httpd_conn_t * conn,
                                  const char *related_start,
                                  const char *related_start_info,
                                  const char *related_type, int code,
                                  const char *text);

/**
  Send boundary and part header and continue 
  with next part
*/
  herror_t httpd_mime_next(httpd_conn_t * conn,
                           const char *content_id,
                           const char *content_type,
                           const char *transfer_encoding);

/**
  Send boundary and part header and continue 
  with next part
*/
  herror_t httpd_mime_send_file(httpd_conn_t * conn,
                                const char *content_id,
                                const char *content_type,
                                const char *transfer_encoding,
                                const char *filename);

/**
  Finish MIME request 
  Returns: HSOCKET_OK  or error flag
*/
  herror_t httpd_mime_end(httpd_conn_t * conn);


#ifdef __cplusplus
}
#endif

#endif
