//apa derived from libevent
#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <winerror.h>
#include "ws2tcpip.h"
#include "stdint.h"
#undef WIN32_LEAN_AND_MEAN
#include <io.h>
#include <tchar.h>
#include <process.h>
#undef _WIN32_WINNT
/* For structs needed by GetAdaptersAddresses */

#include <sys/types.h>
#ifdef EVENT__HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef EVENT__HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef EVENT__HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#ifdef EVENT__HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef EVENT__HAVE_NETINET_IN6_H
#include <netinet/in6.h>
#endif
#ifdef EVENT__HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef EVENT__HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <time.h>
#include <sys/stat.h>
#ifdef EVENT__HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif
#endif

#ifdef _WIN32
#define open _open
#define read _read
#define close _close
#ifndef fstat
#define fstat _fstati64
#endif
#ifndef stat
#define stat _stati64
#endif
#define mode_t int
#endif

#define EVUTIL_EAI_NEED_RESOLVE      -90002
#define EVUTIL_EAI_AGAIN -902
#define EVUTIL_EAI_FAIL -904
#define EVUTIL_EAI_MEMORY -906
#define EVUTIL_EAI_FAMILY -905
#define EVUTIL_EAI_NODATA -907
#define EVUTIL_EAI_NONAME -908
#define EVUTIL_AI_ADDRCONFIG 0x40000

/**
 * A type wide enough to hold the output of "socket()" or "accept()".  On
 * Windows, this is an intptr_t; elsewhere, it is an int. */
#ifdef _WIN32
#define evutil_socket_t intptr_t
#else
#define evutil_socket_t int
#endif

#ifdef _WIN32
#define ev_socklen_t int
#elif defined(EVENT__socklen_t)
#define ev_socklen_t EVENT__socklen_t
#else
#define ev_socklen_t socklen_t
#endif

int
evutil_closesocket(evutil_socket_t sock)
{
#ifndef _WIN32
	return close(sock);
#else
	return closesocket(sock);
#endif
}

/* Tables to implement ctypes-replacement EVUTIL_IS*() functions.  Each table
 * has 256 bits to look up whether a character is in some set or not.  This
 * fails on non-ASCII platforms, but so does every other place where we
 * take a char and write it onto the network.
 **/
static const uint32_t EVUTIL_ISALPHA_TABLE[8] =
  { 0, 0, 0x7fffffe, 0x7fffffe, 0, 0, 0, 0 };
static const uint32_t EVUTIL_ISALNUM_TABLE[8] =
  { 0, 0x3ff0000, 0x7fffffe, 0x7fffffe, 0, 0, 0, 0 };
static const uint32_t EVUTIL_ISSPACE_TABLE[8] = { 0x3e00, 0x1, 0, 0, 0, 0, 0, 0 };
static const uint32_t EVUTIL_ISXDIGIT_TABLE[8] =
  { 0, 0x3ff0000, 0x7e, 0x7e, 0, 0, 0, 0 };
static const uint32_t EVUTIL_ISDIGIT_TABLE[8] = { 0, 0x3ff0000, 0, 0, 0, 0, 0, 0 };
static const uint32_t EVUTIL_ISPRINT_TABLE[8] =
  { 0, 0xffffffff, 0xffffffff, 0x7fffffff, 0, 0, 0, 0x0 };
static const uint32_t EVUTIL_ISUPPER_TABLE[8] = { 0, 0, 0x7fffffe, 0, 0, 0, 0, 0 };
static const uint32_t EVUTIL_ISLOWER_TABLE[8] = { 0, 0, 0, 0x7fffffe, 0, 0, 0, 0 };
/* Upper-casing and lowercasing tables to map characters to upper/lowercase
 * equivalents. */
static const unsigned char EVUTIL_TOUPPER_TABLE[256] = {
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
  96,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
  80,81,82,83,84,85,86,87,88,89,90,123,124,125,126,127,
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
  176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
  192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
  208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
  224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
  240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
};
static const unsigned char EVUTIL_TOLOWER_TABLE[256] = {
  0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
  16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
  32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
  48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
  64,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,91,92,93,94,95,
  96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
  176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
  192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
  208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
  224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
  240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
};


