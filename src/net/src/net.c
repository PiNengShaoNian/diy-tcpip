#include "net.h"

#include "dbg.h"
#include "exmsg.h"
#include "net_plat.h"
#include "netif.h"
#include "pktbuf.h"

net_err_t net_init(void) {
  dbg_info(DBG_INIT, "init net");

  net_plat_init();

  exmsg_init();
  pktbuf_init();
  netif_init();
  return NET_ERR_OK;
}

net_err_t net_start(void) {
  exmsg_start();

  dbg_info(DBG_INIT, "net is running");

  return NET_ERR_OK;
}
