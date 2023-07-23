#ifndef NETIF_H
#define NETIF_H

#include <stdint.h>

#include "fixq.h"
#include "ipaddr.h"
#include "nlist.h"

typedef struct _netif_hwaddr_t {
  uint8_t addr[NETIF_HWADDR_SIZE];
  uint8_t len;
} netif_hwaddr_t;

typedef enum _netif_type_t {
  NETIF_TYPE_NONE = 0,
  NETIF_TYPE_ETHER,
  NETIF_TYPE_LOOP,

  NET_TYPE_SIZE,
} netif_type_t;

typedef struct _netif_t {
  char name[NETIF_NAME_SIZE];
  netif_hwaddr_t hwaddr;

  ipaddr_t ipaddr;
  ipaddr_t netmask;
  ipaddr_t gateway;

  netif_type_t type;
  int mtu;
  nlist_node_t node;

  enum {
    NETIF_CLOSED,
    NETIF_OPENED,
    NETIF_ACTIVE,
  } state;

  fixq_t in_q;
  void *in_q_buf[NETIF_INQ_SIZE];
  fixq_t out_q;
  void *out_q_buf[NETIF_OUTQ_SIZE];
} netif_t;

#endif