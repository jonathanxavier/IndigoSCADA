/******************************************************************
*  $Id: nanohttp-common.c,v 1.30 2006/07/09 16:24:19 snowdrop Exp $
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef MEM_DEBUG
#include <utils/alloc.h>
#endif

#include <stdarg.h>

#include "nanohttp-common.h"
#include "nanohttp-logging.h"

static int
strcmpigcase(const char *s1, const char *s2)
{
  int l1, l2, i;

  if (s1 == NULL && s2 == NULL)
    return 1;
  if (s1 == NULL || s2 == NULL)
    return 0;

  l1 = strlen(s1);
  l2 = strlen(s2);

  if (l1 != l2)
    return 0;

  for (i = 0; i < l1; i++)
    if (toupper(s1[i]) != toupper(s2[i]))
      return 0;

  return 1;
}

typedef struct _herror_impl_t
{
  int errcode;
  char message[250];
  char func[100];
} herror_impl_t;

herror_t
herror_new(const char *func, int errcode, const char *format, ...)
{
  va_list ap;

  herror_impl_t *impl = (herror_impl_t *) malloc(sizeof(herror_impl_t));
  impl->errcode = errcode;
  strcpy(impl->func, func);
  va_start(ap, format);
  vsprintf(impl->message, format, ap);
  va_end(ap);

  return (herror_t) impl;

}

int
herror_code(herror_t err)
{
  herror_impl_t *impl = (herror_impl_t *) err;
  if (!err)
    return H_OK;
  return impl->errcode;
}

char *
herror_func(herror_t err)
{
  herror_impl_t *impl = (herror_impl_t *) err;
  if (!err)
    return "";
  return impl->func;
}

char *
herror_message(herror_t err)
{
  herror_impl_t *impl = (herror_impl_t *) err;
  if (!err)
    return "";
  return impl->message;
}

void
herror_release(herror_t err)
{
  herror_impl_t *impl = (herror_impl_t *) err;
  if (!err)
    return;
  free(impl);
}


hpair_t *
hpairnode_new(const char *key, const char *value, hpair_t * next)
{
  hpair_t *pair;

  log_verbose3("new pair ('%s','%s')", SAVE_STR(key), SAVE_STR(value));
  pair = (hpair_t *) malloc(sizeof(hpair_t));

  if (key != NULL)
  {
    pair->key = (char *) malloc(strlen(key) + 1);
    strcpy(pair->key, key);
  }
  else
  {
    pair->key = NULL;
  }

  if (value != NULL)
  {
    pair->value = (char *) malloc(strlen(value) + 1);
    strcpy(pair->value, value);
  }
  else
  {
    pair->value = NULL;
  }

  pair->next = next;

  return pair;
}

hpair_t *
hpairnode_parse(const char *str, const char *delim, hpair_t * next)
{
  hpair_t *pair;
  char *key, *value;
  int c;

  pair = (hpair_t *) malloc(sizeof(hpair_t));
  pair->key = "";
  pair->value = "";
  pair->next = next;

  key = strtok_r((char *) str, delim, &value);

  if (key != NULL)
  {
    pair->key = (char *) malloc(strlen(key) + 1);
    strcpy(pair->key, key);
  }
  if (value != NULL)
  {
    for (c = 0; value[c] == ' '; c++);  /* skip white space */
    pair->value = (char *) malloc(strlen(&value[c]) + 1);
    strcpy(pair->value, &value[c]);
  }
  return pair;
}


hpair_t *
hpairnode_copy(const hpair_t * src)
{
  hpair_t *pair;

  if (src == NULL)
    return NULL;

  pair = hpairnode_new(src->key, src->value, NULL);
  return pair;
}


hpair_t *
hpairnode_copy_deep(const hpair_t * src)
{
  hpair_t *pair, *result, *next;

  if (src == NULL)
    return NULL;

  result = hpairnode_copy(src);

  next = src->next;
  pair = result;

  while (next != NULL)
  {
    pair->next = hpairnode_copy(next);
    pair = pair->next;
    next = next->next;
  }

  return result;
}


void
hpairnode_dump(hpair_t * pair)
{
  if (pair == NULL)
  {
    log_verbose1("(NULL)[]");
    return;
  }
  log_verbose5("(%p)['%s','%s','%p']", pair,
               SAVE_STR(pair->key), SAVE_STR(pair->value), pair->next);
}


void
hpairnode_dump_deep(hpair_t * pair)
{
  hpair_t *p;
  p = pair;

  log_verbose1("-- BEGIN dump hpairnode_t --");

  while (p != NULL)
  {
    hpairnode_dump(p);
    p = p->next;
  }

  log_verbose1("-- END dump hpairnode_t --\n");
}


void
hpairnode_free(hpair_t * pair)
{
  if (pair == NULL)
    return;

  free(pair->key);
  free(pair->value);

  free(pair);
}


