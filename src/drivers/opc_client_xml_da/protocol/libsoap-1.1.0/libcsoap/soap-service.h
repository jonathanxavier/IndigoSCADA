/******************************************************************
 *  $Id: soap-service.h,v 1.6 2006/03/06 13:37:38 m0gg Exp $
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
 * Email: ferhatayaz@jprogrammer.net
 ******************************************************************/
#ifndef cSOAP_SERVICE_H
#define cSOAP_SERVICE_H

#include <libcsoap/soap-env.h>
#include <libcsoap/soap-ctx.h>

typedef herror_t(*SoapServiceFunc) (SoapCtx *, SoapCtx *);


typedef struct _SoapService
{
  char *urn;
  char *method;
  SoapServiceFunc func;
} SoapService;


typedef struct _SoapServiceNode
{
  SoapService *service;
  struct _SoapServiceNode *next;
} SoapServiceNode;


#ifdef __cplusplus
extern "C" {
#endif

SoapServiceNode *soap_service_node_new(SoapService * service,
                                       SoapServiceNode * next);

SoapService *soap_service_new(const char *urn, const char *method, SoapServiceFunc f);

void soap_service_free(SoapService * service);

#ifdef __cplusplus
}
#endif

#endif
