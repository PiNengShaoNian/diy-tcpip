#include "raw.h"

#include "dbg.h"
#include "mblock.h"
#include "nlist.h"

static raw_t raw_tbl[RAW_MAX_NR];
static mblock_t raw_mblock;
static nlist_t raw_list;

net_err_t raw_init(void) {
  dbg_info(DBG_RAW, "raw init");

  nlist_init(&raw_list);
  mblock_init(&raw_mblock, raw_tbl, sizeof(raw_t), RAW_MAX_NR, NLOCKER_NONE);

  dbg_info(DBG_RAW, "done");
  return NET_ERR_OK;
}