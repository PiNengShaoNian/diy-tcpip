#ifndef ICMPV4_H
#define ICMPV4_H

#include <stdint.h>

#include "ipaddr.h"
#include "net_err.h"
#include "pktbuf.h"

#pragma pack(1)

typedef struct _icmpv4_hdr_t {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
} icmpv4_hdr_t;

typedef struct _icmpv4_pkt_t {
  icmpv4_hdr_t hdr;
  union {
    uint32_t reserve;
  };
  uint8_t data[1];
} icmpv4_pkt_t;

#pragma pack()

net_err_t icmpv4_init(void);
net_err_t icmpv4_in(ipaddr_t *src_ip, ipaddr_t *netif_ip, pktbuf_t *buf);

#endif