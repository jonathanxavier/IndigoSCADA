/******************************************************************
*  $Id: nanohttp-stream.c,v 1.13 2006/07/09 16:24:19 snowdrop Exp $
*
* CSOAP Project:  A http client/server library in C
* Copyright (C) 2003-2004  Ferhat Ayaz
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
#include "nanohttp-stream.h"

/*
-------------------------------------------------------------------

HTTP INPUT STREAM

-------------------------------------------------------------------
*/

static int
_http_stream_is_content_length(hpair_t * header)
{
  return hpairnode_get_ignore_case(header, HEADER_CONTENT_LENGTH) != NULL;
}

static int
_http_stream_is_chunked(hpair_t * header)
{
  char *chunked;
  chunked = hpairnode_get_ignore_case(header, HEADER_TRANSFER_ENCODING);
  if (chunked != NULL)
  {
    if (!strcmp(chunked, TRANSFER_ENCODING_CHUNKED))
    {
      return 1;
    }
  }

  return 0;
}

/**
  Creates a new input stream. 
*/
http_input_stream_t *
http_input_stream_new(hsocket_t *sock, hpair_t * header)
{
  http_input_stream_t *result;
  char *content_length;

  /* Paranoya check */
  /* if (header == NULL) return NULL; */
  /* Create object */
  if (!(result = (http_input_stream_t *) malloc(sizeof(http_input_stream_t))))
  {
    log_error2("malloc failed (%s)", strerror(errno));
    return NULL;
  }

  result->sock = sock;
  result->err = H_OK;

  /* Find connection type */
  hpairnode_dump_deep(header);
  /* Check if Content-type */
  if (_http_stream_is_content_length(header))
  {
    log_verbose1("Stream transfer with 'Content-length'");
    content_length = hpairnode_get_ignore_case(header, HEADER_CONTENT_LENGTH);
    result->content_length = atoi(content_length);
    result->received = 0;
    result->type = HTTP_TRANSFER_CONTENT_LENGTH;
  }
  /* Check if Chunked */
  else if (_http_stream_is_chunked(header))
  {
    log_verbose1("Stream transfer with 'chunked'");
    result->type = HTTP_TRANSFER_CHUNKED;
    result->chunk_size = -1;
    result->received = -1;
  }
  /* Assume connection close */
  else
  {
    log_verbose1("Stream transfer with 'Connection: close'");
    result->type = HTTP_TRANSFER_CONNECTION_CLOSE;
    result->connection_closed = 0;
    result->received = 0;
  }
  return result;
}

/**
  Creates a new input stream from file. 
  This function was added for MIME messages 
  and for debugging.
*/
http_input_stream_t *
http_input_stream_new_from_file(const char *filename)
{
  http_input_stream_t *result;
  FILE *fd;
 
  if (!(fd = fopen(filename, "rb"))) {

    log_error2("fopen failed (%s)", strerror(errno));
    return NULL;
  }

  /* Create object */
  if (!(result = (http_input_stream_t *) malloc(sizeof(http_input_stream_t)))) 
  {
    log_error2("malloc failed (%s)", strerror(errno));
    fclose(fd);
    return NULL;
  }

  result->type = HTTP_TRANSFER_FILE;
  result->fd = fd;
  result->deleteOnExit = 0;
  strcpy(result->filename, filename);

  return result;
}

/**
  Free input stream
*/
void
http_input_stream_free(http_input_stream_t * stream)
{
  if (stream->type == HTTP_TRANSFER_FILE && stream->fd)
  {
    fclose(stream->fd);
    if (stream->deleteOnExit)
      log_info2("Removing '%s'", stream->filename);
    /* remove(stream->filename); */
  }

  free(stream);
}

static int
_http_input_stream_is_content_length_ready(http_input_stream_t * stream)
{
  return (stream->content_length > stream->received);
}

static int
_http_input_stream_is_chunked_ready(http_input_stream_t * stream)
{
  return stream->chunk_size != 0;
}

static int
_http_input_stream_is_connection_closed_ready(http_input_stream_t * stream)
{
  return !stream->connection_closed;
}

static int
_http_input_stream_is_file_ready(http_input_stream_t * stream)
{
  return !feof(stream->fd);
}

static int
_http_input_stream_content_length_read(http_input_stream_t * stream,
                                       byte_t * dest, int size)
{
  herror_t status;
  int read;

  /* check limit */
  if (stream->content_length - stream->received < size)
    size = stream->content_length - stream->received;

  /* read from socket */
  status = hsocket_read(stream->sock, dest, size, 1, &read);
  if (status != H_OK)
  {
    stream->err = status;
    return -1;
  }

  stream->received += read;
  return read;
}