#define IMPL_CTYPE_FN(name)						\
	int EVUTIL_##name##_(char c) {					\
		uint8_t u = c;					\
		return !!(EVUTIL_##name##_TABLE[(u >> 5) & 7] & (1 << (u & 31))); \
	}
IMPL_CTYPE_FN(ISALPHA)
IMPL_CTYPE_FN(ISALNUM)
IMPL_CTYPE_FN(ISSPACE)
IMPL_CTYPE_FN(ISDIGIT)
IMPL_CTYPE_FN(ISXDIGIT)
IMPL_CTYPE_FN(ISPRINT)
IMPL_CTYPE_FN(ISLOWER)
IMPL_CTYPE_FN(ISUPPER)

/* We sometimes need to know whether we have an ipv4 address and whether we
   have an ipv6 address. If 'have_checked_interfaces', then we've already done
   the test.  If 'had_ipv4_address', then it turns out we had an ipv4 address.
   If 'had_ipv6_address', then it turns out we had an ipv6 address.   These are
   set by evutil_check_interfaces. */
static int have_checked_interfaces, had_ipv4_address, had_ipv6_address;

/* True iff the IPv4 address 'addr', in host order, is a class D
 * (multiclass) address.  */
static __inline int evutil_v4addr_is_classd(uint32_t addr)
{ return ((addr>>24) & 0xf0) == 0xe0; }

/* True iff the IPv4 address 'addr', in host order, is link-local
 * 169.254.0.0/16 (RFC3927) */
static __inline int evutil_v4addr_is_linklocal(uint32_t addr)
{ return ((addr & 0xffff0000U) == 0xa9fe0000U); }

/* True iff the IPv4 address 'addr', in host order, is in 127.0.0.0/8 */
static __inline int evutil_v4addr_is_localhost(uint32_t addr)
{ return addr>>24 == 127; }


static int
parse_numeric_servname(const char *servname)
{
	int n;
	char *endptr=NULL;
	n = (int) strtol(servname, &endptr, 10);
	if (n>=0 && n <= 65535 && servname[0] && endptr && !endptr[0])
		return n;
	else
		return -1;
}


/* Return a string corresponding to a protocol number that we can pass to
 * getservyname.  */
static const char *
evutil_unparse_protoname(int proto)
{
	switch (proto) {
	case 0:
		return NULL;
	case IPPROTO_TCP:
		return "tcp";
	case IPPROTO_UDP:
		return "udp";
#ifdef IPPROTO_SCTP
	case IPPROTO_SCTP:
		return "sctp";
#endif
	default:
#ifdef EVENT__HAVE_GETPROTOBYNUMBER
		{
			struct protoent *ent = getprotobynumber(proto);
			if (ent)
				return ent->p_name;
		}
#endif
		return NULL;
	}
}


#define EVUTIL_AI_NUMERICSERV 0x8000

/** Parse a service name in 'servname', which can be a decimal port.
 * Return the port number, or -1 on error.
 */
static int
evutil_parse_servname(const char *servname, const char *protocol,
    const struct addrinfo *hints)
{
	int n = parse_numeric_servname(servname);
	if (n>=0)
		return n;
#if defined(EVENT__HAVE_GETSERVBYNAME) || defined(_WIN32)
	if (!(hints->ai_flags & EVUTIL_AI_NUMERICSERV)) {
		struct servent *ent = getservbyname(servname, protocol);
		if (ent) {
			return ntohs(ent->s_port);
		}
	}
#endif
	return -1;
}



void
evutil_freeaddrinfo(struct addrinfo *ai)
{
#ifdef EVENT__HAVE_GETADDRINFO
	if (!(ai->ai_flags & EVUTIL_AI_LIBEVENT_ALLOCATED)) {
		freeaddrinfo(ai);
		return;
	}
#endif
	while (ai) {
		struct addrinfo *next = ai->ai_next;
		if (ai->ai_canonname)
			free(ai->ai_canonname);
		free(ai);
		ai = next;
	}
}

#define EVUTIL_AI_LIBEVENT_ALLOCATED 0x80000000

/* Helper: construct a new addrinfo containing the socket address in
 * 'sa', which must be a sockaddr_in or a sockaddr_in6.  Take the
 * socktype and protocol info from hints.  If they weren't set, then
 * allocate both a TCP and a UDP addrinfo.
 */
