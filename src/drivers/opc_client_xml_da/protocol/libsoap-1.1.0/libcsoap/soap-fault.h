/******************************************************************
 *  $Id: soap-fault.h,v 1.5 2006/03/06 13:37:38 m0gg Exp $
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
#ifndef cSOAP_FAULT_H
#define cSOAP_FAULT_H

#include <libcsoap/soap-xml.h>



typedef enum _fault_code
{
  Fault_VersionMismatch,
  Fault_MustUnderstand,
  Fault_Client,
  Fault_Server
} fault_code_t;

#ifdef __cplusplus
extern "C" {
#endif

xmlDocPtr soap_fault_build(fault_code_t faultcode,
                           const char *faultstring,
                           const char *faultactor, const char *detail);

#ifdef __cplusplus
}
#endif

#endif
