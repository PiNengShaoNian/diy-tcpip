#include "ipv4.h"

#include "dbg.h"

net_err_t ipv4_init(void) {
  dbg_info(DBG_IP, "init ip\n");

  dbg_info(DBG_IP, "done\n");
  return NET_ERR_OK;
}

net_err_t ipv4_in(netif_t *netif, pktbuf_t *buf) {
  dbg_info(DBG_IP, "ip in\n");

  pktbuf_free(buf);
  return NET_ERR_OK;
}