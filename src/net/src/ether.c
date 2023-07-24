#include "ether.h"

#include "dbg.h"
#include "net_err.h"
#include "netif.h"

net_err_t ether_open(struct _netif_t *netif) { return NET_ERR_OK; }

void ether_close(struct _netif_t *netif) {}

static net_err_t is_pkt_ok(ether_pkt_t *frame, int total_size) {
  if (total_size > sizeof(ether_hdr_t) + ETHER_MTU) {
    dbg_warning(DBG_ETHER, "frame size too big: %d", total_size);
    return NET_ERR_SIZE;
  }

  if (total_size < sizeof(ether_hdr_t)) {
    dbg_warning(DBG_ETHER, "frame size too small: %d", total_size);
    return NET_ERR_SIZE;
  }

  return NET_ERR_OK;
}

net_err_t ether_in(struct _netif_t *netif, pktbuf_t *buf) {
  dbg_info(DBG_ETHER, "ether in");

  ether_pkt_t *pkt = (ether_pkt_t *)pktbuf_data(buf);

  net_err_t err;
  if ((err = is_pkt_ok(pkt, buf->total_size)) < 0) {
    dbg_warning(DBG_ETHER, "ether pkt error");
    return err;
  }

  pktbuf_free(buf);
  return NET_ERR_OK;
}

net_err_t ether_out(struct _netif_t *netif, ipaddr_t *dest, pktbuf_t *buf) {
  return NET_ERR_OK;
}

net_err_t ether_init(void) {
  static const link_layer_t link_layer = {
      .type = NETIF_TYPE_ETHER,
      .open = ether_open,
      .close = ether_close,
      .in = ether_in,
      .out = ether_out,
  };

  dbg_info(DBG_ETHER, "init ether");

  net_err_t err = netif_register_layer(NETIF_TYPE_ETHER, &link_layer);
  if (err < 0) {
    dbg_info(DBG_ETHER, "register error");
    return err;
  }

  dbg_info(DBG_ETHER, "done");

  return NET_ERR_OK;
}