struct addrinfo *
evutil_new_addrinfo_(struct sockaddr *sa, ev_socklen_t socklen,
    const struct addrinfo *hints)
{
	struct addrinfo *res;
	//EVUTIL_ASSERT(hints);

	if (hints->ai_socktype == 0 && hints->ai_protocol == 0) {
		/* Indecisive user! Give them a UDP and a TCP. */
		struct addrinfo *r1, *r2;
		struct addrinfo tmp;
		memcpy(&tmp, hints, sizeof(tmp));
		tmp.ai_socktype = SOCK_STREAM; tmp.ai_protocol = IPPROTO_TCP;
		r1 = evutil_new_addrinfo_(sa, socklen, &tmp);
		if (!r1)
			return NULL;
		tmp.ai_socktype = SOCK_DGRAM; tmp.ai_protocol = IPPROTO_UDP;
		r2 = evutil_new_addrinfo_(sa, socklen, &tmp);
		if (!r2) {
			evutil_freeaddrinfo(r1);
			return NULL;
		}
		r1->ai_next = r2;
		return r1;
	}

	/* We're going to allocate extra space to hold the sockaddr. */
	res = calloc(1,sizeof(struct addrinfo)+socklen);
	if (!res)
		return NULL;
	res->ai_addr = (struct sockaddr*)
	    (((char*)res) + sizeof(struct addrinfo));
	memcpy(res->ai_addr, sa, socklen);
	res->ai_addrlen = socklen;
	res->ai_family = sa->sa_family; /* Same or not? XXX */
	res->ai_flags = EVUTIL_AI_LIBEVENT_ALLOCATED;
	res->ai_socktype = hints->ai_socktype;
	res->ai_protocol = hints->ai_protocol;

	return res;
}

static void
evutil_getaddrinfo_infer_protocols(struct addrinfo *hints)
{
	/* If we can guess the protocol from the socktype, do so. */
	if (!hints->ai_protocol && hints->ai_socktype) {
		if (hints->ai_socktype == SOCK_DGRAM)
			hints->ai_protocol = IPPROTO_UDP;
		else if (hints->ai_socktype == SOCK_STREAM)
			hints->ai_protocol = IPPROTO_TCP;
	}

	/* Set the socktype if it isn't set. */
	if (!hints->ai_socktype && hints->ai_protocol) {
		if (hints->ai_protocol == IPPROTO_UDP)
			hints->ai_socktype = SOCK_DGRAM;
		else if (hints->ai_protocol == IPPROTO_TCP)
			hints->ai_socktype = SOCK_STREAM;
#ifdef IPPROTO_SCTP
		else if (hints->ai_protocol == IPPROTO_SCTP)
			hints->ai_socktype = SOCK_STREAM;
#endif
	}
}


/* Append the addrinfo 'append' to the end of 'first', and return the start of
 * the list.  Either element can be NULL, in which case we return the element
 * that is not NULL. */
struct addrinfo *
evutil_addrinfo_append_(struct addrinfo *first,
    struct addrinfo *append)
{
	struct addrinfo *ai = first;
	if (!ai)
		return append;
	while (ai->ai_next)
		ai = ai->ai_next;
	ai->ai_next = append;

	return first;
}


#define EVUTIL_AI_NUMERICHOST 0x4000
#define EVUTIL_AI_PASSIVE 0x1000

/** Implements the part of looking up hosts by name that's common to both
 * the blocking and nonblocking resolver:
 *   - Adjust 'hints' to have a reasonable socktype and protocol.
 *   - Look up the port based on 'servname', and store it in *portnum,
 *   - Handle the nodename==NULL case
 *   - Handle some invalid arguments cases.
 *   - Handle the cases where nodename is an IPv4 or IPv6 address.
 *
 * If we need the resolver to look up the hostname, we return
 * EVUTIL_EAI_NEED_RESOLVE.  Otherwise, we can completely implement
 * getaddrinfo: we return 0 or an appropriate EVUTIL_EAI_* error, and
 * set *res as getaddrinfo would.
 */
int
evutil_getaddrinfo_common_(const char *nodename, const char *servname,
    struct addrinfo *hints, struct addrinfo **res, int *portnum)
{
	int port = 0;
	const char *pname;

	if (nodename == NULL && servname == NULL)
		return EVUTIL_EAI_NONAME;

