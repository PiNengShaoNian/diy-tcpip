#ifndef ARP_H
#define ARP_H

#include "ether.h"
#include "ipaddr.h"
#include "net_err.h"
#include "pktbuf.h"

typedef struct _arp_entry_t {
  uint8_t ipaddr[IPV4_ADDR_SIZE];
  uint8_t hwaddr[ETHER_HWA_SIZE];

  enum {
    NET_ARP_FREE,
    NET_ARP_WAITING,
    NET_ARP_RESOLVED,
  } state;

  nlist_node_t node;
  nlist_t buf_list;
  netif_t *netif;
} arp_entry_t;

net_err_t arp_init(void);

#endif