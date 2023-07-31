#ifndef SOCK_H
#define SOCK_H

#include <stdint.h>

#include "ipaddr.h"
#include "net_err.h"
#include "nlist.h"
#include "sys.h"

struct _sock_t;
struct x_sockaddr;
typedef int x_socklen_t;

typedef struct _sock_ops_t {
  net_err_t (*close)(struct _sock_t *sock);
  net_err_t (*sendto)(struct _sock_t *sock, const void *buf, size_t len,
                      int flags, const struct x_sockaddr *dest,
                      x_socklen_t dest_len, ssize_t *result_len);
  net_err_t (*recvfrom)(struct _sock_t *sock, const void *buf, size_t len,
                        int flags, const struct x_sockaddr *src,
                        x_socklen_t src_len, ssize_t *result_len);
  net_err_t (*setopt)(struct _sock_t *s, int level, int optname,
                      const char *optval, int optlen);
  void (*destroy)(struct _sock_t *s);
} sock_ops_t;

sock_ops_t;
typedef struct _sock_t {
  ipaddr_t local_ip;
  uint16_t local_port;
  ipaddr_t remote_ip;
  uint16_t remote_port;

  const sock_ops_t *ops;

  int family;
  int protocol;
  int err;
  int rcv_tmo;
  int snd_tmo;

  nlist_node_t node;
} sock_t;

typedef struct _x_socket_t {
  enum {
    SOCKET_STATE_FREE,
    SOCKET_STATE_USED,
  } state;

  sock_t *sock;
} x_socket_t;

typedef struct _sock_create_t {
  int family;
  int type;
  int protocol;
} sock_create_t;

typedef struct _sock_req_t {
  int sockfd;
  union {
    sock_create_t create;
  };
} sock_req_t;

net_err_t socket_init(void);
net_err_t sock_create_req_in(struct _func_msg_t *msg);

net_err_t sock_init(sock_t *sock, int family, int protocol,
                    const sock_ops_t *ops);

#endif