static int
_http_input_stream_chunked_read_chunk_size(http_input_stream_t * stream)
{
  char chunk[25];
  int status, i = 0;
  int chunk_size;
  herror_t err;

  while (1)
  {
    err = hsocket_read(stream->sock, &(chunk[i]), 1, 1, &status);
    if (status != 1)
    {
      stream->err = herror_new("_http_input_stream_chunked_read_chunk_size",
                               GENERAL_INVALID_PARAM,
                               "This should never happen!");
      return -1;
    }

    if (err != H_OK)
    {
      log_error4("[%d] %s(): %s ", herror_code(err), herror_func(err),
                 herror_message(err));

      stream->err = err;
      return -1;
    }

    if (chunk[i] == '\r' || chunk[i] == ';')
    {
      chunk[i] = '\0';
    }
    else if (chunk[i] == '\n')
    {
      chunk[i] = '\0';          /* double check */
      chunk_size = strtol(chunk, (char **) NULL, 16);   /* hex to dec */
      /* 
         log_verbose3("chunk_size: '%s' as dec: '%d'", chunk, chunk_size); */
      return chunk_size;
    }

    if (i == 24)
    {
      stream->err =
        herror_new("_http_input_stream_chunked_read_chunk_size",
                   STREAM_ERROR_NO_CHUNK_SIZE, "reached max line == %d", i);
      return -1;
    }
    else
      i++;
  }

  /* this should never happen */
  stream->err =
    herror_new("_http_input_stream_chunked_read_chunk_size",
               STREAM_ERROR_NO_CHUNK_SIZE, "reached max line == %d", i);
  return -1;
}

static int
_http_input_stream_chunked_read(http_input_stream_t * stream, byte_t * dest,
                                int size)
{
  int status, counter;
  int remain, read = 0;
  char ch;
  herror_t err;

  while (size > 0)
  {
    remain = stream->chunk_size - stream->received;

    if (remain == 0 && stream->chunk_size != -1)
    {
      /* This is not the first chunk. so skip new line until chunk size
         string */
      counter = 100;            /* maximum for stop infinity */
      while (1)
      {
        if ((err = hsocket_read(stream->sock, &ch, 1, 1, &status)) != H_OK)
        {
          stream->err = err;
          return -1;
        }

        if (ch == '\n')
        {
          break;
        }
        if (counter-- == 0)
        {
          stream->err = herror_new("_http_input_stream_chunked_read",
                                   STREAM_ERROR_WRONG_CHUNK_SIZE,
                                   "Wrong chunk-size");
          return -1;
        }
      }
    }

    if (remain == 0)
    {
      /* receive new chunk size */
      stream->chunk_size = _http_input_stream_chunked_read_chunk_size(stream);
      stream->received = 0;

      if (stream->chunk_size < 0)
      {
        /* TODO (#1#): set error flag */
        return stream->chunk_size;
      }
      else if (stream->chunk_size == 0)
      {
        return read;
      }
      remain = stream->chunk_size;
    }

    /* show remaining chunk size in socket */
    if (remain < size)
    {
      /* read from socket */
      if ((err = hsocket_read(stream->sock, &(dest[read]), remain, 1, &status)) != H_OK)
      {
        stream->err = err;
        return -1;
      }
      if (status != remain)
      {
        stream->err = herror_new("_http_input_stream_chunked_read",
                                 GENERAL_INVALID_PARAM,
                                 "This should never happen (remain=%d)(status=%d)!",
                                 remain, status);
        return -1;
      }
    }
    else
    {
      /* read from socket */
      err = hsocket_read(stream->sock, &(dest[read]), size, 1, &status);
      if (status != size)
      {
        stream->err = herror_new("_http_input_stream_chunked_read",
                                 GENERAL_INVALID_PARAM,
                                 "This should never happen (size=%d)(status=%d)!",
                                 size, status);
        return -1;
      }
      if (err != H_OK)
      {
        stream->err = err;
        return -1;
      }
    }

    read += status;
    size -= status;
    stream->received += status;
  }

  return read;
}


static int
_http_input_stream_connection_closed_read(http_input_stream_t * stream,
                                          byte_t * dest, int size)
{
  int status;
  herror_t err;

  /* read from socket */
  if ((err = hsocket_read(stream->sock, dest, size, 0, &status)) != H_OK)
  {
    stream->err = err;
    return -1;
  }

  if (status == 0)
    stream->connection_closed = 1;

  stream->received += status;
  return status;
}

static int
_http_input_stream_file_read(http_input_stream_t * stream, byte_t * dest,
                             int size)
{
  size_t len;

  if ((len = fread(dest, 1, size, stream->fd)) == -1)
  {
    stream->err = herror_new("_http_input_stream_file_read",
                             HSOCKET_ERROR_RECEIVE, "fread() returned -1");
    return -1;
  }

  return len;
}

