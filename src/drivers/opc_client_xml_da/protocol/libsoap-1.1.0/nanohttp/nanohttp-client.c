/******************************************************************
*  $Id: nanohttp-client.c,v 1.41 2006/07/09 16:24:19 snowdrop Exp $
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef MEM_DEBUG
#include <utils/alloc.h>
#endif

#include "nanohttp-client.h"
#include "nanohttp-socket.h"
#include "nanohttp-base64.h"
#include "nanohttp-logging.h"

/*--------------------------------------------------
FUNCTION: httpc_init
DESC: Initialize http client connection
NOTE: This will be called from soap_client_init_args()
----------------------------------------------------*/
herror_t
httpc_init(int argc, char **argv)
{
  return hsocket_module_init(argc, argv);
}

/*--------------------------------------------------
FUNCTION: httpc_destroy
DESC: Destroy the http client module
----------------------------------------------------*/
void
httpc_destroy(void)
{
  hsocket_module_destroy();

  return;
}

/*--------------------------------------------------
FUNCTION: httpc_new
DESC: Creates a new http client connection object
You need to create at least 1 http client connection
to communicate via http.
----------------------------------------------------*/
httpc_conn_t *
httpc_new(void)
{
  static int counter = 10000;
  herror_t status;
  httpc_conn_t *res;
 
  if (!(res = (httpc_conn_t *) malloc(sizeof(httpc_conn_t))))
    return NULL;

  if ((status = hsocket_init(&res->sock)) != H_OK)
  {
    log_warn2("hsocket_init failed (%s)", herror_message(status));
    return NULL;
  }

  res->header = NULL;
  res->version = HTTP_1_1;
  res->out = NULL;
  res->_dime_package_nr = 0;
  res->_dime_sent_bytes = 0;
  res->id = counter++;

  return res;
}

/*--------------------------------------------------
FUNCTION: httpc_free
DESC: Free the given http client object.
----------------------------------------------------*/
void
httpc_free(httpc_conn_t * conn)
{
  hpair_t *tmp;

  if (conn == NULL)
    return;


  while (conn->header != NULL)
  {
    tmp = conn->header;
    conn->header = conn->header->next;
    hpairnode_free(tmp);
  }

  if (conn->out != NULL)
  {
    http_output_stream_free(conn->out);
    conn->out = NULL;
  }

  hsocket_free(&(conn->sock));
  free(conn);

  return;
}

/*--------------------------------------------------
 FUNCTION: httpc_close_free
 DESC: Close and free the given http client object.
 ----------------------------------------------------*/
void
httpc_close_free(httpc_conn_t * conn)
{
  if (conn == NULL)
    return;

  hsocket_close(&(conn->sock));
  httpc_free(conn);

  return;
}

int
httpc_add_header(httpc_conn_t *conn, const char *key, const char *value)
{
  if (!conn)
  {
    log_warn1("Connection object is NULL");
    return -1;
  }

  conn->header = hpairnode_new(key, value, conn->header);

  return 0;
}

void
httpc_add_headers(httpc_conn_t *conn, const hpair_t *values)
{
  if (conn == NULL)
  {
    log_warn1("Connection object is NULL");
    return;
  }

  for ( ;values; values=values->next)
    httpc_add_header(conn, values->key, values->value);

  return;
}
 
/*--------------------------------------------------
FUNCTION: httpc_set_header
DESC: Adds a new (key, value) pair to the header
or modifies the old pair if this function will
finds another pair with the same 'key' value.
----------------------------------------------------*/
int
httpc_set_header(httpc_conn_t *conn, const char *key, const char *value)
{
  hpair_t *p;

  if (conn == NULL)
  {
    log_warn1("Connection object is NULL");
    return 0;
  }

  for (p = conn->header; p; p = p->next)
  {
    if (p->key && !strcmp(p->key, key))
    {
      free(p->value);
      p->value = strdup(value);
      return 1;
    }
  }

  conn->header = hpairnode_new(key, value, conn->header);

  return 0;
}

