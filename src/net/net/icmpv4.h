#ifndef ICMPV4_H
#define ICMPV4_H

#include "ipaddr.h"
#include "net_err.h"
#include "pktbuf.h"

net_err_t icmpv4_init(void);
net_err_t icmpv4_in(ipaddr_t *src_ip, ipaddr_t *netif_ip, pktbuf_t *buf);

#endif