void
hpairnode_free_deep(hpair_t * pair)
{
  hpair_t *tmp;

  while (pair != NULL)
  {
    tmp = pair->next;
    hpairnode_free(pair);
    pair = tmp;
  }
}

char *
hpairnode_get_ignore_case(hpair_t * pair, const char *key)
{
  if (key == NULL)
  {
    log_error1("key is NULL");
    return NULL;
  }
  while (pair != NULL)
  {
    if (pair->key != NULL)
    {
      if (strcmpigcase(pair->key, key))
      {
        return pair->value;
      }
    }
    pair = pair->next;
  }

  return NULL;
}

char *
hpairnode_get(hpair_t * pair, const char *key)
{
  if (key == NULL)
  {
    log_error1("key is NULL");
    return NULL;
  }
  while (pair != NULL)
  {
    if (pair->key != NULL)
    {
      if (!strcmp(pair->key, key))
      {
        return pair->value;
      }
    }
    pair = pair->next;
  }

  return NULL;
}

static void
hurl_dump(const hurl_t * url)
{

  if (url == NULL)
  {
    log_error1("url is NULL!");
    return;
  }
  log_verbose2("PROTOCOL : %d", url->protocol);
  log_verbose2("    HOST : %s", url->host);
  log_verbose2("    PORT : %d", url->port);
  log_verbose2(" CONTEXT : %s", url->context);
}

herror_t
hurl_parse(hurl_t * url, const char *urlstr)
{
  int iprotocol;
  int ihost;
  int iport;
  int len;
  int size;
  char tmp[8];
  char protocol[1024];

  iprotocol = 0;
  len = strlen(urlstr);

  /* find protocol */
  while (urlstr[iprotocol] != ':' && urlstr[iprotocol] != '\0')
  {
    iprotocol++;
  }

  if (iprotocol == 0)
  {
    log_error1("no protocol");
    return herror_new("hurl_parse", URL_ERROR_NO_PROTOCOL, "No protocol");
  }
  if (iprotocol + 3 >= len)
  {
    log_error1("no host");
    return herror_new("hurl_parse", URL_ERROR_NO_HOST, "No host");
  }
  if (urlstr[iprotocol] != ':'
      && urlstr[iprotocol + 1] != '/' && urlstr[iprotocol + 2] != '/')
  {
    log_error1("no protocol");
    return herror_new("hurl_parse", URL_ERROR_NO_PROTOCOL, "No protocol");
  }
  /* find host */
  ihost = iprotocol + 3;
  while (urlstr[ihost] != ':'
         && urlstr[ihost] != '/' && urlstr[ihost] != '\0')
  {
    ihost++;
  }

  if (ihost == iprotocol + 1)
  {
    log_error1("no host");
    return herror_new("hurl_parse", URL_ERROR_NO_HOST, "No host");
  }
  /* find port */
  iport = ihost;
  if (ihost + 1 < len)
  {
    if (urlstr[ihost] == ':')
    {
      while (urlstr[iport] != '/' && urlstr[iport] != '\0')
      {
        iport++;
      }
    }
  }

  /* find protocol */
  strncpy(protocol, urlstr, iprotocol);
  protocol[iprotocol] = '\0';
  if (strcmpigcase(protocol, "http"))
    url->protocol = PROTOCOL_HTTP;
  else if (strcmpigcase(protocol, "https"))
    url->protocol = PROTOCOL_HTTPS;
  else if (strcmpigcase(protocol, "ftp"))
    url->protocol = PROTOCOL_FTP;
  else
    return herror_new("hurl_parse",
                      URL_ERROR_UNKNOWN_PROTOCOL, "Unknown protocol '%s'",
                      protocol);

  /* TODO (#1#): add max of size and URL_MAX_HOST_SIZE */
  size = ihost - iprotocol - 3;
  strncpy(url->host, &urlstr[iprotocol + 3], size);
  url->host[size] = '\0';

  if (iport > ihost)
  {
    size = iport - ihost;
    strncpy(tmp, &urlstr[ihost + 1], size);
    url->port = atoi(tmp);
  }
  else
  {
    switch (url->protocol)
    {
    case PROTOCOL_HTTP:
      url->port = URL_DEFAULT_PORT_HTTP;
      break;
    case PROTOCOL_HTTPS:
      url->port = URL_DEFAULT_PORT_HTTPS;
      break;
    case PROTOCOL_FTP:
      url->port = URL_DEFAULT_PORT_FTP;
      break;
    }
  }

  len = strlen(urlstr);
  if (len > iport)
  {
    /* TODO (#1#): find max of size and URL_MAX_CONTEXT_SIZE */
    size = len - iport;
    strncpy(url->context, &urlstr[iport], size);
    url->context[size] = '\0';
  }
  else
  {
    url->context[0] = '\0';
  }

  hurl_dump(url);

  return H_OK;
}


/* Content-type stuff */