	/* We only understand 3 families */
	if (hints->ai_family != PF_UNSPEC && hints->ai_family != PF_INET &&
	    hints->ai_family != PF_INET6)
		return EVUTIL_EAI_FAMILY;

	evutil_getaddrinfo_infer_protocols(hints);

	/* Look up the port number and protocol, if possible. */
	pname = evutil_unparse_protoname(hints->ai_protocol);
	if (servname) {
		/* XXXX We could look at the protocol we got back from
		 * getservbyname, but it doesn't seem too useful. */
		port = evutil_parse_servname(servname, pname, hints);
		if (port < 0) {
			return EVUTIL_EAI_NONAME;
		}
	}

	/* If we have no node name, then we're supposed to bind to 'any' and
	 * connect to localhost. */
	if (nodename == NULL) {
		struct addrinfo *res4=NULL, *res6=NULL;
		if (hints->ai_family != PF_INET) { /* INET6 or UNSPEC. */
			struct sockaddr_in6 sin6;
			memset(&sin6, 0, sizeof(sin6));
			sin6.sin6_family = AF_INET6;
			sin6.sin6_port = htons(port);
			if (hints->ai_flags & EVUTIL_AI_PASSIVE) {
				/* Bind to :: */
			} else {
				/* connect to ::1 */
				sin6.sin6_addr.s6_addr[15] = 1;
			}
			res6 = evutil_new_addrinfo_((struct sockaddr*)&sin6,
			    sizeof(sin6), hints);
			if (!res6)
				return EVUTIL_EAI_MEMORY;
		}

		if (hints->ai_family != PF_INET6) { /* INET or UNSPEC */
			struct sockaddr_in sin;
			memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_port = htons(port);
			if (hints->ai_flags & EVUTIL_AI_PASSIVE) {
				/* Bind to 0.0.0.0 */
			} else {
				/* connect to 127.0.0.1 */
				sin.sin_addr.s_addr = htonl(0x7f000001);
			}
			res4 = evutil_new_addrinfo_((struct sockaddr*)&sin,
			    sizeof(sin), hints);
			if (!res4) {
				if (res6)
					evutil_freeaddrinfo(res6);
				return EVUTIL_EAI_MEMORY;
			}
		}
		*res = evutil_addrinfo_append_(res4, res6);
		return 0;
	}

	/* If we can, we should try to parse the hostname without resolving
	 * it. */
	/* Try ipv6. */
	if (hints->ai_family == PF_INET6 || hints->ai_family == PF_UNSPEC) {
		struct sockaddr_in6 sin6;
		memset(&sin6, 0, sizeof(sin6));
		if (1==evutil_inet_pton(AF_INET6, nodename, &sin6.sin6_addr)) {
			/* Got an ipv6 address. */
			sin6.sin6_family = AF_INET6;
			sin6.sin6_port = htons(port);
			*res = evutil_new_addrinfo_((struct sockaddr*)&sin6,
			    sizeof(sin6), hints);
			if (!*res)
				return EVUTIL_EAI_MEMORY;
			return 0;
		}
	}

	/* Try ipv4. */
	if (hints->ai_family == PF_INET || hints->ai_family == PF_UNSPEC) {
		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		if (1==evutil_inet_pton(AF_INET, nodename, &sin.sin_addr)) {
			/* Got an ipv4 address. */
			sin.sin_family = AF_INET;
			sin.sin_port = htons(port);
			*res = evutil_new_addrinfo_((struct sockaddr*)&sin,
			    sizeof(sin), hints);
			if (!*res)
				return EVUTIL_EAI_MEMORY;
			return 0;
		}
	}


	/* If we have reached this point, we definitely need to do a DNS
	 * lookup. */
	if ((hints->ai_flags & EVUTIL_AI_NUMERICHOST)) {
		/* If we're not allowed to do one, then say so. */
		return EVUTIL_EAI_NONAME;
	}
	*portnum = port;
	return EVUTIL_EAI_NEED_RESOLVE;
}


