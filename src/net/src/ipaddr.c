#include "ipaddr.h"

#include "net_err.h"

void ipaddr_set_any(ipaddr_t* ip) {
  ip->type = IPADDR_V4;
  ip->q_addr = 0;
}

net_err_t ipaddr_from_str(ipaddr_t* dest, const char* str) {
  if (!dest || !str) {
    return NET_ERR_PARAM;
  }

  dest->type = IPADDR_V4;
  dest->q_addr = 0;

  uint8_t* p = dest->a_addr;
  uint8_t sub_addr = 0;
  char c;
  while ((c = *str++) != '\0') {
    if (c >= '0' && c <= '9') {
      sub_addr = sub_addr * 10 + c - '0';
    } else if (c == '.') {
      *p++ = sub_addr;
      sub_addr = 0;
    } else {
      return NET_ERR_PARAM;
    }
  }

  *p = sub_addr;
  return NET_ERR_OK;
}

void ipaddr_copy(ipaddr_t* dest, ipaddr_t* src) {
  if (!dest || !src) {
    return;
  }
  dest->type = src->type;
  dest->q_addr = src->q_addr;
}

ipaddr_t* ipaddr_get_any() {
  static const ipaddr_t ipaddr_any = {
      .type = IPADDR_V4,
      .q_addr = 0,
  };

  return (ipaddr_t*)&ipaddr_any;
}

int ipaddr_is_equal(const ipaddr_t* ipaddr1, const ipaddr_t* ipaddr2) {
  return ipaddr1->q_addr == ipaddr2->q_addr;
}

void ipaddr_to_buf(const ipaddr_t* src, uint8_t* in_buf) {
  *(uint32_t*)in_buf = src->q_addr;
}

void ipaddr_from_buf(ipaddr_t* dest, uint8_t* ip_buf) {
  dest->q_addr = *(uint32_t*)ip_buf;
  dest->type = IPADDR_V4;
}

int ipaddr_is_local_broadcast(const ipaddr_t* ipaddr) {
  return ipaddr->q_addr == 0xFFFFFFFF;
}

ipaddr_t ipaddr_get_host(const ipaddr_t* ipaddr, const ipaddr_t* netmask) {
  ipaddr_t host_id;

  host_id.q_addr = ipaddr->q_addr & ~netmask->q_addr;

  return host_id;
}

int ipaddr_is_direct_broadcast(const ipaddr_t* ipaddr,
                               const ipaddr_t* netmask) {
  ipaddr_t host_id = ipaddr_get_host(ipaddr, netmask);

  return host_id.q_addr == (0xFFFFFFFF & ~netmask->q_addr);
}