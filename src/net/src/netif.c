#include "netif.h"

#include "dbg.h"
#include "mblock.h"
#include "pktbuf.h"

static netif_t netif_buffer[NETIF_DEV_CNT];
static mblock_t netif_mblock;
static nlist_t netif_list;
static netif_t *netif_default;

net_err_t netif_init(void) {
  dbg_info(DBG_NETIF, "init netif");

  nlist_init(&netif_list);
  mblock_init(&netif_mblock, netif_buffer, sizeof(netif_t), NETIF_DEV_CNT,
              NLOCKER_NONE);
  netif_default = (netif_t *)0;

  dbg_info(DBG_NETIF, "init done.");
  return NET_ERR_OK;
}

netif_t *netif_open(const char *dev_name, netif_ops_t *ops, void *ops_data) {
  netif_t *netif = (netif_t *)mblock_alloc(&netif_mblock, -1);

  if (!netif) {
    dbg_error(DBG_NETIF, "no netif");
    return (netif_t *)0;
  }

  ipaddr_set_any(&netif->ipaddr);
  ipaddr_set_any(&netif->netmask);
  ipaddr_set_any(&netif->gateway);

  plat_strncpy(netif->name, dev_name, NETIF_NAME_SIZE);
  netif->name[NETIF_NAME_SIZE - 1] = '\0';

  plat_memset(&netif->hwaddr, 0, sizeof(netif_hwaddr_t));
  netif->type = NETIF_TYPE_NONE;
  netif->mtu = 0;
  nlist_node_init(&netif->node);

  net_err_t err =
      fixq_init(&netif->in_q, netif->in_q_buf, NETIF_INQ_SIZE, NLOCKER_THREAD);
  if (err < 0) {
    dbg_error(DBG_NETIF, "netif in_q init failed.");
    mblock_free(&netif_mblock, netif);
    return (netif_t *)0;
  }

  err = fixq_init(&netif->out_q, netif->out_q_buf, NETIF_OUTQ_SIZE,
                  NLOCKER_THREAD);
  if (err < 0) {
    dbg_error(DBG_NETIF, "netif out_q init failed.");
    mblock_free(&netif_mblock, netif);
    fixq_destroy(&netif->in_q);
    return (netif_t *)0;
  }

  err = ops->open(netif, ops_data);

  if (err < 0) {
    dbg_error(DBG_NETIF, "netif ops open failed");
    goto fail;
  }
  netif->state = NETIF_OPENED;

  if (netif->type == NETIF_TYPE_NONE) {
    dbg_error(DBG_NETIF, "netif type unknown");
    goto fail;
  }

  netif->ops = ops;
  netif->ops_data = ops_data;

  nlist_insert_last(&netif_list, &netif->node);

  return netif;

fail:
  if (netif->state == NETIF_OPENED) {
    netif->ops->close(netif);
  }
  fixq_destroy(&netif->in_q);
  fixq_destroy(&netif->out_q);
  mblock_free(&netif_mblock, netif);
  return (netif_t *)0;
}

net_err_t netif_set_addr(netif_t *netif, ipaddr_t *ip, ipaddr_t *netmask,
                         ipaddr_t *gateway) {
  ipaddr_copy(&netif->ipaddr, ip ? ip : ipaddr_get_any());
  ipaddr_copy(&netif->netmask, netmask ? netmask : ipaddr_get_any());
  ipaddr_copy(&netif->gateway, gateway ? gateway : ipaddr_get_any());
  return NET_ERR_OK;
}

net_err_t netif_set_hwaddr(netif_t *netif, const char *hwaddr, int len) {
  plat_memcpy(netif->hwaddr.addr, hwaddr, len);
  netif->hwaddr.len = len;
  return NET_ERR_OK;
}

net_err_t netif_set_active(netif_t *netif) {
  if (netif->state != NETIF_OPENED) {
    dbg_error(DBG_NETIF, "netif is not opened");
    return NET_ERR_STATE;
  }

  netif->state = NETIF_ACTIVE;
  return NET_ERR_OK;
}

net_err_t netif_set_deactivate(netif_t *netif) {
  if (netif->state != NETIF_ACTIVE) {
    dbg_error(DBG_NETIF, "netif is not active");
    return NET_ERR_STATE;
  }

  pktbuf_t *buf;
  while ((buf = fixq_recv(&netif->in_q, -1)) != (pktbuf_t *)0) {
    pktbuf_free(buf);
  }

  while ((buf = fixq_recv(&netif->out_q, -1)) != (pktbuf_t *)0) {
    pktbuf_free(buf);
  }

  if (netif_default == netif) {
    netif_default = (netif_t *)0;
  }

  netif->state = NETIF_OPENED;
  return NET_ERR_OK;
}
