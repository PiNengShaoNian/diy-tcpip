#ifndef TIMER_H
#define TIMER_H

#include "net_cfg.h"
#include "net_err.h"
#include "nlist.h"

#define NET_TIMER_RELOAD (1 << 0)

struct _net_timer_t;
typedef void (*timer_proc_t)(struct _net_timer_t *timer, void *arg);

typedef struct _net_timer_t {
  char name[TIMER_NAME_SIZE];
  int flags;

  int curr;
  int reload;

  nlist_node_t node;
  void *arg;

  timer_proc_t proc;
} net_timer_t;

net_err_t net_timer_init(void);
net_err_t net_timer_add(net_timer_t *timer, const char *name, timer_proc_t proc,
                        void *arg, int ms, int flags);

#endif