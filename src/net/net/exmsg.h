#ifndef EXMSG_H
#define EXMSG_H

#include "net_err.h"

typedef struct _exmsg_t {
  enum {
    NET_EXMSG_NETIF_IN,
  } type;
} exmsg_t;

net_err_t exmsg_init(void);
net_err_t exmsg_start(void);

#endif