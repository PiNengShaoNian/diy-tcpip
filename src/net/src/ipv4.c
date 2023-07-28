#include "ipv4.h"

#include "dbg.h"
#include "icmpv4.h"
#include "mblock.h"
#include "protocol.h"
#include "tools.h"

static uint16_t packet_id = 0;

static ip_frag_t frag_array[IP_FRAGS_MAX_NR];
static mblock_t frag_mblock;
static nlist_t frag_list;

static int get_data_size(ipv4_pkt_t *pkt) {
  return pkt->hdr.total_len - ipv4_hdr_size(pkt);
}

static uint16_t get_frag_start(ipv4_pkt_t *pkt) {
  return pkt->hdr.frag_offset * 8;
}

static uint16_t get_frag_end(ipv4_pkt_t *pkt) {
  return get_frag_start(pkt) + get_data_size(pkt);
}

#if DBG_DISP_ENABLED(DBG_IP)

static display_ip_frags(void) {
  plat_printf("ip frags: \n");

  int f_index = 0;
  nlist_node_t *node;
  nlist_for_each(node, &frag_list) {
    ip_frag_t *frag = nlist_entry(node, ip_frag_t, node);

    plat_printf("[%d]: \n", f_index++);
    dbg_dump_ip(DBG_IP, "    ip: ", &frag->ip);
    plat_printf("    id: %d\n", frag->id);
    plat_printf("    tmo: %d\n", frag->tmo);
    plat_printf("    bufs count: %d\n", nlist_count(&frag->buf_list));
    plat_printf("    bufs:\n");

    int p_index = 0;
    nlist_node_t *p_node;
    nlist_for_each(p_node, &frag->buf_list) {
      pktbuf_t *buf = nlist_entry(p_node, pktbuf_t, node);

      ipv4_pkt_t *pkt = (ipv4_pkt_t *)pktbuf_data(buf);
      plat_printf("    B%d:[%d-%d], ", p_index, get_frag_start(pkt),
                  get_frag_end(pkt) - 1);
    }
    plat_printf("\n");
  }
}

static void display_ip_pkt(ipv4_pkt_t *pkt) {
  ipv4_hdr_t *ip_hdr = &pkt->hdr;

  plat_printf("--------------- ip packet---------------\n");
  plat_printf("    version: %d\n", ip_hdr->version);
  plat_printf("    header len: %d\n", ipv4_hdr_size(pkt));
  plat_printf("    total len: %d\n", ip_hdr->total_len);
  plat_printf("    id: %d\n", ip_hdr->id);
  plat_printf("    ttl: %d\n", ip_hdr->ttl);
  plat_printf("    frag_offset: %d\n", ip_hdr->frag_offset);
  plat_printf("    more: %d\n", ip_hdr->more);
  plat_printf("    protocol: %d\n", ip_hdr->protocol);
  plat_printf("    checksum: %d\n", ip_hdr->hdr_checksum);
  dbg_dump_ip_buf(DBG_IP, "    src ip: ", ip_hdr->src_ip);
  dbg_dump_ip_buf(DBG_IP, "    dest ip: ", ip_hdr->dest_ip);
  plat_printf("--------------- ip packet end---------------\n");
}

#else
#define display_ip_pkt(pkt)
#define display_ip_frags()
#endif

static net_err_t frag_init(void) {
  nlist_init(&frag_list);
  net_err_t err = mblock_init(&frag_mblock, frag_array, sizeof(ip_frag_t),
                              IP_FRAGS_MAX_NR, NLOCKER_NONE);
  return err;
}

static void frag_free_buf_list(ip_frag_t *frag) {
  nlist_node_t *node;
  while (node = nlist_remove_first(&frag->buf_list)) {
    pktbuf_t *buf = nlist_entry(node, pktbuf_t, node);
    pktbuf_free(buf);
  }
}

static void frag_free(ip_frag_t *frag) {
  frag_free_buf_list(frag);
  nlist_remove(&frag_list, &frag->node);
  mblock_free(&frag_mblock, frag);
}

