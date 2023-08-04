#include "tcp.h"

#include "dbg.h"
#include "mblock.h"

static tcp_t tcp_tbl[TCP_MAX_NR];
static mblock_t tcp_mblock;
static nlist_t tcp_list;

net_err_t tcp_init(void) {
  dbg_info(DBG_TCP, "tcp init.");

  nlist_init(&tcp_list);
  mblock_init(&tcp_mblock, tcp_tbl, sizeof(tcp_t), TCP_MAX_NR, NLOCKER_NONE);

  dbg_info(DBG_TCP, "init done.");
  return NET_ERR_OK;
}