/**
  Returns the actual status of the stream.
*/
int
http_input_stream_is_ready(http_input_stream_t * stream)
{
  /* paranoia check */
  if (stream == NULL)
    return 0;

  /* reset error flag */
  stream->err = H_OK;

  switch (stream->type)
  {
  case HTTP_TRANSFER_CONTENT_LENGTH:
    return _http_input_stream_is_content_length_ready(stream);
  case HTTP_TRANSFER_CHUNKED:
    return _http_input_stream_is_chunked_ready(stream);
  case HTTP_TRANSFER_CONNECTION_CLOSE:
    return _http_input_stream_is_connection_closed_ready(stream);
  case HTTP_TRANSFER_FILE:
    return _http_input_stream_is_file_ready(stream);
  default:
    return 0;
  }

}

/**
  Returns the actual read bytes
  <0 on error
*/
int
http_input_stream_read(http_input_stream_t * stream, byte_t * dest, int size)
{
  int len = 0;
  /* paranoia check */
  if (stream == NULL)
  {
    return -1;
  }

  /* XXX: possible memleak! reset error flag */
  stream->err = H_OK;

  switch (stream->type)
  {
  case HTTP_TRANSFER_CONTENT_LENGTH:
    len = _http_input_stream_content_length_read(stream, dest, size);
    break;
  case HTTP_TRANSFER_CHUNKED:
    len = _http_input_stream_chunked_read(stream, dest, size);
    break;
  case HTTP_TRANSFER_CONNECTION_CLOSE:
    len = _http_input_stream_connection_closed_read(stream, dest, size);
    break;
  case HTTP_TRANSFER_FILE:
    len = _http_input_stream_file_read(stream, dest, size);
    break;
  default:
    stream->err = herror_new("http_input_stream_read",
                             STREAM_ERROR_INVALID_TYPE,
                             "%d is invalid stream type", stream->type);
    return -1;
  }

  return len;
}


/*
-------------------------------------------------------------------

HTTP OUTPUT STREAM

-------------------------------------------------------------------
*/



/**
  Creates a new output stream. Transfer code will be found from header.
*/
http_output_stream_t *
http_output_stream_new(hsocket_t *sock, hpair_t * header)
{
  http_output_stream_t *result;
  char *content_length;

  /* Paranoya check */
/*  if (header == NULL)
    return NULL;
*/
  /* Create object */
  if (!(result = (http_output_stream_t *) malloc(sizeof(http_output_stream_t))))
  {
    log_error2("malloc failed (%s)", strerror(errno));
    return NULL;
  }

  result->sock = sock;
  result->sent = 0;

  /* Find connection type */

  /* Check if Content-type */
  if (_http_stream_is_content_length(header))
  {
    log_verbose1("Stream transfer with 'Content-length'");
    content_length = hpairnode_get_ignore_case(header, HEADER_CONTENT_LENGTH);
    result->content_length = atoi(content_length);
    result->type = HTTP_TRANSFER_CONTENT_LENGTH;
  }
  /* Check if Chunked */
  else if (_http_stream_is_chunked(header))
  {
    log_verbose1("Stream transfer with 'chunked'");
    result->type = HTTP_TRANSFER_CHUNKED;
  }
  /* Assume connection close */
  else
  {
    log_verbose1("Stream transfer with 'Connection: close'");
    result->type = HTTP_TRANSFER_CONNECTION_CLOSE;
  }

  return result;
}

/**
  Free output stream
*/
void
http_output_stream_free(http_output_stream_t * stream)
{
  free(stream);

  return;
}

/**
  Writes 'size' bytes of 'bytes' into stream.
  Returns socket error flags or H_OK.
*/
herror_t
http_output_stream_write(http_output_stream_t * stream,
                         const byte_t * bytes, int size)
{
  herror_t status;
  char chunked[15];

  if (stream->type == HTTP_TRANSFER_CHUNKED)
  {
    sprintf(chunked, "%x\r\n", size);
    if ((status = hsocket_send(stream->sock, chunked)) != H_OK)
      return status;
  }

  if (size > 0)
  {
    if ((status = hsocket_nsend(stream->sock, bytes, size)) != H_OK)
      return status;
  }

  if (stream->type == HTTP_TRANSFER_CHUNKED)
  {
    if ((status = hsocket_send(stream->sock, "\r\n")) != H_OK)
      return status;
  }

  return H_OK;
}

/**
  Writes 'strlen()' bytes of 'str' into stream.
  Returns socket error flags or H_OK.
*/
herror_t
http_output_stream_write_string(http_output_stream_t * stream,
                                const char *str)
{
  return http_output_stream_write(stream, str, strlen(str));
}


herror_t
http_output_stream_flush(http_output_stream_t * stream)
{
  herror_t status;

  if (stream->type == HTTP_TRANSFER_CHUNKED)
  {
    if ((status = hsocket_send(stream->sock, "0\r\n\r\n")) != H_OK)
      return status;
  }

  return H_OK;
}
