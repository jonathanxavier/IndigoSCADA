/******************************************************************
*  $Id: soap-router.c,v 1.10 2006/07/09 16:24:19 snowdrop Exp $
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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include <nanohttp/nanohttp-logging.h>

#include "soap-router.h"

SoapRouter *
soap_router_new(void)
{
  SoapRouter *router;

  if (!(router = (SoapRouter *) malloc(sizeof(SoapRouter))))
  {
    log_error2("malloc failed (%s)", strerror(errno));
    return NULL;
  }
  memset(router, 0, sizeof(SoapRouter));

  return router;
}

void
soap_router_register_service(SoapRouter * router,
                             SoapServiceFunc func,
                             const char *method, const char *urn)
{
  SoapService *service;

  service = soap_service_new(urn, method, func);

  if (router->service_tail == NULL)
  {
    router->service_head =
      router->service_tail = soap_service_node_new(service, NULL);
  }
  else
  {
    router->service_tail->next = soap_service_node_new(service, NULL);
    router->service_tail = router->service_tail->next;
  }

  return;
}

void
soap_router_register_security(SoapRouter * router, httpd_auth auth)
{
  router->auth = auth;

  return;
}

void
soap_router_register_description(SoapRouter * router, xmlDocPtr wsdl)
{
  if (router->wsdl)
    xmlFreeDoc(router->wsdl);

  router->wsdl = xmlCopyDoc(wsdl, 1);

  return;
}

void
soap_router_register_default_service(SoapRouter *router, SoapServiceFunc func, const char *method, const char *urn) {

  SoapService *service;

  service = soap_service_new(urn, method, func);

  if (router->service_tail == NULL)
  {
    router->service_head = router->service_tail = soap_service_node_new(service, NULL);
  }
  else
  {
    router->service_tail->next = soap_service_node_new(service, NULL);
    router->service_tail = router->service_tail->next;
  }

  router->default_service = service;

  return;
}

SoapService *
soap_router_find_service(SoapRouter * router,
                         const char *urn, const char *method)
{
  SoapServiceNode *node;

  if (router == NULL || urn == NULL || method == NULL)
    return NULL;

  node = router->service_head;

  while (node)
  {
    if (node->service && node->service->urn && node->service->method)
    {

      if (!strcmp(node->service->urn, urn)
          && !strcmp(node->service->method, method))
        return node->service;

    }

    node = node->next;
  }

  return router->default_service;
}


void
soap_router_free(SoapRouter * router)
{
  SoapServiceNode *node;
  log_verbose2("enter: router=%p", router);

  if (!router)
    return;

  while (router->service_head)
  {
    node = router->service_head->next;
    /* log_verbose2("soap_service_free(%p)\n",
       router->service_head->service); */
    soap_service_free(router->service_head->service);
    free(router->service_head);
    router->service_head = node;
  }
  if (router->wsdl)
    xmlFreeDoc(router->wsdl);

  free(router);
  log_verbose1("leave with success");

  return;
}
