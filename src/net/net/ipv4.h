#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>

#include "net_cfg.h"
#include "net_err.h"
#include "netif.h"
#include "pktbuf.h"

#define IPV4_ADDR_SIZE 4

#define NET_VERSION_IPV4 4
#define NET_IP_DEFAULT_TTL 64

#pragma pack(1)

typedef struct _ipv4_hdr_t {
  union {
    struct {
#if NET_ENDIAN_LITTLE
      uint16_t shdr : 4;
      uint16_t version : 4;
      uint16_t tos : 8;
#else
      uint16_t version : 4;
      uint16_t shdr : 4;
      uint16_t tos : 8;
#endif
    };
    uint16_t shdr_all;
  };

  uint16_t total_len;
  uint16_t id;
  union {
#if NET_ENDIAN_LITTLE
    struct {
      uint16_t frag_offset : 13;
      uint16_t more : 1;
      uint16_t disable : 1;
      uint16_t reserve : 1;
    };
#else
    struct {
      uint16_t reserve : 1;
      uint16_t disable : 1;
      uint16_t more : 1;
      uint16_t frag_offset : 13;
    };
#endif
    uint16_t frag_all;
  };

  uint8_t ttl;
  uint8_t protocol;
  uint16_t hdr_checksum;
  uint8_t src_ip[IPV4_ADDR_SIZE];
  uint8_t dest_ip[IPV4_ADDR_SIZE];
} ipv4_hdr_t;

typedef struct _ipv4_pkt_t {
  ipv4_hdr_t hdr;
  uint8_t data[1];
} ipv4_pkt_t;

#pragma pack()

typedef struct _ip_frag_t {
  ipaddr_t ip;
  uint16_t id;
  int tmo;
  nlist_t buf_list;
  nlist_node_t node;
} ip_frag_t;

typedef struct _rentry_t {
  ipaddr_t net;
  ipaddr_t mask;
  int mask_1_cnt;
  ipaddr_t next_hop;
  netif_t *netif;

  nlist_node_t node;
} rentry_t;

void rt_init(void);
void rt_add(ipaddr_t *net, ipaddr_t *mask, ipaddr_t *next_hop, netif_t *netif);
void rt_remove(ipaddr_t *net, ipaddr_t *mask);
rentry_t *rt_find(ipaddr_t *ip);

net_err_t ipv4_init(void);

net_err_t ipv4_in(netif_t *netif, pktbuf_t *buf);
net_err_t ipv4_out(uint8_t protocol, ipaddr_t *dest, ipaddr_t *src,
                   pktbuf_t *buf);

static inline int ipv4_hdr_size(ipv4_pkt_t *pkt) { return pkt->hdr.shdr * 4; }

static inline int ipv4_set_hdr_size(ipv4_pkt_t *pkt, int len) {
  return pkt->hdr.shdr = len / 4;
}

#endif