int
evutil_v4addr_is_local_(const struct in_addr *in)
{
	const uint32_t addr = ntohl(in->s_addr);
	return addr == INADDR_ANY ||
		evutil_v4addr_is_localhost(addr) ||
		evutil_v4addr_is_linklocal(addr) ||
		evutil_v4addr_is_classd(addr);
}
int
evutil_v6addr_is_local_(const struct in6_addr *in)
{
	static const char ZEROES[] =
		"\x00\x00\x00\x00\x00\x00\x00\x00"
		"\x00\x00\x00\x00\x00\x00\x00\x00";

	const unsigned char *addr = (const unsigned char *)in->s6_addr;
	return !memcmp(addr, ZEROES, 8) ||
		((addr[0] & 0xfe) == 0xfc) ||
		(addr[0] == 0xfe && (addr[1] & 0xc0) == 0x80) ||
		(addr[0] == 0xfe && (addr[1] & 0xc0) == 0xc0) ||
		(addr[0] == 0xff);
}


#define EVUTIL_AI_CANONNAME 0x2000

/* Helper for systems with no getaddrinfo(): make one or more addrinfos out of
 * a struct hostent.
 */
static struct addrinfo *
addrinfo_from_hostent(const struct hostent *ent,
    int port, const struct addrinfo *hints)
{
	int i;
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
	struct sockaddr *sa;
	int socklen;
	struct addrinfo *res=NULL, *ai;
	void *addrp;

	if (ent->h_addrtype == PF_INET) {
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sa = (struct sockaddr *)&sin;
		socklen = sizeof(struct sockaddr_in);
		addrp = &sin.sin_addr;
		if (ent->h_length != sizeof(sin.sin_addr)) {
			printf("Weird h_length from gethostbyname");
			return NULL;
		}
	} else if (ent->h_addrtype == PF_INET6) {
		memset(&sin6, 0, sizeof(sin6));
		sin6.sin6_family = AF_INET6;
		sin6.sin6_port = htons(port);
		sa = (struct sockaddr *)&sin6;
		socklen = sizeof(struct sockaddr_in6);
		addrp = &sin6.sin6_addr;
		if (ent->h_length != sizeof(sin6.sin6_addr)) {
			printf("Weird h_length from gethostbyname");
			return NULL;
		}
	} else
		return NULL;

	for (i = 0; ent->h_addr_list[i]; ++i) {
		memcpy(addrp, ent->h_addr_list[i], ent->h_length);
		ai = evutil_new_addrinfo_(sa, socklen, hints);
		if (!ai) {
			evutil_freeaddrinfo(res);
			return NULL;
		}
		res = evutil_addrinfo_append_(res, ai);
	}

	if (res && ((hints->ai_flags & EVUTIL_AI_CANONNAME) && ent->h_name)) {
		res->ai_canonname = strdup(ent->h_name);
		if (res->ai_canonname == NULL) {
			evutil_freeaddrinfo(res);
			return NULL;
		}
	}

	return res;
}


