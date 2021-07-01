/******************************************************************
*  $Id: soap-client.c,v 1.27 2006/07/09 16:24:19 snowdrop Exp $
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#include <nanohttp/nanohttp-logging.h>
#include <nanohttp/nanohttp-client.h>

#include "soap-client.h"

static herror_t
_soap_client_build_result(hresponse_t * res, SoapEnv ** env)
{
  log_verbose2("Building result (%p)", res);

  if (res == NULL)
    return herror_new("_soap_client_build_result",
                      GENERAL_INVALID_PARAM, "hresponse_t is NULL");


  if (res->in == NULL)
    return herror_new("_soap_client_build_result",
                      GENERAL_INVALID_PARAM, "Empty response from server");

  if (res->errcode != 200)
    return herror_new("_soap_client_build_result",
                      GENERAL_INVALID_PARAM, "HTTP code is not OK (%i)", res->errcode);

  return soap_env_new_from_stream(res->in, env);
}

herror_t
soap_client_init_args(int argc, char *argv[])
{

  return httpc_init(argc, argv);
}

void
soap_client_destroy(void)
{
  httpc_destroy();

  return;
}

herror_t
soap_client_invoke(SoapCtx * call, SoapCtx ** response, const char *url,
                   const char *soap_action)
{
  /* Status */
  herror_t status;

  /* Result document */
  SoapEnv *res_env;

  /* Buffer variables */
  xmlBufferPtr buffer;
  char *content;
  char tmp[15];

  /* Transport variables */
  httpc_conn_t *conn;
  hresponse_t *res;

  /* multipart/related start id */
  char start_id[150];
  static int counter = 1;
  part_t *part;

  /* for copy attachments */
  char href[MAX_HREF_SIZE];

  /* Create buffer */
  buffer = xmlBufferCreate();
  xmlNodeDump(buffer, call->env->root->doc, call->env->root, 1, 0);
  content = (char *) xmlBufferContent(buffer);

  /* Transport via HTTP */
  if (!(conn = httpc_new()))
  {
    return herror_new("soap_client_invoke", SOAP_ERROR_CLIENT_INIT,
                      "Unable to create SOAP client!");
  }

  /* Set soap action */
  if (soap_action != NULL)
    httpc_set_header(conn, "SoapAction", soap_action);

  httpc_set_header(conn, HEADER_CONNECTION, "Close");

  /* check for attachments */
  if (!call->attachments)
  {
    /* content-type is always 'text/xml' */
    httpc_set_header(conn, HEADER_CONTENT_TYPE, "text/xml");

    sprintf(tmp, "%d", (int) strlen(content));
    httpc_set_header(conn, HEADER_CONTENT_LENGTH, tmp);

    if ((status = httpc_post_begin(conn, url)) != H_OK)
    {
      httpc_close_free(conn);
      xmlBufferFree(buffer);
      return status;
    }

    if ((status = http_output_stream_write_string(conn->out, content)) != H_OK)
    {
      httpc_close_free(conn);
      xmlBufferFree(buffer);
      return status;
    }

    if ((status = httpc_post_end(conn, &res)) != H_OK)
    {
      httpc_close_free(conn);
      xmlBufferFree(buffer);
      return status;
    }
  }
  else
  {

    /* Use chunked transport */
    httpc_set_header(conn, HEADER_TRANSFER_ENCODING,
                     TRANSFER_ENCODING_CHUNKED);

    sprintf(start_id, "289247829121218%d", counter++);
    if ((status = httpc_mime_begin(conn, url, start_id, "", "text/xml")) != H_OK)
    {
      httpc_close_free(conn);
      xmlBufferFree(buffer);
      return status;
    }

    if ((status = httpc_mime_next(conn, start_id, "text/xml", "binary")) != H_OK)
    {
      httpc_close_free(conn);
      xmlBufferFree(buffer);
      return status;
    }

    if ((status = http_output_stream_write(conn->out, content, strlen(content))) != H_OK)
    {
      httpc_close_free(conn);
      xmlBufferFree(buffer);
      return status;
    }


    for (part = call->attachments->parts; part; part = part->next)
    {
      status = httpc_mime_send_file(conn, part->id,
                                    part->content_type,
                                    part->transfer_encoding, part->filename);
      if (status != H_OK)
      {
        log_error2("Send file failed. Status:%d", status);
        httpc_close_free(conn);
        xmlBufferFree(buffer);
        return status;
      }
    }

    if ((status = httpc_mime_end(conn, &res)) != H_OK)
    {
      httpc_close_free(conn);
      xmlBufferFree(buffer);
      return status;
    }
  }

  /* Free buffer */
  xmlBufferFree(buffer);

  /* Build result */
  if ((status = _soap_client_build_result(res, &res_env)) != H_OK)
  {
    hresponse_free(res);
    httpc_close_free(conn);
    return status;
  }

  /* Create Context */
  *response = soap_ctx_new(res_env);
/*	soap_ctx_add_files(*response, res->attachments);*/

  if (res->attachments != NULL)
  {
    part = res->attachments->parts;
    while (part)
    {
      soap_ctx_add_file(*response, part->filename, part->content_type, href);
      part->deleteOnExit = 0;
      part = part->next;
    }
    part = (*response)->attachments->parts;
    while (part)
    {
      part->deleteOnExit = 1;
      part = part->next;
    }
  }

  hresponse_free(res);
  httpc_close_free(conn);

  return H_OK;
}

