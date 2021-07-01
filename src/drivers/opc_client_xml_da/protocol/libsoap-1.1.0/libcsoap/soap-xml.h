/******************************************************************
 *  $Id: soap-xml.h,v 1.8 2006/03/06 13:37:38 m0gg Exp $
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
#ifndef cSOAP_XML_H
#define cSOAP_XML_H

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <nanohttp/nanohttp-common.h>

static const char * const soap_env_ns = "http://schemas.xmlsoap.org/soap/envelope/";
static const char * const soap_env_enc = "http://schemas.xmlsoap.org/soap/encoding/";
static const char * const soap_xsi_ns = "http://www.w3.org/1999/XMLSchema-instance";
static const char * const soap_xsd_ns = "http://www.w3.org/1999/XMLSchema";

typedef int (*soap_xmlnode_callback) (xmlNodePtr, void *);

#ifdef __cplusplus
extern "C" {
#endif

xmlNodePtr soap_xml_get_children(xmlNodePtr param);
xmlNodePtr soap_xml_get_next(xmlNodePtr param);

xmlXPathObjectPtr soap_xpath_eval(xmlDocPtr doc, const char *xpath);

int
soap_xpath_foreach(xmlDocPtr doc, const char *xpath,
                   soap_xmlnode_callback cb, void *userdata);


void soap_xml_doc_print(xmlDocPtr doc);
char *soap_xml_get_text(xmlNodePtr node);

#ifdef __cplusplus
}
#endif

#endif
