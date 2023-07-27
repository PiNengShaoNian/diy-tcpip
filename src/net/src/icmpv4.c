#include "icmpv4.h"

#include "dbg.h"

net_err_t icmpv4_init(void) {
  dbg_info(DBG_ICMP, "init icmp");

  dbg_info(DBG_ICMP, "init icmp done");
  return NET_ERR_OK;
}

net_err_t icmpv4_in(ipaddr_t *src_ip, ipaddr_t *netif_ip, pktbuf_t *buf) {
  return NET_ERR_OK;
}
