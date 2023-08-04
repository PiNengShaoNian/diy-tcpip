#include "tcp_in.h"

#include "dbg.h"
#include "protocol.h"
#include "tcp_out.h"
#include "tools.h"

void tcp_seg_init(tcp_seg_t *seg, pktbuf_t *buf, ipaddr_t *local,
                  ipaddr_t *remote) {
  seg->buf = buf;
  seg->hdr = (tcp_hdr_t *)pktbuf_data(buf);

  ipaddr_copy(&seg->local_ip, local);
  ipaddr_copy(&seg->remote_ip, remote);
  seg->data_len = buf->total_size - tcp_hdr_size(seg->hdr);
  seg->seq = seg->hdr->seq;
  seg->seq_len = seg->data_len + seg->hdr->f_syn + seg->hdr->f_fin;
}

net_err_t tcp_in(pktbuf_t *buf, ipaddr_t *src_ip, ipaddr_t *dest_ip) {
  tcp_hdr_t *tcp_hdr = (tcp_hdr_t *)pktbuf_data(buf);
  if (tcp_hdr->checksum) {
    pktbuf_reset_acc(buf);
    if (checksum_peso(buf, dest_ip, src_ip, NET_PROTOCOL_TCP)) {
      dbg_warning(DBG_TCP, "tcp check sum failed.");
      return NET_ERR_BROKEN;
    }
  }

  if (buf->total_size < sizeof(tcp_hdr_t) ||
      buf->total_size < tcp_hdr_size(tcp_hdr)) {
    dbg_warning(DBG_TCP, "tcp pkt size error");
    return NET_ERR_SIZE;
  }

  if (!tcp_hdr->sport || !tcp_hdr->dport) {
    dbg_warning(DBG_TCP, "port == 0");
    return NET_ERR_BROKEN;
  }

  if (tcp_hdr->flags == 0) {
    dbg_warning(DBG_TCP, "flags == 0");
    return NET_ERR_BROKEN;
  }

  tcp_hdr->sport = x_ntohs(tcp_hdr->sport);
  tcp_hdr->dport = x_ntohs(tcp_hdr->dport);
  tcp_hdr->seq = x_ntohl(tcp_hdr->seq);
  tcp_hdr->ack = x_ntohl(tcp_hdr->ack);
  tcp_hdr->win = x_ntohs(tcp_hdr->win);
  tcp_hdr->urgptr = x_ntohs(tcp_hdr->urgptr);

  tcp_seg_t seg;
  tcp_seg_init(&seg, buf, dest_ip, src_ip);

  tcp_send_reset(&seg);

  return NET_ERR_OK;
}