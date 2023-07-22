#ifndef EXMSG_H
#define EXMSG_H

#include "net_err.h"

typedef struct _exmsg_t {
  enum {
    NET_EXMSG_NETIF_IN,
  } type;

  int id;
} exmsg_t;

net_err_t exmsg_init(void);
net_err_t exmsg_start(void);
net_err_t exmsg_netif_in(void);

#endif