static int
_httpc_set_basic_authorization_header(httpc_conn_t *conn, const char *key, const char *user, const char *password)
{
  /* XXX: use malloc/free */
  char in[64], out[64];

  if (!user)
    user = "";

  if (!password)
    password = "";

  memset(in, 0, 64);
  memset(out, 0, 64);

  sprintf(in, "%s:%s", user, password);

  base64_encode(in, out);

  sprintf(in, "Basic %s", out);

  return httpc_set_header(conn, key, in);
}

int
httpc_set_basic_authorization(httpc_conn_t *conn, const char *user, const char *password)
{
  return _httpc_set_basic_authorization_header(conn, HEADER_AUTHORIZATION, user, password);
}

int
httpc_set_basic_proxy_authorization(httpc_conn_t *conn, const char *user, const char *password)
{
  return _httpc_set_basic_authorization_header(conn, HEADER_PROXY_AUTHORIZATION, user, password);
}

/*--------------------------------------------------
FUNCTION: httpc_header_set_date
DESC: Adds the current date to the header.
----------------------------------------------------*/
static void
httpc_header_set_date(httpc_conn_t * conn)
{
  char buffer[32];
  time_t ts;
  struct tm stm;

  ts = time(NULL);
  localtime_r(&ts, &stm);
  strftime(buffer, 32, "%a, %d %b %Y %H:%M:%S GMT", &stm);

  httpc_set_header(conn, HEADER_DATE, buffer);

  return;
}


/*--------------------------------------------------
FUNCTION: httpc_send_header
DESC: Sends the current header information stored
in conn through conn->sock.
----------------------------------------------------*/
herror_t
httpc_send_header(httpc_conn_t * conn)
{
  hpair_t *walker;
  herror_t status;
  char buffer[1024];

  for (walker = conn->header; walker; walker = walker->next)
  {
    if (walker->key && walker->value)
    {
      sprintf(buffer, "%s: %s\r\n", walker->key, walker->value);
      if ((status = hsocket_send(&(conn->sock), buffer)) != H_OK)
        return status;
    }
  }

  return hsocket_send(&(conn->sock), "\r\n");
}

/*--------------------------------------------------
FUNCTION: httpc_talk_to_server
DESC: This function is the heart of the httpc
module. It will send the request and process the
response.

Here the parameters:

method:
the request method. This can be HTTP_REQUEST_POST and
HTTP_REQUEST_GET.

conn:
the connection object (created with httpc_new())

urlstr:
the complete url in string format.
http://<host>:<port>/<context>
where <port> is not mendatory.

start_cb:
a callback function, which will be called when
the response header is completely arrives.

cb:
a callback function, which will be called everytime
when data arrives.

content_size:
size of content to send.
(only if method is HTTP_REQUEST_POST)

content:
the content data to send.
(only if method is HTTP_REQUEST_POST)

userdata:
a user define data, which will be passed to the
start_cb and cb callbacks as a parameter. This
can also be NULL.


If success, this function will return 0.
>0 otherwise.
----------------------------------------------------*/
static herror_t
httpc_talk_to_server(hreq_method_t method, httpc_conn_t * conn,
                     const char *urlstr)
{

  hurl_t url;
  char buffer[4096];
  herror_t status;
  int ssl;

  if (conn == NULL)
  {
    return herror_new("httpc_talk_to_server",
                      GENERAL_INVALID_PARAM, "httpc_conn_t param is NULL");
  }
  /* Build request header */
  httpc_header_set_date(conn);

  if ((status = hurl_parse(&url, urlstr)) != H_OK)
  {
    log_error2("Can not parse URL '%s'", SAVE_STR(urlstr));
    return status;
  }
/* TODO (#1#): Check for HTTP protocol in URL */

  /* Set hostname */
  httpc_set_header(conn, HEADER_HOST, url.host);

  ssl = url.protocol == PROTOCOL_HTTPS ? 1 : 0;

  /* Open connection */
  if ((status = hsocket_open(&conn->sock, url.host, url.port, ssl)) != H_OK)
    return status;

  switch(method)
  {
    case HTTP_REQUEST_GET:

      sprintf(buffer, "GET %s HTTP/%s\r\n",
        (url.context[0] != '\0') ? url.context : ("/"),
        (conn->version == HTTP_1_0) ? "1.0" : "1.1");
      break;

    case HTTP_REQUEST_POST:

      sprintf(buffer, "POST %s HTTP/%s\r\n",
        (url.context[0] != '\0') ? url.context : ("/"),
        (conn->version == HTTP_1_0) ? "1.0" : "1.1");
      break;

    default:
      log_error1("Unknown method type!");
      return herror_new("httpc_talk_to_server",
        GENERAL_INVALID_PARAM,
        "hreq_method_t must be  HTTP_REQUEST_GET or HTTP_REQUEST_POST");
  }

  log_verbose1("Sending request...");
  if ((status = hsocket_send(&(conn->sock), buffer)) != H_OK)
  {
    log_error2("Cannot send request (%s)", herror_message(status));
    hsocket_close(&(conn->sock));
    return status;
  }

  log_verbose1("Sending header...");
  if ((status = httpc_send_header(conn)) != H_OK)
  {
    log_error2("Cannot send header (%s)", herror_message(status));
    hsocket_close(&(conn->sock));
    return status;
  }

  return H_OK;
}

