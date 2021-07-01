/******************************************************************
*  $Id: nanohttp-socket.c,v 1.61 2006/07/09 16:24:19 snowdrop Exp $
*
* CSOAP Project:  A http client/server library in C
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
* Email: ferhatayaz@yahoo.com
******************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef WIN32
#include "wsockcompat.h"
#include <winsock2.h>
#include <process.h>

#define inline

#ifndef __MINGW32__
typedef int ssize_t;
#endif

#undef errno
#define errno WSAGetLastError()

#endif

#ifdef MEM_DEBUG
#include <utils/alloc.h>
#endif

#include "nanohttp-logging.h"
#include "nanohttp-socket.h"
#include "nanohttp-common.h"
#include "nanohttp-ssl.h"

#ifdef WIN32
static inline void
_hsocket_module_sys_init(int argc, char **argv)
{
  struct WSAData info;
  WSAStartup(MAKEWORD(2, 2), &info);

  return;
}

static inline void
_hsocket_module_sys_destroy(void)
{
  WSACleanup();

  return;
}
#else
static inline void
_hsocket_module_sys_init(int argc, char **argv)
{
  return;
}
static inline void
_hsocket_module_sys_destroy(void)
{
  return;
}
#endif

/*--------------------------------------------------
FUNCTION: hsocket_module_init
NOTE: This will be called from httpd_init()
	for server and from httpc_init() for client
----------------------------------------------------*/
herror_t
hsocket_module_init(int argc, char **argv)
{
  _hsocket_module_sys_init(argc, argv);

  return hssl_module_init(argc, argv);
}

/*--------------------------------------------------
FUNCTION: hsocket_module_destroy
----------------------------------------------------*/
void
hsocket_module_destroy(void)
{
  _hsocket_module_sys_destroy();

  return;
}

/*--------------------------------------------------
FUNCTION: hsocket_init
----------------------------------------------------*/
herror_t
hsocket_init(hsocket_t * sock)
{

  memset(sock, 0, sizeof(hsocket_t));
  sock->sock = HSOCKET_FREE;

  return H_OK;
}

/*--------------------------------------------------
FUNCTION: hsocket_free
----------------------------------------------------*/
void
hsocket_free(hsocket_t * sock)
{
  /* nop */

  return;
}

