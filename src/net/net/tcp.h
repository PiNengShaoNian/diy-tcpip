#ifndef TCP_H
#define TCP_H

#include "net_err.h"
#include "nlist.h"
#include "sock.h"

typedef struct _tcp_t {
  sock_t base;

  nlist_t recv_list;

  sock_wait_t recv_wait;
} tcp_t;

net_err_t tcp_init(void);
sock_t *tcp_create(int family, int protocol);

#endif