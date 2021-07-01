/******************************************************************
*  $Id: nanohttp-ssl.h,v 1.18 2006/05/15 06:40:47 m0gg Exp $
*
* CSOAP Project:  A http client/server library in C
* Copyright (C) 2001-2005  Rochester Institute of Technology
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
* Author: Matt Campbell
******************************************************************/
#ifndef __NANOHTTP_SSL_H_
#define __NANOHTTP_SSL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SSL

#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *
 * Initialization and shutdown of the SSL module
 *
 */
  herror_t hssl_module_init(int argc, char **argv);
  void hssl_module_destroy(void);

  void hssl_set_certificate(char *c);
  void hssl_set_certpass(char *c);
  void hssl_set_ca(char *c);
  void hssl_enable(void);

  int hssl_enabled(void);

/**
 *
 * Socket initialization and shutdown
 *
 */
  herror_t hssl_client_ssl(hsocket_t * sock);
  herror_t hssl_server_ssl(hsocket_t * sock);

  void hssl_cleanup(hsocket_t * sock);

/*
 * Callback for password checker
 */
/* static int pw_cb(char* buf, int num, int rwflag, void *userdata); */

/*
 * Quick function for verifying a portion of the cert
 * nid is any NID_ defined in <openssl/objects.h>
 * returns non-zero if everything went ok
 */
#define CERT_SUBJECT	1

  int verify_sn(X509 * cert, int who, int nid, char *str);

/*
 * Called by framework for verify
 */

/* static int verify_cb(int prev_ok, X509_STORE_CTX* ctx); */

  void hssl_set_user_verify(int func(X509 * cert));

#ifdef __cplusplus
}
#endif

#else /* HAVE_SSL */

static __inline herror_t
hssl_module_init(int argc, char **argv)
{
  return H_OK;
}
static __inline void
hssl_module_destroy(void)
{
  return;
}

static __inline int
hssl_enabled(void)
{
  return 0;
}

static __inline herror_t
hssl_client_ssl(hsocket_t * sock)
{
  return H_OK;
}

static __inline herror_t
hssl_server_ssl(hsocket_t * sock)
{
  return H_OK;
}

static __inline void
hssl_cleanup(hsocket_t * sock)
{
  return;
}

#endif /* HAVE_SSL */

#ifdef __cplusplus
extern "C"
{
#endif

  herror_t hssl_read(hsocket_t * sock, char *buf, size_t len,
                     size_t * received);
  herror_t hssl_write(hsocket_t * sock, const char *buf, size_t len,
                      size_t * sent);

#ifdef __cplusplus
}
#endif

#endif
