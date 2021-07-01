/******************************************************************
 *  $Id: soap-ctx.h,v 1.9 2006/03/06 13:37:38 m0gg Exp $
 *
 * CSOAP Project:  A SOAP client/server library in C
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
 * Email: ferhatayaz@jprogrammer.net
 ******************************************************************/
#ifndef cSOAP_CTX_H
#define cSOAP_CTX_H

#include <nanohttp/nanohttp-common.h>
#include <nanohttp/nanohttp-server.h>

#include <libcsoap/soap-env.h>

#define SOAP_ERROR_NO_FILE_ATTACHED 4001
#define SOAP_ERROR_EMPTY_ATTACHMENT 4002

#define MAX_HREF_SIZE 150

typedef struct _SoapCtx
{
  SoapEnv *env;
  char *action;
  hrequest_t *http;
  attachments_t *attachments;
} SoapCtx;

#ifdef __cplusplus
extern "C" {
#endif

SoapCtx *soap_ctx_new(SoapEnv * env);   /* should only be used internally */

/**
	Returns the attached file if any found. 
	@param ctx the SoapCtx object which should contain the part
	@param node the xml node which points to a file via the "href" xml attribute

	@returns a part_t object of attachment was found, NULL otherwise.
	  
*/
part_t *soap_ctx_get_file(SoapCtx * ctx, xmlNodePtr node);

/**
	Creates a new soap context object.
*/
herror_t soap_ctx_new_with_method(const char *urn, const char *method,
                                  SoapCtx ** out);

/* Size of destination dest_href should be MAX_HREF_SIZE */
herror_t soap_ctx_add_file(SoapCtx * ctx, const char *filename,
                           const char *content_type, char *dest_href);
/* 
Used internally. Will switch the deleteOnExit flag from the 
given one to the added part.
*/
void soap_ctx_add_files(SoapCtx * ctx, attachments_t * attachments);
void soap_ctx_free(SoapCtx * ctx);

#ifdef __cplusplus
}
#endif

#endif
