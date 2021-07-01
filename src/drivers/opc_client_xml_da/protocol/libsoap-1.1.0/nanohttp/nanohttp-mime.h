/******************************************************************
*  _  _   _   _  _   __
* | \/ | | | | \/ | | _/
* |_''_| |_| |_''_| |_'/  PARSER
*
*  $Id: nanohttp-mime.h,v 1.8 2006/02/27 22:26:02 snowdrop Exp $
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

#ifndef NANO_HTTP_MIME_PARSER_H
#define NANO_HTTP_MIME_PARSER_H

#include <nanohttp/nanohttp-common.h>
#include <nanohttp/nanohttp-stream.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------
  "multipart/related"  MIME Message Builder
 ------------------------------------------------------------------*/



herror_t mime_get_attachments(content_type_t * ctype,
                              http_input_stream_t * in,
                              attachments_t ** dest);

#ifdef __cplusplus
}
#endif

#endif
