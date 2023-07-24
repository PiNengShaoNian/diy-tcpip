#ifndef NETIF_PCAP_H
#define NETIF_PCAP_H

#include <stdint.h>

#include "net_err.h"
#include "netif.h"

typedef struct _pcap_data_t {
  const char* ip;
  const uint8_t* hwaddr;
} pcap_data_t;

net_err_t netif_pcap_open(netif_t* netif, void* data);

extern const netif_ops_t netdev_ops;

#endif