/*--------------------------------------------------
FUNCTION: httpc_get
DESC:
----------------------------------------------------*/
herror_t
httpc_get(httpc_conn_t * conn, hresponse_t ** out, const char *urlstr)
{
  herror_t status;

  if ((status = httpc_talk_to_server(HTTP_REQUEST_GET, conn, urlstr)) != H_OK)
    return status;

  if ((status = hresponse_new_from_socket(&(conn->sock), out)) != H_OK)
    return status;

  return H_OK;
}


/*--------------------------------------------------
FUNCTION: httpc_post_begin
DESC: Returns H_OK if success
----------------------------------------------------*/
herror_t
httpc_post_begin(httpc_conn_t * conn, const char *url)
{
  herror_t status;

  if ((status = httpc_talk_to_server(HTTP_REQUEST_POST, conn, url)) != H_OK)
    return status;

  conn->out = http_output_stream_new(&(conn->sock), conn->header);

  return H_OK;
}


/*--------------------------------------------------
FUNCTION: httpc_post_begin
DESC: End a "POST" method and receive the response.
  You MUST call httpc_post_end() before!
----------------------------------------------------*/
herror_t
httpc_post_end(httpc_conn_t * conn, hresponse_t ** out)
{
  herror_t status;

  if ((status = http_output_stream_flush(conn->out)) != H_OK)
    return status;

  if ((status = hresponse_new_from_socket(&(conn->sock), out)) != H_OK)
    return status;

  return H_OK;
}

/* ---------------------------------------------------
  MIME support functions httpc_mime_* function set
-----------------------------------------------------*/

static void
_httpc_mime_get_boundary(httpc_conn_t * conn, char *dest)
{
  sprintf(dest, "---=.Part_NH_%d", conn->id);
  log_verbose2("boundary= \"%s\"", dest);
}

