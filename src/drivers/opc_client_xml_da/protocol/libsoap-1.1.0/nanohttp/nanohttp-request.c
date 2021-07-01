/******************************************************************
*  $Id: nanohttp-request.c,v 1.14 2006/07/09 16:24:19 snowdrop Exp $
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
* Email: ayaz@jprogrammer.net
******************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef MEM_DEBUG
#include <utils/alloc.h>
#endif

#include "nanohttp-logging.h"
#include "nanohttp-common.h"
#include "nanohttp-request.h"

static hrequest_t *
hrequest_new(void)
{
  hrequest_t *req;
 
  if (!(req = (hrequest_t *) malloc(sizeof(hrequest_t)))) {

	  log_error2("malloc failed (%s)", strerror(errno));
	  return NULL;
  }

  req->method = HTTP_REQUEST_GET;
  req->version = HTTP_1_1;
  req->query = NULL;
  req->header = NULL;
  req->in = NULL;
  req->attachments = NULL;
  req->content_type = NULL;

  return req;
}

static hrequest_t *
_hrequest_parse_header(char *data)
{
  hrequest_t *req;
  hpair_t *hpair = NULL, *qpair = NULL, *tmppair = NULL;

  char *tmp;
  char *tmp2;
  char *saveptr;
  char *saveptr2;
  char *saveptr3;
  char *result;
  char *key;
  char *opt_key;
  char *opt_value;
  int firstline = 1;

  req = hrequest_new();
  tmp = data;

  for (;;)
  {
    result = (char *) strtok_r(tmp, "\r\n", &saveptr);
    tmp = saveptr;

    if (result == NULL)
      break;

    if (firstline)
    {
      firstline = 0;
      tmp2 = result;

      /* parse [GET|POST] [PATH] [SPEC] */
      key = (char *) strtok_r(tmp2, " ", &saveptr2);

      /* save method (get or post) */
      tmp2 = saveptr2;
      if (key != NULL)
      {
        if (!strcmp(key, "POST"))
          req->method = HTTP_REQUEST_POST;
        else if (!strcmp(key, "GET"))
          req->method = HTTP_REQUEST_GET;
	else if (!strcmp(key, "OPTIONS"))
          req->method = HTTP_REQUEST_OPTIONS;
	else if (!strcmp(key, "HEAD"))
          req->method = HTTP_REQUEST_HEAD;
        else if (!strcmp(key, "PUT"))
          req->method = HTTP_REQUEST_PUT;
        else if (!strcmp(key, "DELETE"))
          req->method = HTTP_REQUEST_DELETE;
        else if (!strcmp(key, "TRACE"))
          req->method = HTTP_REQUEST_TRACE;
        else if (!strcmp(key, "CONNECT"))
          req->method = HTTP_REQUEST_CONNECT;
        else
          req->method = HTTP_REQUEST_UNKOWN;
      }
      /* below is key the path and tmp2 the spec */
      key = (char *) strtok_r(tmp2, " ", &saveptr2);

      /* save version */
      tmp2 = saveptr2;
      if (tmp2 != NULL)
      {
        /* req->spec = (char *) malloc(strlen(tmp2) + 1); strcpy(req->spec,
           tmp2); */
        if (!strcmp(tmp2, "HTTP/1.0"))
          req->version = HTTP_1_0;
        else
          req->version = HTTP_1_1;
      }
      /* 
       * parse and save path+query parse:
       * /path/of/target?key1=value1&key2=value2...
       */

      if (key != NULL)
      {
        tmp2 = key;
        key = (char *) strtok_r(tmp2, "?", &saveptr2);
        tmp2 = saveptr2;

        /* save path */
        /* req->path = (char *) malloc(strlen(key) + 1); */
        strncpy(req->path, key, REQUEST_MAX_PATH_SIZE);

        /* parse options */
        for (;;)
        {
          key = (char *) strtok_r(tmp2, "&", &saveptr2);
          tmp2 = saveptr2;

          if (key == NULL)
            break;

          opt_key = (char *) strtok_r(key, "=", &saveptr3);
          opt_value = saveptr3;

          if (opt_value == NULL)
            opt_value = "";

          /* create option pair */
          if (opt_key != NULL)
          {
            if (!(tmppair = (hpair_t *) malloc(sizeof(hpair_t))))
            {
              log_error2("malloc failed (%s)", strerror(errno));
              return NULL;
            }

            if (req->query == NULL)
            {
              req->query = qpair = tmppair;
            }
            else
            {
              qpair->next = tmppair;
              qpair = tmppair;
            }

            /* fill hpairnode_t struct */
            qpair->next = NULL;
            qpair->key = strdup(opt_key);
            qpair->value = strdup(opt_value);

          }
        }
      }
    }
    else
    {

      /* parse "key: value" */
      /* tmp2 = result; key = (char *) strtok_r(tmp2, ": ", &saveptr2); value 
         = saveptr2; */

      /* create pair */
/*			tmppair = (hpair_t *) malloc(sizeof(hpair_t));*/
      tmppair = hpairnode_parse(result, ":", NULL);

      if (req->header == NULL)
      {
        req->header = hpair = tmppair;
      }
      else
      {
        hpair->next = tmppair;
        hpair = tmppair;
      }

      /* fill pairnode_t struct */
      /* 
         hpair->next = NULL; hpair->key = (char *) malloc(strlen(key) + 1);
         hpair->value = (char *) malloc(strlen(value) + 1);

         strcpy(hpair->key, key); strcpy(hpair->value, value); */
    }
  }

  /* Check Content-type */
  tmp = hpairnode_get_ignore_case(req->header, HEADER_CONTENT_TYPE);
  if (tmp != NULL)
    req->content_type = content_type_new(tmp);

  return req;
}


