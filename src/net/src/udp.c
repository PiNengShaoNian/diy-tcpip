#include "udp.h"

#include "dbg.h"
#include "ipaddr.h"
#include "mblock.h"
#include "net_cfg.h"
#include "nlist.h"
#include "pktbuf.h"
#include "protocol.h"
#include "socket.h"
#include "tools.h"

static udp_t udp_tbl[UDP_MAX_NR];
static mblock_t udp_mblock;
static nlist_t udp_list;

net_err_t udp_init(void) {
  dbg_info(DBG_UDP, "udp init");

  nlist_init(&udp_list);
  mblock_init(&udp_mblock, udp_tbl, sizeof(udp_t), RAW_MAX_NR, NLOCKER_NONE);

  dbg_info(DBG_UDP, "udp init done");
  return NET_ERR_OK;
}

static int is_port_used(int port) {
  nlist_node_t *node;
  nlist_for_each(node, &udp_list) {
    sock_t *sock = (sock_t *)nlist_entry(node, sock_t, node);
    if (sock->local_port == port) {
      return 1;
    }
  }

  return 0;
}

static net_err_t alloc_port(sock_t *sock) {
  static int search_index = NET_PORT_DYN_START;
  for (int i = search_index; i < NET_PORT_DYN_END; i++) {
    int port = search_index++;
    if (search_index >= NET_PORT_DYN_END) {
      search_index = NET_PORT_DYN_START;
    }

    if (!is_port_used(port)) {
      sock->local_port = port;
      return NET_ERR_OK;
    }
  }

  return NET_ERR_NONE;
}

static net_err_t udp_sendto(struct _sock_t *sock, const void *buf, size_t len,
                            int flags, const struct x_sockaddr *dest,
                            x_socklen_t dest_len, ssize_t *result_len) {
  ipaddr_t dest_ip;
  struct x_sockaddr_in *addr = (struct x_sockaddr_in *)dest;
  ipaddr_from_buf(&dest_ip, addr->sin_addr.addr_array);
  uint16_t dport = x_ntohs(addr->sin_port);

  if (!ipaddr_is_any(&sock->remote_ip) &&
      !ipaddr_is_equal(&dest_ip, &sock->remote_ip)) {
    dbg_error(DBG_UDP, "dest is incorrect");
    return NET_ERR_PARAM;
  }

  if (sock->remote_port && (sock->remote_port != dport)) {
    dbg_error(DBG_UDP, "dest is incorrect");
    return NET_ERR_PARAM;
  }

  if (!sock->local_port && (sock->err = alloc_port(sock)) < 0) {
    dbg_error(DBG_UDP, "no port available");
    return NET_ERR_NONE;
  }

  pktbuf_t *pktbuf = pktbuf_alloc((int)len);
  if (!pktbuf) {
    dbg_error(DBG_UDP, "no buffer");
    return NET_ERR_MEM;
  }

  net_err_t err = pktbuf_write(pktbuf, (uint8_t *)buf, (int)len);
  if (err < 0) {
    dbg_error(DBG_UDP, "copy data error");
    goto fail;
  }

  err = udp_out(&dest_ip, dport, &sock->local_ip, sock->local_port, pktbuf);
  if (err < 0) {
    dbg_error(DBG_UDP, "send error");
    goto fail;
  }

  *result_len = (ssize_t)len;
  return NET_ERR_OK;

fail:
  pktbuf_free(pktbuf);
  return err;
}

sock_t *udp_create(int family, int protocol) {
  static const sock_ops_t udp_ops = {.setopt = sock_setopt,
                                     .sendto = udp_sendto};
  udp_t *udp = mblock_alloc(&udp_mblock, -1);

  if (!udp) {
    dbg_error(DBG_UDP, "no raw sock");
    return (sock_t *)0;
  }

  net_err_t err = sock_init((sock_t *)udp, family, protocol, &udp_ops);
  if (err < 0) {
    dbg_error(DBG_UDP, "create raw sock failed.");
    return (sock_t *)0;
  }

  nlist_init(&udp->recv_list);

  udp->base.rcv_wait = &udp->recv_wait;
  if (sock_wait_init(udp->base.rcv_wait) < 0) {
    dbg_error(DBG_UDP, "create rcv wait failed");
    goto create_failed;
  }

  nlist_insert_last(&udp_list, &udp->base.node);
  return (sock_t *)udp;
create_failed:
  sock_uninit(&udp->base);
  return (sock_t *)0;
}

net_err_t udp_out(ipaddr_t *dest_ip, uint16_t dest_port, ipaddr_t *src_ip,
                  uint16_t src_port, pktbuf_t *buf) {
  net_err_t err = pktbuf_add_header(buf, sizeof(udp_hdr_t), 1);
  if (err < 0) {
    dbg_error(DBG_UDP, "add header failed.");
    return NET_ERR_SIZE;
  }

  udp_hdr_t *udp_hdr = (udp_hdr_t *)pktbuf_data(buf);
  udp_hdr->src_port = x_htons(src_port);
  udp_hdr->dest_port = x_htons(dest_port);
  udp_hdr->total_len = x_htons(buf->total_size);
  udp_hdr->checksum = 0;

  ipv4_out(NET_PROTOCOL_UDP, dest_ip, src_ip, buf);

  if (err < 0) {
    dbg_error(DBG_UDP, "udp out err");
    return err;
  }

  return NET_ERR_OK;
}