static ip_frag_t *frag_alloc(void) {
  ip_frag_t *frag = mblock_alloc(&frag_mblock, -1);

  if (!frag) {
    nlist_node_t *node = nlist_remove_last(&frag_list);
    frag = nlist_entry(node, ip_frag_t, node);
    if (frag) {
      frag_free_buf_list(frag);
    }
  }

  return frag;
}

static void frag_add(ip_frag_t *frag, ipaddr_t *ip, uint16_t id) {
  ipaddr_copy(&frag->ip, ip);
  frag->tmo = 0;
  frag->id = id;
  nlist_node_init(&frag->node);
  nlist_init(&frag->buf_list);

  nlist_insert_first(&frag_list, &frag->node);
}

static ip_frag_t *frag_find(ipaddr_t *ip, uint16_t id) {
  nlist_node_t *node;

  nlist_for_each(node, &frag_list) {
    ip_frag_t *frag = nlist_entry(node, ip_frag_t, node);

    if (ipaddr_is_equal(ip, &frag->ip) && (id == frag->id)) {
      nlist_remove(&frag_list, node);
      nlist_insert_first(&frag_list, node);
      return frag;
    }
  }

  return (ip_frag_t *)0;
}

net_err_t ipv4_init(void) {
  dbg_info(DBG_IP, "init ip");

  net_err_t err = frag_init();
  if (err < 0) {
    dbg_error(DBG_IP, "frag init failed.");
    return err;
  }

  dbg_info(DBG_IP, "init done");
  return NET_ERR_OK;
}

static net_err_t is_pkt_ok(ipv4_pkt_t *pkt, int size, netif_t *netif) {
  if (pkt->hdr.version != NET_VERSION_IPV4) {
    dbg_warning(DBG_IP, "invalid ip version = %d", pkt->hdr.version);
    return NET_ERR_NOT_SUPPORT;
  }

  int hdr_len = ipv4_hdr_size(pkt);
  if (hdr_len < sizeof(ipv4_hdr_t)) {
    dbg_warning(DBG_IP, "ipv4 header error");
    return NET_ERR_SIZE;
  }

  int total_size = x_ntohs(pkt->hdr.total_len);
  if (total_size < sizeof(ipv4_hdr_t) || (size < total_size)) {
    dbg_warning(DBG_IP, "ipv4 size error");
    return NET_ERR_SIZE;
  }

  if (pkt->hdr.hdr_checksum) {
    uint16_t c = checksum_16(pkt, hdr_len, 0, 1);
    if (c != 0) {
      dbg_warning(DBG_IP, "bad checksum");
      return NET_ERR_BROKEN;
    }
  }

  return NET_ERR_OK;
}

static void iphdr_ntohs(ipv4_pkt_t *pkt) {
  pkt->hdr.total_len = x_ntohs(pkt->hdr.total_len);
  pkt->hdr.id = x_ntohs(pkt->hdr.id);
  pkt->hdr.frag_all = x_ntohs(pkt->hdr.frag_all);
}

static void iphdr_htons(ipv4_pkt_t *pkt) {
  pkt->hdr.total_len = x_htons(pkt->hdr.total_len);
  pkt->hdr.id = x_htons(pkt->hdr.id);
  pkt->hdr.frag_all = x_htons(pkt->hdr.frag_all);
}

static net_err_t ip_frag_in(netif_t *netif, pktbuf_t *buf, ipaddr_t *src_ip,
                            ipaddr_t *dest_ip) {
  ipv4_pkt_t *curr = (ipv4_pkt_t *)pktbuf_data(buf);

  ip_frag_t *frag = frag_find(src_ip, curr->hdr.id);
  if (!frag) {
    frag = frag_alloc();
    frag_add(frag, src_ip, curr->hdr.id);
  }

  display_ip_frags();
  return NET_ERR_OK;
}