/*--------------------------------------------------
FUNCTION: hsocket_open
----------------------------------------------------*/
herror_t
hsocket_open(hsocket_t * dsock, const char *hostname, int port, int ssl)
{
  struct sockaddr_in address;
  struct hostent *host;
  char *ip;

  if ((dsock->sock = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    return herror_new("hsocket_open", HSOCKET_ERROR_CREATE,
                      "Socket error (%s)", strerror(errno));

  /* Get host data */
  if (!(host = gethostbyname(hostname)))
    return herror_new("hsocket_open", HSOCKET_ERROR_GET_HOSTNAME,
                      "Socket error (%s)", strerror(errno));

  ip = inet_ntoa(*(struct in_addr *) *host->h_addr_list);
  address.sin_addr.s_addr = inet_addr(ip);

  /* set server addresss */
  address.sin_family = host->h_addrtype;
  address.sin_port = htons((unsigned short) port);

  log_verbose4("Opening %s://%s:%i", ssl ? "https" : "http", hostname, port);

  /* connect to the server */
  if (connect(dsock->sock, (struct sockaddr *) &address, sizeof(address)) !=
      0)
    return herror_new("hsocket_open", HSOCKET_ERROR_CONNECT,
                      "Socket error (%s)", strerror(errno));

  if (ssl)
  {
    herror_t status;

    if ((status = hssl_client_ssl(dsock)) != H_OK)
    {
      log_error2("hssl_client_ssl failed (%s)", herror_message(status));
      return status;
    }
  }

  return H_OK;
}

/*--------------------------------------------------
FUNCTION: hsocket_bind
----------------------------------------------------*/
herror_t
hsocket_bind(hsocket_t * dsock, int port)
{
  hsocket_t sock;
  struct sockaddr_in addr;
  int opt = 1;

  /* create socket */
  if ((sock.sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    log_error2("Cannot create socket (%s)", strerror(errno));
    return herror_new("hsocket_bind", HSOCKET_ERROR_CREATE,
                      "Socket error (%s)", strerror(errno));
  }

  setsockopt(sock.sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  /* bind socket */
  addr.sin_family = AF_INET;
  addr.sin_port = htons((unsigned short) port); /* short, network byte order */
  addr.sin_addr.s_addr = INADDR_ANY;
  memset(&(addr.sin_zero), '\0', 8);    /* zero the rest of the struct */

  if (bind(sock.sock, (struct sockaddr *) &addr, sizeof(struct sockaddr)) ==
      -1)
  {
    log_error2("Cannot bind socket (%s)", strerror(errno));
    return herror_new("hsocket_bind", HSOCKET_ERROR_BIND, "Socket error (%s)",
                      strerror(errno));
  }
  dsock->sock = sock.sock;
  return H_OK;
}

#ifdef WIN32
static herror_t
_hsocket_sys_accept(hsocket_t * sock, hsocket_t * dest)
{
  int asize;
  hsocket_t sockfd;

  asize = sizeof(struct sockaddr_in);
  while (1)
  {
    sockfd.sock =
      accept(sock->sock, (struct sockaddr *) &(dest->addr), &asize);
    if (sockfd.sock == INVALID_SOCKET)
    {
      if (WSAGetLastError() != WSAEWOULDBLOCK)
        return herror_new("hsocket_accept", HSOCKET_ERROR_ACCEPT,
                          "Socket error (%s)", strerror(errno));
    }
    else
    {
      break;
    }
  }

  dest->sock = sockfd.sock;

  return H_OK;
}
#else
static herror_t
_hsocket_sys_accept(hsocket_t * sock, hsocket_t * dest)
{
  socklen_t len;

  len = sizeof(struct sockaddr_in);

  if ((dest->sock =
       accept(sock->sock, (struct sockaddr *) &(dest->addr), &len)) == -1)
  {
    log_warn2("accept failed (%s)", strerror(errno));
    return herror_new("hsocket_accept", HSOCKET_ERROR_ACCEPT,
                      "Cannot accept network connection (%s)",
                      strerror(errno));
  }

  return H_OK;
}
#endif

/*----------------------------------------------------------
FUNCTION: hsocket_accept
----------------------------------------------------------*/
herror_t
hsocket_accept(hsocket_t * sock, hsocket_t * dest)
{
  herror_t status;

  if (sock->sock < 0)
    return herror_new("hsocket_accept", HSOCKET_ERROR_NOT_INITIALIZED,
                      "hsocket_t not initialized");

  if ((status = _hsocket_sys_accept(sock, dest)) != H_OK)
    return status;

  if ((status = hssl_server_ssl(dest)) != H_OK)
  {
    log_warn2("SSL startup failed (%s)", herror_message(status));
    return status;
  }

  log_verbose3("accepting connection from '%s' socket=%d",
               SAVE_STR(((char *) inet_ntoa(dest->addr.sin_addr))),
               dest->sock);

  return H_OK;
}

/*--------------------------------------------------
FUNCTION: hsocket_listen
----------------------------------------------------*/
herror_t
hsocket_listen(hsocket_t * sock)
{
  if (sock->sock < 0)
    return herror_new("hsocket_listen", HSOCKET_ERROR_NOT_INITIALIZED,
                      "Called hsocket_listen before initializing!");

  if (listen(sock->sock, 15) == -1)
  {
    log_error2("listen failed (%s)", strerror(errno));
    return herror_new("hsocket_listen", HSOCKET_ERROR_LISTEN,
                      "Cannot listen on this socket (%s)", strerror(errno));
  }

  return H_OK;
}

#ifdef WIN32
static inline void
_hsocket_sys_close(hsocket_t * sock)
{
  char junk[10];

  /* shutdown(sock,SD_RECEIVE); */

  shutdown(sock->sock, SD_SEND);
  while (recv(sock->sock, junk, sizeof(junk), 0) > 0);
  /* nothing */
  closesocket(sock->sock);

  return;
}
#else
static inline void
_hsocket_sys_close(hsocket_t * sock)
{

  shutdown(sock->sock, SHUT_RDWR);

  close(sock->sock);

  return;
}
#endif

/*--------------------------------------------------
FUNCTION: hsocket_close
----------------------------------------------------*/
void
hsocket_close(hsocket_t * sock)
{
  log_verbose3("closing socket %p (%d)...", sock, sock->sock);

  hssl_cleanup(sock);

  _hsocket_sys_close(sock);

  log_verbose1("socket closed");

  return;
}

/*--------------------------------------------------
FUNCTION: hsocket_send
----------------------------------------------------*/
herror_t
hsocket_nsend(hsocket_t * sock, const byte_t * bytes, int n)
{
  herror_t status;
  size_t total = 0;
  size_t size;

  log_verbose2("Starting to send on sock=%p", &sock);
  if (sock->sock < 0)
    return herror_new("hsocket_nsend", HSOCKET_ERROR_NOT_INITIALIZED,
                      "hsocket not initialized");

  /* log_verbose2( "SENDING %s", bytes ); */

  while (1)
  {

    if ((status = hssl_write(sock, bytes + total, n, &size)) != H_OK)
    {
      log_warn2("hssl_write failed (%s)", herror_message(status));
      return status;
    }

    n -= size;
    total += size;
    if (n <= 0)
      break;
  }

  return H_OK;
}

/*--------------------------------------------------
FUNCTION: hsocket_send
----------------------------------------------------*/
herror_t
hsocket_send(hsocket_t * sock, const char *str)
{
  return hsocket_nsend(sock, str, strlen(str));
}

int
hsocket_select_read(int sock, char *buf, size_t len)
{
  struct timeval timeout;
  fd_set fds;
  int ret;
  FD_ZERO(&fds);
  FD_SET(sock, &fds);
  timeout.tv_sec = httpd_get_timeout();
  timeout.tv_usec = 0;
  ret = select(sock + 1, &fds, NULL, NULL, &timeout);
  if (ret == 0)
  {
    //errno = ETIMEDOUT; apa---
    log_verbose2("Socket %d timeout", sock);
    return -1;
  }
#ifdef WIN32
  return recv(sock, buf, len, 0);
#else
  return read(sock, buf, len);
#endif
}

herror_t
hsocket_read(hsocket_t * sock, byte_t * buffer, int total, int force,
             int *received)
{
  herror_t status;
  size_t totalRead;
  size_t count;

/* log_verbose3("Entering hsocket_read(total=%d,force=%d)", total, force); */

  totalRead = 0;
  do
  {

    if ((status =
         hssl_read(sock, &buffer[totalRead], (size_t) total - totalRead,
                   &count)) != H_OK)
    {
      log_warn2("hssl_read failed (%s)", herror_message(status));
      return status;
    }

    if (!force)
    {
      /* log_verbose3("Leaving !force (received=%d)(status=%d)", *received,
         status); */
      *received = count;
      return H_OK;
    }

    totalRead += count;

    if (totalRead == total)
    {
      *received = totalRead;
      /* 
         log_verbose4("Leaving totalRead == total
         (received=%d)(status=%d)(totalRead=%d)", *received, status,
         totalRead); */
      return H_OK;
    }
  }
  while (1);
}
