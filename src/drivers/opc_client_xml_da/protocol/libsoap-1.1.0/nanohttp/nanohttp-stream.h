/******************************************************************
 *  $Id: nanohttp-stream.h,v 1.10 2006/03/06 13:37:38 m0gg Exp $
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
#ifndef NANO_HTTP_STREAM_H
#define NANO_HTTP_STREAM_H

#include <stdio.h>

#include <nanohttp/nanohttp-socket.h>
#include <nanohttp/nanohttp-common.h>

/*
  HTTP Stream modul:

  nanohttp supports 2 different streams:

  1. http_input_stream_t
  2. http_output_stream_t

  These are not regular streams. They will care about 
  transfer styles while sending/receiving data.

  Supported transfer styles are

  o Content-length
  o Chunked encoding
  o Connection: "close"

  A stream will set its transfer style from the header
  information, which must be given while creating a stream.

  A stream will start sending/receiving data "after" 
  sending/receiving header information. (After <CF><CF>)"
  
*/

/**
  Transfer types supported while 
  sending/receiving data.
*/
typedef enum http_transfer_type
{
  /** The stream cares about Content-length */
  HTTP_TRANSFER_CONTENT_LENGTH,

  /** The stream sends/receives chunked data */
  HTTP_TRANSFER_CHUNKED,

  /** The stream sends/receives data until 
    connection is closed */
  HTTP_TRANSFER_CONNECTION_CLOSE,

  /** This transfer style will be used by MIME support 
    and for debug purposes.*/
  HTTP_TRANSFER_FILE
} http_transfer_type_t;


/**
  HTTP INPUT STREAM. Receives data from a socket/file
  and cares about the transfer style.
*/
typedef struct http_input_stream
{
  hsocket_t *sock;
  herror_t err;
  http_transfer_type_t type;
  int received;
  int content_length;
  int chunk_size;
  byte_t connection_closed;

  /* file handling */
  FILE *fd;
  char filename[255];
  int deleteOnExit;             /* default is 0 */
} http_input_stream_t;


/**
  HTTP OUTPUT STREAM. Sends data to a socket
  and cares about the transfer style.
*/
typedef struct http_output_stream
{
  hsocket_t *sock;
  http_transfer_type_t type;
  int content_length;
  int sent;
} http_output_stream_t;


#ifdef __cplusplus
extern "C" {
#endif

/*
--------------------------------------------------------------
  HTTP INPUT STREAM
--------------------------------------------------------------
*/

/**
  Creates a new input stream. The transfer style will be 
  choosen from the given header.

  @param sock the socket to receive data from
  @param header the http header. This must be received before
    creating a http_input_stream_t.

  @returns  a newly created http_input_stream_t object. If no 
    transfer style was found in the header, 
    HTTP_TRANSFER_CONNECTION_CLOSE  will be used as default.

  @see http_input_stream_free
*/
http_input_stream_t *http_input_stream_new(hsocket_t *sock, hpair_t *header);


/**
  Creates a new input stream from file. 
  This function was added for MIME messages 
  and for debugging. The transfer style is always 
  HTTP_TRANSFER_FILE.

  @param filename the name of the file to open and read. 

  @returns The return value is a http_input_stream_t object 
  if the file exists and could be opened. NULL otherwise.

  @see   http_input_stream_free
*/
http_input_stream_t *http_input_stream_new_from_file(const char *filename);


/**
  Free input stream. Note that the socket will not be closed
  by this functions.

  @param stream the input stream to free.
*/
void http_input_stream_free(http_input_stream_t * stream);


/**
  Returns the actual status of the stream.

  @param stream the stream to check its status
  @returns <br>1 if there are still data to read. 
           <br>0 if no more data exists.
*/
int http_input_stream_is_ready(http_input_stream_t * stream);


/**
  Tries to read 'size' bytes from the stream. Check always 
  with http_input_stream_is_ready() if there are some data
  to read. If it returns 0, no more data is available on 
  stream.
  <P>
  On error this function will return -1. In this case the 
  "err" field of stream will be set to the actual error.
  This can be one of the followings: <P>
  
  <BR>STREAM_ERROR_NO_CHUNK_SIZE 
  <BR>STREAM_ERROR_WRONG_CHUNK_SIZE
  <BR>STREAM_ERROR_INVALID_TYPE
  <BR>HSOCKET_ERROR_RECEIVE  

  @param stream the stream to read data from
  @param dest destination memory to store readed bytes
  @param size maximum size of 'dest' (size to read)

  @returns the actual readed bytes or -1 on error.
*/
int http_input_stream_read(http_input_stream_t * stream,
                           byte_t * dest, int size);


/*
--------------------------------------------------------------
  HTTP OUTPUT STREAM
--------------------------------------------------------------
*/

/**
  Creates a new output stream. Transfer style will be found 
  from the given header.

  @param sock the socket to to send data to
  @param header the header which must be sent before

  @returns a http_output_stream_t object. If no proper transfer
  style was found in the header, HTTP_TRANSFER_CONNECTION_CLOSE
  will be used as default.

  @see http_output_stream_free
*/
http_output_stream_t *http_output_stream_new(hsocket_t *sock,
                                             hpair_t * header);


/**
  Free output stream. Note that this functions will not 
  close any socket connections.

  @param stream the stream to free.
*/
void http_output_stream_free(http_output_stream_t * stream);


/**
  Writes 'size' bytes of 'bytes' into stream.

  @param stream the stream to use to send data
  @param bytes bytes to send
  @param size size of bytes to send

  @returns H_OK if success. One of the followings otherwise
    <BR>HSOCKET_ERROR_NOT_INITIALIZED
    <BR>HSOCKET_ERROR_SEND
*/
herror_t http_output_stream_write(http_output_stream_t * stream,
                                  const byte_t * bytes, int size);

/**
  Writes a null terminated string to the stream.

  @param stream the stream to use to send data
  @param str a null terminated string to send

  @returns H_OK if success. One of the followings otherwise
    <BR>HSOCKET_ERROR_NOT_INITIALIZED
    <BR>HSOCKET_ERROR_SEND
*/
herror_t http_output_stream_write_string(http_output_stream_t * stream,
                                         const char *str);


/**
  Sends finish flags if nesseccary (like in chunked transport).
  Call always this function before closing the connections.

  @param stream the stream to send post data.

  @returns H_OK if success. One of the followings otherwise
    <BR>HSOCKET_ERROR_NOT_INITIALIZED
    <BR>HSOCKET_ERROR_SEND
*/
herror_t http_output_stream_flush(http_output_stream_t * stream);

#ifdef __cplusplus
}
#endif

#endif
