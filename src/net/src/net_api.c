#include <WinSock2.h>

#include "net_api.h"


char *x_inet_ntoa(struct in_addr in) { return (char *)0; }

uint32_t x_inet_addr(const char *str) { return 0; }

int x_inet_pton(int family, const char *strptr, void *addrptr) { return 0; }

const char *x_inet_ntop(int family, const void *addrptr, char *strptr,
                        size_t len) {
  return (const char *)0;
}