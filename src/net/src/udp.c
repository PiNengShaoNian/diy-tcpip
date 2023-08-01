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

  dbg_info(DBG_RAW, "udp init done");
  return NET_ERR_OK;
}
