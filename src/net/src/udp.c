#include "udp.h"

#include "dbg.h"
#include "mblock.h"
#include "net_cfg.h"
#include "nlist.h"

static udp_t udp_tbl[UDP_MAX_NR];
static mblock_t udp_mblock;
static nlist_t udp_list;

net_err_t udp_init(void) {
  dbg_info(DBG_UDP, "udp init");

  nlist_init(&udp_list);
  mblock_init(&udp_mblock, udp_tbl, sizeof(udp_t), RAW_MAX_NR, NLOCKER_NONE);

  dbg_info(DBG_UDP, "udp init done");
  return NET_ERR_OK;
}

sock_t *udp_create(int family, int protocol) {
  static const sock_ops_t udp_ops = {
      .setopt = sock_setopt,
  };
  udp_t *udp = mblock_alloc(&udp_mblock, -1);

  if (!udp) {
    dbg_error(DBG_UDP, "no raw sock");
    return (sock_t *)0;
  }

  net_err_t err = sock_init((sock_t *)udp, family, protocol, &udp_ops);
  if (err < 0) {
    dbg_error(DBG_UDP, "create raw sock failed.");
    return (sock_t *)0;
  }

  nlist_init(&udp->recv_list);

  udp->base.rcv_wait = &udp->recv_wait;
  if (sock_wait_init(udp->base.rcv_wait) < 0) {
    dbg_error(DBG_UDP, "create rcv wait failed");
    goto create_failed;
  }

  nlist_insert_last(&udp_list, &udp->base.node);
  return (sock_t *)udp;
create_failed:
  sock_uninit(&udp->base);
  return (sock_t *)0;
}