static net_err_t ip_normal_in(netif_t *netif, pktbuf_t *buf, ipaddr_t *src_ip,
                              ipaddr_t *dest_ip) {
  ipv4_pkt_t *pkt = (ipv4_pkt_t *)pktbuf_data(buf);

  display_ip_pkt(pkt);

  switch (pkt->hdr.protocol) {
    case NET_PROTOCOL_ICMPv4:
      net_err_t err = icmpv4_in(src_ip, &netif->ipaddr, buf);

      if (err < 0) {
        dbg_warning(DBG_IP, "icmp in failed.");
      }

      return err;
    case NET_PROTOCOL_UDP:
      iphdr_htons(pkt);
      icmpv4_out_unreach(src_ip, &netif->ipaddr, ICMPv4_UNREACH_PORT, buf);
      break;
    case NET_PROTOCOL_TCP:
      break;
    default:
      dbg_warning(DBG_IP, "unknown protocol");
      break;
  }

  return NET_ERR_UNREACH;
}

net_err_t ipv4_in(netif_t *netif, pktbuf_t *buf) {
  dbg_info(DBG_IP, "ip in");

  net_err_t err = pktbuf_set_cont(buf, sizeof(ipv4_hdr_t));

  if (err < 0) {
    dbg_error(DBG_IP, "adjust header failed, err = %d", err);
    return err;
  }

  ipv4_pkt_t *pkt = (ipv4_pkt_t *)pktbuf_data(buf);
  if (is_pkt_ok(pkt, buf->total_size, netif) != NET_ERR_OK) {
    dbg_warning(DBG_INIT, "packet is broken.");
    return err;
  }

  iphdr_ntohs(pkt);
  err = pktbuf_resize(buf, pkt->hdr.total_len);

  if (err < 0) {
    dbg_error(DBG_IP, "ip pkt resize failed.");
  }

  ipaddr_t dest_ip, src_ip;
  ipaddr_from_buf(&dest_ip, pkt->hdr.dest_ip);
  ipaddr_from_buf(&src_ip, pkt->hdr.src_ip);

  if (!ipaddr_is_match(&dest_ip, &netif->ipaddr, &netif->netmask)) {
    dbg_error(DBG_IP, "ipaddr not match");
    return NET_ERR_UNREACH;
  }

  if (pkt->hdr.frag_offset || pkt->hdr.more) {
    err = ip_frag_in(netif, buf, &src_ip, &dest_ip);
  } else {
    err = ip_normal_in(netif, buf, &src_ip, &dest_ip);
  }

  return err;
}

net_err_t ipv4_out(uint8_t protocol, ipaddr_t *dest, ipaddr_t *src,
                   pktbuf_t *buf) {
  dbg_info(DBG_IP, "send an ip pkt");

  net_err_t err = pktbuf_add_header(buf, sizeof(ipv4_hdr_t), 1);

  if (err < 0) {
    dbg_error(DBG_IP, "add header failed");
    return NET_ERR_SIZE;
  }

  ipv4_pkt_t *pkt = (ipv4_pkt_t *)pktbuf_data(buf);
  pkt->hdr.shdr_all = 0;
  pkt->hdr.version = NET_VERSION_IPV4;
  ipv4_set_hdr_size(pkt, sizeof(ipv4_hdr_t));
  pkt->hdr.total_len = buf->total_size;
  pkt->hdr.id = packet_id++;
  pkt->hdr.frag_all = 0;
  pkt->hdr.ttl = NET_IP_DEFAULT_TTL;
  pkt->hdr.protocol = protocol;
  pkt->hdr.hdr_checksum = 0;
  ipaddr_to_buf(src, pkt->hdr.src_ip);
  ipaddr_to_buf(dest, pkt->hdr.dest_ip);

  iphdr_htons(pkt);
  pktbuf_reset_acc(buf);
  pkt->hdr.hdr_checksum = pktbuf_checksum16(buf, ipv4_hdr_size(pkt), 0, 1);

  display_ip_pkt(pkt);

  err = netif_out(netif_get_default(), dest, buf);

  if (err < 0) {
    dbg_warning(DBG_IP, "send ip packet failed.");
    return err;
  }

  return NET_ERR_OK;
}