content_type_t *
content_type_new(const char *content_type_str)
{
  hpair_t *pair = NULL, *last = NULL;
  content_type_t *ct;
  char ch, key[256], value[256];
  int inQuote = 0, i = 0, c = 0, begin = 0, len;
  int mode = 0;
  /* 0: searching ';' 1: process key 2: process value */


  /* Create object */
  ct = (content_type_t *) malloc(sizeof(content_type_t));
  ct->params = NULL;

  len = strlen(content_type_str);
  while (i <= len)
  {
    if (i != len)
      ch = content_type_str[i++];
    else
    {
      ch = ' ';
      i++;
    }

    switch (mode)
    {
    case 0:

      if (ch == ';')
      {
        ct->type[c] = '\0';
        c = 0;
        mode = 1;
      }
      else if (ch != ' ' && ch != '\t' && ch != '\r')
        ct->type[c++] = ch;
      break;

    case 1:

      if (ch == '=')
      {
        key[c] = '\0';
        c = 0;
        mode = 2;
      }
      else if (ch != ' ' && ch != '\t' && ch != '\r')
        key[c++] = ch;
      break;

    case 2:

      if (ch != ' ')
        begin = 1;

      if ((ch == ' ' || ch == ';') && !inQuote && begin)
      {
        value[c] = '\0';

        pair = hpairnode_new(key, value, NULL);
        if (ct->params == NULL)
          ct->params = pair;
        else
          last->next = pair;
        last = pair;

        c = 0;
        begin = 0;
        mode = 1;
      }
      else if (ch == '"')
        inQuote = !inQuote;
      else if (begin && ch != '\r')
        value[c++] = ch;

      break;

    }
  }

  return ct;
}


void
content_type_free(content_type_t * ct)
{
  if (!ct)
    return;

  hpairnode_free_deep(ct->params);
  free(ct);
}


part_t *
part_new(const char *id, const char *filename,
         const char *content_type, const char *transfer_encoding,
         part_t * next)
{
  part_t *part = (part_t *) malloc(sizeof(part_t));
  part->header = NULL;
  part->next = next;
  part->deleteOnExit = 0;
  strcpy(part->id, id);
  strcpy(part->filename, filename);
  if (content_type)
    strcpy(part->content_type, content_type);
  else
    part->content_type[0] = '\0';

  part->header = hpairnode_new(HEADER_CONTENT_ID, id, part->header);
  /* TODO (#1#): encoding is always binary. implement also others! */
/*  part->header = hpairnode_new(HEADER_CONTENT_TRANSFER_ENCODING, "binary", part->header);*/

  strcpy(part->transfer_encoding,
         transfer_encoding ? transfer_encoding : "binary");

  if (content_type)
  {
    part->header =
      hpairnode_new(HEADER_CONTENT_TYPE, content_type, part->header);
  }
  else
  {
    /* TODO (#1#): get content-type from mime type list */
  }

  return part;
}

void
part_free(part_t * part)
{
  if (part == NULL)
    return;

  if (part->deleteOnExit)
  {
    remove(part->filename);
  }

  hpairnode_free_deep(part->header);

  free(part);
}

attachments_t *
attachments_new()               /* should be used internally */
{
  attachments_t *attachments =
    (attachments_t *) malloc(sizeof(attachments_t));
  attachments->parts = NULL;
  attachments->last = NULL;
  attachments->root_part = NULL;

  return attachments;
}

void
attachments_add_part(attachments_t * attachments, part_t * part)
{
  /* paranoya check */
  if (!attachments)
    return;

  if (attachments->last)
    attachments->last->next = part;
  else
    attachments->parts = part;

  attachments->last = part;
}

/*
  Free a mime message 
*/
void
attachments_free(attachments_t * message)
{
  part_t *tmp, *part;

  if (message == NULL)
    return;

  part = message->parts;
  while (part)
  {
    tmp = part->next;
    part_free(part);
    part = tmp;
  }

  if (message->root_part)
    part_free(message->root_part);
/* TODO (#1#): HERE IS A BUG!!!! */
  free(message);
}


#ifdef WIN32

/* strtok_r() */
char *
strtok_r(char *s, const char *delim, char **save_ptr)
{
  char *token;

  if (s == NULL)
    s = *save_ptr;

  /* Scan leading delimiters.  */
  s += strspn(s, delim);
  if (*s == '\0')
    return NULL;

  /* Find the end of the token.  */
  token = s;
  s = strpbrk(token, delim);
  if (s == NULL)
    /* This token finishes the string.  */
    *save_ptr = strchr(token, '\0');
  else
  {
    /* Terminate the token and make *SAVE_PTR point past it.  */
    *s = '\0';
    *save_ptr = s + 1;
  }
  return token;
}

/* localtime_r() */
struct tm *
localtime_r(const time_t * const timep, struct tm *p_tm)
{
  static struct tm *tmp;
  tmp = localtime(timep);
  if (tmp)
  {
    memcpy(p_tm, tmp, sizeof(struct tm));
    tmp = p_tm;
  }
  return tmp;
}

#endif
