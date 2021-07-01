/******************************************************************
 *  $Id: nanohttp-response.h,v 1.7 2006/03/06 13:37:38 m0gg Exp $
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
#ifndef NANO_HTTP_RESPONSE_H
#define NANO_HTTP_RESPONSE_H

#include <nanohttp/nanohttp-stream.h>
#include <nanohttp/nanohttp-common.h>
#include <nanohttp/nanohttp-mime.h>

/* response object */
typedef struct hresponse
{
  http_version_t version;
  int errcode;
  char desc[RESPONSE_MAX_DESC_SIZE];

  hpair_t *header;

  http_input_stream_t *in;
  content_type_t *content_type;
  attachments_t *attachments;
  char root_part_id[150];
} hresponse_t;

#ifdef __cplusplus
extern "C" {
#endif

herror_t hresponse_new_from_socket(hsocket_t *sock, hresponse_t ** out);
void hresponse_free(hresponse_t * res);

#ifdef __cplusplus
}
#endif

#endif
