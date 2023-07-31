#include "raw.h"

#include "dbg.h"
#include "ipv4.h"
#include "mblock.h"
#include "nlist.h"
#include "socket.h"

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

static net_err_t raw_sendto(struct _sock_t *sock, const void *buf, size_t len,
                            int flags, const struct x_sockaddr *dest,
                            x_socklen_t dest_len, ssize_t *result_len) {
  ipaddr_t dest_ip;
  struct x_sockaddr_in *addr = (struct x_sockaddr_in *)dest;
  ipaddr_from_buf(&dest_ip, addr->sin_addr.addr_array);

  if (!ipaddr_is_any(&sock->remote_ip) &&
      !ipaddr_is_equal(&dest_ip, &sock->remote_ip)) {
    dbg_error(DBG_RAW, "dest is incorrect");
    return NET_ERR_PARAM;
  }

  pktbuf_t *pktbuf = pktbuf_alloc((int)len);
  if (!pktbuf) {
    dbg_error(DBG_SOCKET, "no buffer");
    return NET_ERR_MEM;
  }

  net_err_t err = pktbuf_write(pktbuf, (uint8_t *)buf, (int)len);
  if (err < 0) {
    dbg_error(DBG_RAW, "copy data error");
    goto fail;
  }

  err =
      ipv4_out(sock->protocol, &dest_ip, &netif_get_default()->ipaddr, pktbuf);
  if (err < 0) {
    dbg_error(DBG_RAW, "send error");
    goto fail;
  }

  *result_len = (ssize_t)len;
  return NET_ERR_OK;

fail:
  pktbuf_free(pktbuf);
  return err;
}

net_err_t raw_recvfrom(struct _sock_t *sock, const void *buf, size_t len,
                       int flags, const struct x_sockaddr *src,
                       x_socklen_t *src_len, ssize_t *result_len) {
  *result_len = 0;

  return NET_ERR_NEED_WAIT;
}

sock_t *raw_create(int family, int protocol) {
  static const sock_ops_t raw_ops = {
      .sendto = raw_sendto,
      .recvfrom = raw_recvfrom,
      .setopt = sock_setopt,
  };
  raw_t *raw = mblock_alloc(&raw_mblock, -1);

  if (!raw) {
    dbg_error(DBG_RAW, "no raw sock");
    return (sock_t *)0;
  }

  net_err_t err = sock_init((sock_t *)raw, family, protocol, &raw_ops);
  if (err < 0) {
    dbg_error(DBG_RAW, "create raw sock failed.");
    return (sock_t *)0;
  }

  raw->base.rcv_wait = &raw->recv_wait;
  if (sock_wait_init(raw->base.rcv_wait) < 0) {
    dbg_error(DBG_RAW, "create rcv wait failed");
    goto create_failed;
  }

  nlist_insert_last(&raw_list, &raw->base.node);
  return (sock_t *)raw;
create_failed:
  sock_uninit(&raw->base);
  return (sock_t *)0;
}

static raw_t *raw_find(ipaddr_t *src, ipaddr_t *dest, int protocol) {
  nlist_node_t *node;

  nlist_for_each(node, &raw_list) {
    raw_t *raw = (raw_t *)nlist_entry(node, sock_t, node);

    if (raw->base.protocol && (raw->base.protocol != protocol)) {
      continue;
    }

    if (!ipaddr_is_any(&raw->base.remote_ip) &&
        !ipaddr_is_equal(&raw->base.remote_ip, src)) {
      continue;
    }

    if (!ipaddr_is_any(&raw->base.local_ip) &&
        !ipaddr_is_equal(&raw->base.local_ip, dest)) {
      continue;
    }

    return raw;
  }

  return (raw_t *)0;
}

net_err_t raw_in(pktbuf_t *pktbuf) {
  ipv4_hdr_t *iphdr = (ipv4_hdr_t *)pktbuf_data(pktbuf);

  ipaddr_t src, dest;
  ipaddr_from_buf(&dest, iphdr->dest_ip);
  ipaddr_from_buf(&src, iphdr->src_ip);

  raw_t *raw = raw_find(&src, &dest, iphdr->protocol);
  if (!raw) {
    dbg_warning(DBG_RAW, "no raw for this packet");
    return NET_ERR_UNREACH;
  }

  return NET_ERR_OK;
}