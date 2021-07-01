/******************************************************************
*  $Id: soap-fault.c,v 1.11 2006/07/09 16:24:19 snowdrop Exp $
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

#include <nanohttp/nanohttp-logging.h>

#include "soap-fault.h"
#include "soap-xml.h"

/*
Parameters:
1- soap_env_ns
2- soap_env_enc
3- xsi_ns
4- xsd_ns
5- faultcode
6- faultstring
7- faultactor
8- detail
*/
#define  _SOAP_FAULT_TEMPLATE_ \
	"<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"%s\" SOAP-ENV:encoding=\"%s\"" \
	" xmlns:xsi=\"%s\"" \
	" xmlns:xsd=\"%s\">" \
        " <SOAP-ENV:Header />" \
	" <SOAP-ENV:Body>" \
	"  <SOAP-ENV:Fault>"\
	"   <faultcode>%s</faultcode>"\
	"   <faultstring>%s</faultstring>"\
	"   <faultactor>%s</faultactor>"\
	"   <detail>%s</detail>"\
	"  </SOAP-ENV:Fault>" \
	" </SOAP-ENV:Body>"\
	"</SOAP-ENV:Envelope>"



static char *fault_vm = "VersionMismatch";
static char *fault_mu = "MustUnderstand";
static char *fault_client = "Client";
static char *fault_server = "Server";

xmlDocPtr
soap_fault_build(fault_code_t fcode,
                 const char *faultstring,
                 const char *faultactor, const char *detail)
{

  /* variables */
  char *faultcode;
  int bufferlen = 2000;
  char *buffer;
  xmlDocPtr fault;              /* result */

  log_verbose1("Build fault");

  switch (fcode)
  {
  case Fault_VersionMismatch:
    faultcode = fault_vm;
    break;
  case Fault_MustUnderstand:
    faultcode = fault_mu;
    break;
  case Fault_Client:
    faultcode = fault_client;
    break;
  case Fault_Server:
    faultcode = fault_server;
    break;
  default:
    faultcode = fault_client;
  }

  /* calculate buffer length */
  if (faultstring)
    bufferlen += strlen(faultstring);
  if (faultactor)
    bufferlen += strlen(faultactor);
  if (detail)
    bufferlen += strlen(detail);

  log_verbose2("Creating buffer with %d bytes", bufferlen);
  buffer = (char *) malloc(bufferlen);

  sprintf(buffer, _SOAP_FAULT_TEMPLATE_,
          soap_env_ns, soap_env_enc, soap_xsi_ns,
          soap_xsd_ns, faultcode,
          faultstring ? faultstring : "error",
          faultactor ? faultactor : "", detail ? detail : "");

  fault = xmlParseDoc(BAD_CAST buffer);
  free(buffer);

  if (fault == NULL)
  {
    log_error1("Can not create xml document!");

    return soap_fault_build(fcode, "Can not create fault object in xml",
                            "soap_fault_build()", NULL);
  }

  log_verbose2("Returning fault (%p)", fault);
  return fault;

}
