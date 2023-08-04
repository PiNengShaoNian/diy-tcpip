#include "tcp_out.h"

#include "dbg.h"
#include "ipv4.h"
#include "protocol.h"
#include "tools.h"

static net_err_t send_out(tcp_hdr_t *out, pktbuf_t *buf, ipaddr_t *dest,
                          ipaddr_t *src) {
  out->sport = x_htons(out->sport);
  out->dport = x_htons(out->dport);
  out->seq = x_htons(out->seq);
  out->ack = x_htons(out->ack);
  out->win = x_htons(out->win);
  out->urgptr = x_htons(out->urgptr);

  out->checksum = 0;
  out->checksum = checksum_peso(buf, dest, src, NET_PROTOCOL_TCP);

  net_err_t err = ipv4_out(NET_PROTOCOL_TCP, dest, src, buf);
  if (err < 0) {
    dbg_warning(DBG_TCP, "send tcp error");
    pktbuf_free(buf);
    return err;
  }

  return NET_ERR_OK;
}

net_err_t tcp_send_reset(tcp_seg_t *seg) {
  tcp_hdr_t *in = seg->hdr;
  pktbuf_t *buf = pktbuf_alloc(sizeof(tcp_hdr_t));
  if (!buf) {
    dbg_warning(DBG_TCP, "no pktbuf");
    return NET_ERR_NONE;
  }

  tcp_hdr_t *out = (tcp_hdr_t *)pktbuf_data(buf);
  out->sport = in->dport;
  out->dport = in->sport;
  out->flags = 0;
  out->f_rst = 1;
  tcp_set_hdr_size(out, sizeof(tcp_hdr_t));
  out->win = out->urgptr = 0;

  return send_out(out, buf, &seg->remote_ip, &seg->local_ip);
}
