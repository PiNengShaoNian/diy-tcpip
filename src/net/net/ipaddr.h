#ifndef IPADDR_H
#define IPADDR_H

#include <stdint.h>

#include "net_err.h"

#define IPV4_ADDR_SIZE 4

typedef struct _ipaddr_t {
  enum {
    IPADDR_V4,
  } type;
  union {
    uint32_t q_addr;
    uint8_t a_addr[IPV4_ADDR_SIZE];
  };
} ipaddr_t;

void ipaddr_set_any(ipaddr_t* ip);
net_err_t ipaddr_from_str(ipaddr_t* dest, const char* str);
void ipaddr_copy(ipaddr_t* dest, ipaddr_t* src);
ipaddr_t* ipaddr_get_any();
int ipaddr_is_equal(const ipaddr_t* ipaddr1, const ipaddr_t* ipaddr2);
void ipaddr_to_buf(const ipaddr_t* src, uint8_t* in_buf);
void ipaddr_from_buf(ipaddr_t* dest, const uint8_t* ip_buf);
int ipaddr_is_local_broadcast(const ipaddr_t* ipaddr);
int ipaddr_is_direct_broadcast(const ipaddr_t* ipaddr, const ipaddr_t* netmask);
int ipaddr_is_match(const ipaddr_t* dest_ip, const ipaddr_t* src_ip,
                    const ipaddr_t* netmask);
int ipaddr_is_any(ipaddr_t* ip);
ipaddr_t ipaddr_get_host(const ipaddr_t* ipaddr, const ipaddr_t* netmask);
ipaddr_t ipaddr_get_net(const ipaddr_t* ipaddr, const ipaddr_t* netmask);
int ipaddr_1_cnt(ipaddr_t* ip);

#endif