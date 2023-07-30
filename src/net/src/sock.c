#include "sock.h"

#include "dbg.h"
#include "exmsg.h"
#include "sys.h"

#define SOCKET_MAX_NR 10

static x_socket_t socket_tbl[SOCKET_MAX_NR];

static int get_index(x_socket_t *socket) { return (int)(socket - socket_tbl); }

static x_socket_t *get_socket(int idx) {
  if (idx < 0 || idx >= SOCKET_MAX_NR) {
    return (x_socket_t *)0;
  }

  return socket_tbl + idx;
}

static x_socket_t *socket_alloc(void) {
  x_socket_t *s = (x_socket_t *)0;

  for (int i = 0; i < SOCKET_MAX_NR; i++) {
    x_socket_t *curr = socket_tbl + i;
    if (curr->state == SOCKET_STATE_FREE) {
      curr->state = SOCKET_STATE_USED;
      s = curr;
      break;
    }
  }

  return s;
}

static void socket_free(x_socket_t *s) { s->state = SOCKET_STATE_FREE; }

net_err_t socket_init(void) {
  plat_memset(socket_tbl, 0, sizeof(socket_tbl));
  return NET_ERR_OK;
}

net_err_t sock_init(sock_t *sock, int family, int protocol,
                    const sock_ops_t *ops) {
  sock->protocol = protocol;
  sock->family = family;
  sock->ops = ops;

  ipaddr_set_any(&sock->local_ip);
  ipaddr_set_any(&sock->remote_ip);
  sock->local_port = 0;
  sock->remote_port = 0;
  sock->err = NET_ERR_OK;
  sock->rcv_tmo = 0;
  sock->snd_tmo = 0;
  nlist_node_init(&sock->node);
  return NET_ERR_OK;
}

net_err_t sock_create_req_in(func_msg_t *msg) {
  sock_req_t *req = (sock_req_t *)msg->param;

  x_socket_t *s = socket_alloc();
  if (!s) {
    dbg_error(DBG_SOCKET, "no socket");
    return NET_ERR_MEM;
  }

  req->sockfd = get_index(s);

  return NET_ERR_OK;
}