herror_t
httpc_mime_begin(httpc_conn_t * conn, const char *url,
                 const char *related_start,
                 const char *related_start_info, const char *related_type)
{
  herror_t status;
  char buffer[300];
  char temp[75];
  char boundary[75];

  /* 
     Set Content-type Set multipart/related parameter type=..; start=.. ;
     start-info= ..; boundary=...

   */
  sprintf(buffer, "multipart/related;");
  /* 
     using sprintf instead of snprintf because visual c does not support
     snprintf */
#ifdef WIN32
#define snprintf(buffer, num, s1, s2) sprintf(buffer, s1,s2)
#endif

  if (related_type)
  {
    snprintf(temp, 75, " type=\"%s\";", related_type);
    strcat(buffer, temp);
  }

  if (related_start)
  {
    snprintf(temp, 75, " start=\"%s\";", related_start);
    strcat(buffer, temp);
  }

  if (related_start_info)
  {
    snprintf(temp, 75, " start-info=\"%s\";", related_start_info);
    strcat(buffer, temp);
  }

  _httpc_mime_get_boundary(conn, boundary);
  snprintf(temp, 75, " boundary=\"%s\"", boundary);
  strcat(buffer, temp);

  httpc_set_header(conn, HEADER_CONTENT_TYPE, buffer);

  status = httpc_post_begin(conn, url);
  return status;
}

herror_t
httpc_mime_next(httpc_conn_t * conn,
                const char *content_id,
                const char *content_type, const char *transfer_encoding)
{
  herror_t status;
  char buffer[512];
  char boundary[75];

  /* Get the boundary string */
  _httpc_mime_get_boundary(conn, boundary);
  sprintf(buffer, "\r\n--%s\r\n", boundary);

  /* Send boundary */
  status = http_output_stream_write(conn->out,
                                    (const byte_t *) buffer, strlen(buffer));

  if (status != H_OK)
    return status;

  /* Send Content header */
  sprintf(buffer, "%s: %s\r\n%s: %s\r\n%s: %s\r\n\r\n",
          HEADER_CONTENT_TYPE, content_type,
          HEADER_CONTENT_TRANSFER_ENCODING, transfer_encoding,
          HEADER_CONTENT_ID, content_id);

  return http_output_stream_write(conn->out,
                                    (const byte_t *) buffer, strlen(buffer));
}


herror_t
httpc_mime_end(httpc_conn_t * conn, hresponse_t ** out)
{
  herror_t status;
  char buffer[512];
  char boundary[75];

  /* Get the boundary string */
  _httpc_mime_get_boundary(conn, boundary);
  sprintf(buffer, "\r\n--%s--\r\n\r\n", boundary);

  /* Send boundary */
  status = http_output_stream_write(conn->out,
                                    (const byte_t *) buffer, strlen(buffer));

  if (status != H_OK)
    return status;

  if ((status = http_output_stream_flush(conn->out)) != H_OK)
    return status;

  if ((status = hresponse_new_from_socket(&(conn->sock), out)) != H_OK)
    return status;

  return H_OK;
}


/**
  Send boundary and part header and continue 
  with next part
*/
herror_t
httpc_mime_send_file(httpc_conn_t * conn,
                     const char *content_id,
                     const char *content_type,
                     const char *transfer_encoding, const char *filename)
{
  herror_t status;
  FILE *fd = fopen(filename, "rb");
  byte_t buffer[MAX_FILE_BUFFER_SIZE];
  size_t size;

  if (fd == NULL)
    return herror_new("httpc_mime_send_file", FILE_ERROR_OPEN,
                      "Can not open file '%s'", filename);

  status = httpc_mime_next(conn, content_id, content_type, transfer_encoding);
  if (status != H_OK)
  {
    fclose(fd);
    return status;
  }

  while (!feof(fd))
  {
    size = fread(buffer, 1, MAX_FILE_BUFFER_SIZE, fd);
    if (size == -1)
    {
      fclose(fd);
      return herror_new("httpc_mime_send_file", FILE_ERROR_READ,
                        "Can not read from file '%s'", filename);
    }

    if (size > 0)
    {
      /* DEBUG: fwrite(buffer, 1, size, stdout); */
      status = http_output_stream_write(conn->out, buffer, size);
      if (status != H_OK)
      {
        fclose(fd);
        return status;
      }
    }
  }

  fclose(fd);
  log_verbose1("file sent!");
  return H_OK;
}
