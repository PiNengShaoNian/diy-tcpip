#include "timer.h"

#include "dbg.h"
#include "sys_plat.h"

static nlist_t timer_list;

#if DBG_DISP_ENABLED(DBG_TIMER)
void display_timer_list(void) {
  plat_printf("------------ timer list ------------\n");

  nlist_node_t *node;
  int index = 0;
  nlist_for_each(node, &timer_list) {
    net_timer_t *timer = nlist_entry(node, net_timer_t, node);

    plat_printf("%d: %s, period=%d, curr: %d ms, reload: %d ms\n", index++,
                timer->name, (timer->flags & NET_TIMER_RELOAD) ? 1 : 0,
                timer->curr, timer->reload);
    plat_printf("------------- timer list end ------------\n");
  }
}
#else
#define display_timer_list()
#endif

net_err_t net_timer_init(void) {
  dbg_info(DBG_TIMER, "timer init");

  nlist_init(&timer_list);

  dbg_info(DBG_TIMER, "done");
  return NET_ERR_OK;
}

net_err_t net_timer_add(net_timer_t *timer, const char *name, timer_proc_t proc,
                        void *arg, int ms, int flags) {
  dbg_info(DBG_TIMER, "insert timer: %s", timer->name);

  plat_strncpy(timer->name, name, TIMER_NAME_SIZE);
  timer->name[TIMER_NAME_SIZE - 1] = '\0';

  timer->reload = ms;
  timer->curr = ms;
  timer->proc = proc;
  timer->arg = arg;
  timer->flags = flags;

  nlist_insert_last(&timer_list, &timer->node);

  display_timer_list();
  return NET_ERR_OK;
}