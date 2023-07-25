#include "timer.h"

#include "dbg.h"

static nlist_t timer_list;

net_err_t net_timer_init(void) {
  dbg_info(DBG_TIMER, "timer init");

  nlist_init(&timer_list);

  dbg_info(DBG_TIMER, "done");
  return NET_ERR_OK;
}