int
evutil_inet_pton(int af, const char *src, void *dst)
{
#if defined(EVENT__HAVE_INET_PTON) && !defined(USE_INTERNAL_PTON)
	return inet_pton(af, src, dst);
#else
	if (af == AF_INET) {
		unsigned a,b,c,d;
		char more;
		struct in_addr *addr = dst;
		if (sscanf(src, "%u.%u.%u.%u%c", &a,&b,&c,&d,&more) != 4)
			return 0;
		if (a > 255) return 0;
		if (b > 255) return 0;
		if (c > 255) return 0;
		if (d > 255) return 0;
		addr->s_addr = htonl((a<<24) | (b<<16) | (c<<8) | d);
		return 1;
#ifdef AF_INET6
	} else if (af == AF_INET6) {
		struct in6_addr *out = dst;
		uint16_t words[8];
		int gapPos = -1, i, setWords=0;
		const char *dot = strchr(src, '.');
		const char *eow; /* end of words. */
		if (dot == src)
			return 0;
		else if (!dot)
			eow = src+strlen(src);
		else {
			unsigned byte1,byte2,byte3,byte4;
			char more;
			for (eow = dot-1; eow >= src && EVUTIL_ISDIGIT_(*eow); --eow)
				;
			++eow;

			/* We use "scanf" because some platform inet_aton()s are too lax
			 * about IPv4 addresses of the form "1.2.3" */
			if (sscanf(eow, "%u.%u.%u.%u%c",
					   &byte1,&byte2,&byte3,&byte4,&more) != 4)
				return 0;

			if (byte1 > 255 ||
			    byte2 > 255 ||
			    byte3 > 255 ||
			    byte4 > 255)
				return 0;

			words[6] = (byte1<<8) | byte2;
			words[7] = (byte3<<8) | byte4;
			setWords += 2;
		}

		i = 0;
		while (src < eow) {
			if (i > 7)
				return 0;
			if (EVUTIL_ISXDIGIT_(*src)) {
				char *next;
				long r = strtol(src, &next, 16);
				if (next > 4+src)
					return 0;
				if (next == src)
					return 0;
				if (r<0 || r>65536)
					return 0;

				words[i++] = (uint16_t)r;
				setWords++;
				src = next;
				if (*src != ':' && src != eow)
					return 0;
				++src;
			} else if (*src == ':' && i > 0 && gapPos==-1) {
				gapPos = i;
				++src;
			} else if (*src == ':' && i == 0 && src[1] == ':' && gapPos==-1) {
				gapPos = i;
				src += 2;
			} else {
				return 0;
			}
		}

		if (setWords > 8 ||
			(setWords == 8 && gapPos != -1) ||
			(setWords < 8 && gapPos == -1))
			return 0;

		if (gapPos >= 0) {
			int nToMove = setWords - (dot ? 2 : 0) - gapPos;
			int gapLen = 8 - setWords;
			/* assert(nToMove >= 0); */
			if (nToMove < 0)
				return -1; /* should be impossible */
			memmove(&words[gapPos+gapLen], &words[gapPos],
					sizeof(uint16_t)*nToMove);
			memset(&words[gapPos], 0, sizeof(uint16_t)*gapLen);
		}
		for (i = 0; i < 8; ++i) {
			out->s6_addr[2*i  ] = words[i] >> 8;
			out->s6_addr[2*i+1] = words[i] & 0xff;
		}

		return 1;
#endif
	} else {
		return -1;
	}
#endif
}


static void
evutil_found_ifaddr(const struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		const struct sockaddr_in *sin = (struct sockaddr_in *)sa;
		if (!evutil_v4addr_is_local_(&sin->sin_addr)) {
			printf(("Detected an IPv4 interface"));
			had_ipv4_address = 1;
		}
	} else if (sa->sa_family == AF_INET6) {
		const struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
		if (!evutil_v6addr_is_local_(&sin6->sin6_addr)) {
			printf(("Detected an IPv6 interface"));
			had_ipv6_address = 1;
		}
	}
}


/* Test whether we have an ipv4 interface and an ipv6 interface.  Return 0 if
 * the test seemed successful. */
static int
evutil_check_interfaces(void)
{
	evutil_socket_t fd = -1;
	struct sockaddr_in sin, sin_out;
	struct sockaddr_in6 sin6, sin6_out;
	ev_socklen_t sin_out_len = sizeof(sin_out);
	ev_socklen_t sin6_out_len = sizeof(sin6_out);
	int r;
	if (have_checked_interfaces)
		return 0;

	/* From this point on we have done the ipv4/ipv6 interface check */
	have_checked_interfaces = 1;

	//if (evutil_check_ifaddrs() == 0) { //apa---
		/* Use a nice sane interface, if this system has one. *///apa---
	//	return 0;//apa---
	//}

	/* Ugh. There was no nice sane interface.  So to check whether we have
	 * an interface open for a given protocol, will try to make a UDP
	 * 'connection' to a remote host on the internet.  We don't actually
	 * use it, so the address doesn't matter, but we want to pick one that
	 * keep us from using a host- or link-local interface. */
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(53);
	r = evutil_inet_pton(AF_INET, "18.244.0.188", &sin.sin_addr);
	//EVUTIL_ASSERT(r);

	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_port = htons(53);
	r = evutil_inet_pton(AF_INET6, "2001:4860:b002::68", &sin6.sin6_addr);
	//EVUTIL_ASSERT(r);

	memset(&sin_out, 0, sizeof(sin_out));
	memset(&sin6_out, 0, sizeof(sin6_out));

	/* XXX some errnos mean 'no address'; some mean 'not enough sockets'. */
	if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) >= 0 &&
	    connect(fd, (struct sockaddr*)&sin, sizeof(sin)) == 0 &&
	    getsockname(fd, (struct sockaddr*)&sin_out, &sin_out_len) == 0) {
		/* We might have an IPv4 interface. */
		evutil_found_ifaddr((struct sockaddr*) &sin_out);
	}
	if (fd >= 0)
		evutil_closesocket(fd);

	if ((fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) >= 0 &&
	    connect(fd, (struct sockaddr*)&sin6, sizeof(sin6)) == 0 &&
	    getsockname(fd, (struct sockaddr*)&sin6_out, &sin6_out_len) == 0) {
		/* We might have an IPv6 interface. */
		evutil_found_ifaddr((struct sockaddr*) &sin6_out);
	}

	if (fd >= 0)
		evutil_closesocket(fd);

	return 0;
}

