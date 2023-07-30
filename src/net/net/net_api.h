#ifndef NET_API_H
#define NET_API_H

#include "socket.h"
#include "tools.h"

#define x_htons(v) swap_u16(v)
#define x_ntohs(v) swap_u16(v)
#define x_htonl(v) swap_u32(v)
#define x_ntohl(v) swap_u32(v)

#undef htons
#define htons(v) x_htons(v)

#undef ntohs
#define ntohs(v) x_ntohs(v)

#undef htonl
#define htonl(v) x_htonl(v)

#undef ntohl
#define ntohl(v) x_ntohl(v)

char *x_inet_ntoa(struct x_in_addr in);
uint32_t x_inet_addr(const char *str);
int x_inet_pton(int family, const char *strptr, void *addrptr);
const char *x_inet_ntop(int family, const void *addrptr, char *strptr,
                        size_t len);

#define inet_ntoa(in) x_inet_ntoa(in)
#define inet_addr(str) x_inet_addr(str)
#define inet_pton(family, strptr, addrptr) x_inet_pton(family, strptr, addrptr)
#define x_inet_ntop(family, addrptr, strptr, len) \
  x_inet_ntop(family, addrptr, strptr, len)

#define sockaddr_in x_sockaddr_in

#endif