void
hrequest_free(hrequest_t * req)
{
  if (req == NULL)
    return;

  hpairnode_free_deep(req->header);
  hpairnode_free_deep(req->query);

  if (req->in)
    http_input_stream_free(req->in);

  if (req->content_type)
    content_type_free(req->content_type);

  if (req->attachments)
    attachments_free(req->attachments);

  free(req);

  return;
}


herror_t
hrequest_new_from_socket(hsocket_t *sock, hrequest_t ** out)
{
  int i, readed;
  herror_t status;
  hrequest_t *req;
  char buffer[MAX_HEADER_SIZE + 1];
  attachments_t *mimeMessage;

  memset(buffer, 0, MAX_HEADER_SIZE);
  /* Read header */
  for(i=0; i < MAX_HEADER_SIZE; i++)
  {
    if ((status = hsocket_read(sock, &(buffer[i]), 1, 1, &readed)) != H_OK)
    {
      log_error2("hsocket_read failed (%s)", herror_message(status));
      return status;
    }

    buffer[i + 1] = '\0';       /* for strmp */

/*    log_error2("buffer=\"%s\"", buffer); */

    if (i > 3)
    {
      if (!strcmp(&(buffer[i - 1]), "\n\n") ||
          !strcmp(&(buffer[i - 2]), "\n\r\n"))
        break;
    }
  }

  /* Create response */
  req = _hrequest_parse_header(buffer);

  /* Create input stream */
  req->in = http_input_stream_new(sock, req->header);

  /* Check for MIME message */
  if ((req->content_type &&
       !strcmp(req->content_type->type, "multipart/related")))
  {
    status = mime_get_attachments(req->content_type, req->in, &mimeMessage);
    if (status != H_OK)
    {
      /* TODO (#1#): Handle error */
      hrequest_free(req);
      return status;
    }
    else
    {
      req->attachments = mimeMessage;
      req->in =
        http_input_stream_new_from_file(mimeMessage->root_part->filename);
    }
  }

  *out = req;
  return H_OK;
}