/* If the EVUTIL_AI_ADDRCONFIG flag is set on hints->ai_flags, and
 * hints->ai_family is PF_UNSPEC, then revise the value of hints->ai_family so
 * that we'll only get addresses we could maybe connect to.
 */
void
evutil_adjust_hints_for_addrconfig_(struct addrinfo *hints)
{
	if (!(hints->ai_flags & EVUTIL_AI_ADDRCONFIG))
		return;
	if (hints->ai_family != PF_UNSPEC)
		return;
	evutil_check_interfaces();
	if (had_ipv4_address && !had_ipv6_address) {
		hints->ai_family = PF_INET;
	} else if (!had_ipv4_address && had_ipv6_address) {
		hints->ai_family = PF_INET6;
	}
}

int
fake_getaddrinfo(const char *nodename, const char *servname,
    const struct addrinfo *hints_in, struct addrinfo **res)
{
#ifdef USE_NATIVE_GETADDRINFO
	struct addrinfo hints;
	int portnum=-1, need_np_hack, err;

	if (hints_in) {
		memcpy(&hints, hints_in, sizeof(hints));
	} else {
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = PF_UNSPEC;
	}

#ifndef AI_ADDRCONFIG
	/* Not every system has AI_ADDRCONFIG, so fake it. */
	if (hints.ai_family == PF_UNSPEC &&
	    (hints.ai_flags & EVUTIL_AI_ADDRCONFIG)) {
		evutil_adjust_hints_for_addrconfig_(&hints);
	}
#endif

#ifndef AI_NUMERICSERV
	/* Not every system has AI_NUMERICSERV, so fake it. */
	if (hints.ai_flags & EVUTIL_AI_NUMERICSERV) {
		if (servname && parse_numeric_servname(servname)<0)
			return EVUTIL_EAI_NONAME;
	}
#endif

	/* Enough operating systems handle enough common non-resolve
	 * cases here weirdly enough that we are better off just
	 * overriding them.  For example:
	 *
	 * - Windows doesn't like to infer the protocol from the
	 *   socket type, or fill in socket or protocol types much at
	 *   all.  It also seems to do its own broken implicit
	 *   always-on version of AI_ADDRCONFIG that keeps it from
	 *   ever resolving even a literal IPv6 address when
	 *   ai_addrtype is PF_UNSPEC.
	 */
#ifdef _WIN32
	{
		int tmp_port;
		err = evutil_getaddrinfo_common_(nodename,servname,&hints,
		    res, &tmp_port);
		if (err == 0 ||
		    err == EVUTIL_EAI_MEMORY ||
		    err == EVUTIL_EAI_NONAME)
			return err;
		/* If we make it here, the system getaddrinfo can
		 * have a crack at it. */
	}
#endif

	/* See documentation for need_numeric_port_hack above.*/
	need_np_hack = need_numeric_port_hack() && servname && !hints.ai_socktype
	    && ((portnum=parse_numeric_servname(servname)) >= 0);
	if (need_np_hack) {
		if (!nodename)
			return evutil_getaddrinfo_common_(
				NULL,servname,&hints, res, &portnum);
		servname = NULL;
	}

	if (need_socktype_protocol_hack()) {
		evutil_getaddrinfo_infer_protocols(&hints);
	}

	/* Make sure that we didn't actually steal any AI_FLAGS values that
	 * the system is using.  (This is a constant expression, and should ge
	 * optimized out.)
	 *
	 * XXXX Turn this into a compile-time failure rather than a run-time
	 * failure.
	 */
	//EVUTIL_ASSERT((ALL_NONNATIVE_AI_FLAGS & ALL_NATIVE_AI_FLAGS) == 0);

	/* Clear any flags that only libevent understands. */
	hints.ai_flags &= ~ALL_NONNATIVE_AI_FLAGS;

	err = getaddrinfo(nodename, servname, &hints, res);
	if (need_np_hack)
		apply_numeric_port_hack(portnum, res);

	if (need_socktype_protocol_hack()) {
		if (apply_socktype_protocol_hack(*res) < 0) {
			evutil_freeaddrinfo(*res);
			*res = NULL;
			return EVUTIL_EAI_MEMORY;
		}
	}
	return err;
#else
	int port=0, err;
	struct hostent *ent = NULL;
	struct addrinfo hints;

	if (hints_in) {
		memcpy(&hints, hints_in, sizeof(hints));
	} else {
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = PF_UNSPEC;
	}

	evutil_adjust_hints_for_addrconfig_(&hints);

	err = evutil_getaddrinfo_common_(nodename, servname, &hints, res, &port);
	if (err != EVUTIL_EAI_NEED_RESOLVE) {
		/* We either succeeded or failed.  No need to continue */
		return err;
	}

	err = 0;
	/* Use any of the various gethostbyname_r variants as available. */
	{
#ifdef EVENT__HAVE_GETHOSTBYNAME_R_6_ARG
		/* This one is what glibc provides. */
		char buf[2048];
		struct hostent hostent;
		int r;
		r = gethostbyname_r(nodename, &hostent, buf, sizeof(buf), &ent,
		    &err);
#elif defined(EVENT__HAVE_GETHOSTBYNAME_R_5_ARG)
		char buf[2048];
		struct hostent hostent;
		ent = gethostbyname_r(nodename, &hostent, buf, sizeof(buf),
		    &err);
#elif defined(EVENT__HAVE_GETHOSTBYNAME_R_3_ARG)
		struct hostent_data data;
		struct hostent hostent;
		memset(&data, 0, sizeof(data));
		err = gethostbyname_r(nodename, &hostent, &data);
		ent = err ? NULL : &hostent;
#else
		/* fall back to gethostbyname. */
		/* XXXX This needs a lock everywhere but Windows. */
		ent = gethostbyname(nodename);
#ifdef _WIN32
		err = WSAGetLastError();
#else
		err = h_errno;
#endif
#endif

		/* Now we have either ent or err set. */
		if (!ent) {
			/* XXX is this right for windows ? */
			switch (err) {
			case TRY_AGAIN:
				return EVUTIL_EAI_AGAIN;
			case NO_RECOVERY:
			default:
				return EVUTIL_EAI_FAIL;
			case HOST_NOT_FOUND:
				return EVUTIL_EAI_NONAME;
			case NO_ADDRESS:
#if NO_DATA != NO_ADDRESS
			case NO_DATA:
#endif
				return EVUTIL_EAI_NODATA;
			}
		}

		if (ent->h_addrtype != hints.ai_family &&
		    hints.ai_family != PF_UNSPEC) {
			/* This wasn't the type we were hoping for.  Too bad
			 * we never had a chance to ask gethostbyname for what
			 * we wanted. */
			return EVUTIL_EAI_NONAME;
		}

		/* Make sure we got _some_ answers. */
		if (ent->h_length == 0)
			return EVUTIL_EAI_NODATA;

		/* If we got an address type we don't know how to make a
		   sockaddr for, give up. */
		if (ent->h_addrtype != PF_INET && ent->h_addrtype != PF_INET6)
			return EVUTIL_EAI_FAMILY;

		*res = addrinfo_from_hostent(ent, port, &hints);
		if (! *res)
			return EVUTIL_EAI_MEMORY;
	}

	return 0;
#endif
}

void
fake_freeaddrinfo(struct addrinfo *ai)
{
#ifdef EVENT__HAVE_GETADDRINFO
	if (!(ai->ai_flags & EVUTIL_AI_LIBEVENT_ALLOCATED)) {
		freeaddrinfo(ai);
		return;
	}
#endif
	while (ai) {
		struct addrinfo *next = ai->ai_next;
		if (ai->ai_canonname)
			free(ai->ai_canonname);
		free(ai);
		ai = next;
	}
}
