#include "icmpv4.h"

#include "dbg.h"
#include "ipv4.h"

net_err_t icmpv4_init(void) {
  dbg_info(DBG_ICMPv4, "init icmp");

  dbg_info(DBG_ICMPv4, "init icmp done");
  return NET_ERR_OK;
}

static net_err_t is_pkt_ok(icmpv4_pkt_t *pkt, int size, pktbuf_t *buf) {
  if (size <= sizeof(ipv4_hdr_t)) {
    dbg_warning(DBG_ICMPv4, "size error");
    return NET_ERR_SIZE;
  }

  uint16_t checksum = pktbuf_checksum16(buf, size, 0, 1);
  if (checksum != 0) {
    dbg_warning(DBG_ICMPv4, "bad checksum");
    return NET_ERR_BROKEN;
  }

  return NET_ERR_OK;
}

net_err_t icmpv4_in(ipaddr_t *src_ip, ipaddr_t *netif_ip, pktbuf_t *buf) {
  dbg_info(DBG_ICMPv4, "icmpv4 in");

  ipv4_pkt_t *ip_pkt = (ipv4_pkt_t *)pktbuf_data(buf);
  int iphdr_size = ipv4_hdr_size(ip_pkt);
  net_err_t err = pktbuf_set_cont(buf, iphdr_size + sizeof(icmpv4_hdr_t));

  if (err < 0) {
    dbg_error(DBG_ICMPv4, "set icmp cont failed.");
    return err;
  }

  ip_pkt = (ipv4_pkt_t *)pktbuf_data(buf);
  err = pktbuf_remove_header(buf, iphdr_size);

  if (err < 0) {
    dbg_error(DBG_ICMPv4, "remove ip header failed.");
    return err;
  }

  pktbuf_reset_acc(buf);
  icmpv4_pkt_t *icmp_pkt = (icmpv4_pkt_t *)pktbuf_data(buf);
  if ((err = is_pkt_ok(icmp_pkt, buf->total_size, buf)) < 0) {
    dbg_warning(DBG_ICMPv4, "icmp pkt error.");
    return err;
  }

  return NET_ERR_OK;
}
