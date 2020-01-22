#ifndef GETADDRINFO_H
#define GETADDRINFO_H

int fake_getaddrinfo(const char *nodename, const char *servname,
    const struct addrinfo *hints_in, struct addrinfo **res);

void fake_freeaddrinfo(struct addrinfo *ai);

#endif