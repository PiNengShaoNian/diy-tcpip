#include "loop.h"

#include "dbg.h"
#include "netif.h"

static net_err_t loop_open(netif_t *netif, void *data) {
  netif->type = NETIF_TYPE_LOOP;

  return NET_ERR_OK;
}

static void loop_close(netif_t *netif) {}

net_err_t loop_xmit(netif_t *netif) { return NET_ERR_OK; }

static netif_ops_t loop_ops = {
    .open = loop_open,
    .close = loop_close,
    .xmit = loop_xmit,
};

net_err_t loop_init(void) {
  dbg_info(DBG_NETIF, "init loop");

  netif_t *netif = netif_open("loop", &loop_ops, (void *)0);
  if (!netif) {
    dbg_error(DBG_NETIF, "open loop err");
    return NET_ERR_NONE;
  }

  dbg_info(DBG_NETIF, "init done");
  return NET_ERR_OK;
}