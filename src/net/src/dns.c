#include "dns.h"

#include "dbg.h"
#include "mblock.h"
#include "net_api.h"
#include "net_cfg.h"
#include "nlist.h"
#include "socket.h"
#include "tools.h"
#include "udp.h"

static nlist_t req_list;
static mblock_t req_mblock;
static dns_req_t dns_req_tbl[DNS_REQ_SIZE];

static char working_buf[DNS_WORKING_BUF_SIZE];
static udp_t *dns_udp;

void dns_init(void) {
  dbg_info(DBG_DNS, "DNS init");

  nlist_init(&req_list);
  mblock_init(&req_mblock, dns_req_tbl, sizeof(dns_req_t), DNS_REQ_SIZE,
              NLOCKER_THREAD);
  dns_udp = (udp_t *)udp_create(AF_INET, IPPROTO_UDP);
  dbg_assert(dns_udp != (udp_t *)0, "create dns socket failed.");

  dbg_info(DBG_DNS, "DNS init done");
}

dns_req_t *dns_alloc_req(void) { return mblock_alloc(&req_mblock, 0); }

void dns_free_req(dns_req_t *req) { mblock_free(&req_mblock, req); }

static void dns_req_add(dns_req_t *req) {
  static int id = 0;
  req->query_id = ++id;
  req->err = NET_ERR_OK;
  ipaddr_set_any(&req->ipaddr);

  nlist_insert_last(&req_list, &req->node);
}

static uint8_t *add_query_field(const char *domain_name, char *buf,
                                size_t size) {
  if (size < plat_strlen(domain_name) + 2 + 4) {
    dbg_error(DBG_DNS, "no enough space");
    return (uint8_t *)0;
  }

  char *name_buf = buf;
  name_buf[0] = '.';
  plat_strcpy(name_buf + 1, domain_name);

  char *c = name_buf;
  while (*c) {
    if (*c == '.') {
      char *dot = c++;

      while (*c && (*c != '.')) {
        c++;
      }
      *dot = (uint8_t)(c - dot - 1);
    } else {
      c++;
    }
  }

  *c++ = '\0';
  dns_qfield_t *f = (dns_qfield_t *)c;
  f->class = x_htons(DNS_QUERY_ClASS_INET);
  f->type = x_htons(DNS_QUERY_TYPE_A);
  return (uint8_t *)f + sizeof(dns_qfield_t);
}

static net_err_t dns_send_query(dns_req_t *req) {
  dns_hdr_t *dns_hdr = (dns_hdr_t *)working_buf;
  dns_hdr->id = x_htons(req->query_id);
  dns_hdr->flags.all = 0;
  dns_hdr->flags.qr = 0;
  dns_hdr->flags.rd = 1;
  dns_hdr->flags.all = x_htons(dns_hdr->flags.all);
  dns_hdr->qdcount = x_htons(1);
  dns_hdr->ancount = 0;
  dns_hdr->nscount = 0;
  dns_hdr->arcount = 0;

  uint8_t *buf = working_buf + sizeof(dns_hdr_t);
  buf = add_query_field(req->domain_name, buf,
                        sizeof(working_buf) - (buf - working_buf));

  if (!buf) {
    dbg_error(DBG_DNS, "add query failed.");
    return NET_ERR_MEM;
  }

  struct x_sockaddr_in dest;
  plat_memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = x_htons(DNS_PORT_DEFAULT);
  dest.sin_addr.s_addr = x_inet_addr("8.8.8.8");
  return udp_sendto((sock_t *)dns_udp, working_buf, buf - working_buf, 0,
                    (const struct x_sockaddr *)&dest, sizeof(dest),
                    (ssize_t *)0);
}

static dns_entry_t *dns_entry_find(const char *domain_name) {
  return (dns_entry_t *)0;
}

int dns_is_arrive(udp_t *udp) { return udp == dns_udp; }

static const char *domain_name_cmp(const char *domain_name, const char *name,
                                   size_t size) {
  const char *src = domain_name;
  const char *dest = name;

  while (*src) {
    int cnt = *dest++;
    for (int i = 0; i < cnt && *src && *dest; i++) {
      if (*dest++ != *src++) {
        return (const char *)0;
      }
    }

    if (*src == '\0') {
      break;
    } else if (*src++ != '.') {
      return (const char *)0;
    }
  }

  return (dest >= name + size) ? (const char *)0 : dest + 1;
}

void dns_in(void) {
  ssize_t rcv_len;
  struct x_sockaddr_in src;
  x_socklen_t addr_len;
  net_err_t err =
      udp_recvfrom((sock_t *)dns_udp, working_buf, sizeof(working_buf), 0,
                   (struct x_sockaddr *)&src, &addr_len, &rcv_len);
  if (err < 0) {
    dbg_error(DBG_DNS, "rcv udp error");
    return;
  }

  const uint8_t *rcv_start = working_buf;
  const uint8_t *rcv_end = working_buf + rcv_len;
  dns_hdr_t *dns_hdr = (dns_hdr_t *)working_buf;
  dns_hdr->id = x_ntohs(dns_hdr->id);
  dns_hdr->flags.all = x_ntohs(dns_hdr->flags.all);
  dns_hdr->qdcount = x_ntohs(dns_hdr->qdcount);
  dns_hdr->ancount = x_ntohs(dns_hdr->ancount);
  dns_hdr->nscount = x_ntohs(dns_hdr->nscount);
  dns_hdr->arcount = x_ntohs(dns_hdr->arcount);

  nlist_node_t *curr;
  nlist_for_each(curr, &req_list) {
    dns_req_t *req = nlist_entry(curr, dns_req_t, node);
    if (req->query_id != dns_hdr->id) {
      continue;
    }

    if (dns_hdr->flags.qr == 0) {
      dbg_warning(DBG_DNS, "not a response");
      goto req_failed;
    }

    if (dns_hdr->flags.tc == 1) {
      dbg_warning(DBG_DNS, "truncated");
      goto req_failed;
    }

    if (dns_hdr->flags.ra == 0) {
      dbg_warning(DBG_DNS, "recursion not support");
      goto req_failed;
    }

    switch (dns_hdr->flags.rcode) {
      // 以下应当更换服务器
      case DNS_ERR_NONE:
        // 没有错误
        break;
      case DNS_ERR_NOTIMP:
        dbg_warning(DBG_DNS, "server reply: not support");
        err = NET_ERR_NOT_SUPPORT;
        goto req_failed;
      case DNS_ERR_REFUSED:
        dbg_warning(DBG_DNS, "server reply: refused");
        err = NET_ERR_REFUSED;
        goto req_failed;
      case DNS_ERR_SERV_FAIL:
        dbg_warning(DBG_DNS, "server reply: server failure");
        err = NET_ERR_SERVER_FAILURE;
        goto req_failed;
      case DNS_ERR_NXMOMAIN:
        dbg_warning(DBG_DNS, "server reply: domain not exist");
        err = NET_ERR_NOT_EXIST;
        goto req_failed;
      // 以下直接删除请求
      case DNS_ERR_FORMAT:
        dbg_warning(DBG_DNS, "server reply: format error");
        err = NET_ERR_FORMAT;
        goto req_failed;
      default:
        dbg_warning(DBG_DNS, "unknow error");
        err = NET_ERR_UNKNOWN;
        goto req_failed;
    }

    if (dns_hdr->qdcount == 1) {
      rcv_start += sizeof(dns_hdr_t);
      rcv_start = domain_name_cmp(req->domain_name, (const char *)rcv_start,
                                  rcv_end - rcv_start);
      if (rcv_start == (uint8_t *)0) {
        dbg_warning(DBG_DNS, "domain name not match");
        err = NET_ERR_BROKEN;
        goto req_failed;
      }

      if (rcv_start + sizeof(dns_qfield_t) > rcv_end) {
        dbg_warning(DBG_DNS, "size error");
        err = NET_ERR_SIZE;
        goto req_failed;
      }

      dns_qfield_t *qf = (dns_qfield_t *)rcv_start;
      if (qf->class != x_ntohs(DNS_QUERY_ClASS_INET)) {
        dbg_warning(DBG_DNS, "query class not inet");
        err = NET_ERR_BROKEN;
        goto req_failed;
      }

      if (qf->type != x_ntohs(DNS_QUERY_TYPE_A)) {
        dbg_warning(DBG_DNS, "query type not A");
        err = NET_ERR_BROKEN;
        goto req_failed;
      }

      rcv_start += sizeof(dns_qfield_t);
    }

    if (dns_hdr->ancount < 1) {
      dbg_warning(DBG_DNS, "query answer == 0");
      err = NET_ERR_NONE;
      goto req_failed;
    }
  }

req_failed:
  return;
}

net_err_t dns_req_in(func_msg_t *msg) {
  dns_req_t *dns_req = (dns_req_t *)msg->param;

  if (plat_strcmp(dns_req->domain_name, "localhost") == 0) {
    ipaddr_from_str(&dns_req->ipaddr, "127.0.0.1");
    dns_req->err = NET_ERR_OK;
    return NET_ERR_OK;
  }

  ipaddr_t ipaddr;
  if (ipaddr_from_str(&ipaddr, dns_req->domain_name) == NET_ERR_OK) {
    ipaddr_copy(&dns_req->ipaddr, &ipaddr);
    dns_req->err = NET_ERR_OK;
    return NET_ERR_OK;
  }

  dns_entry_t *entry = dns_entry_find(dns_req->domain_name);
  if (entry) {
    ipaddr_copy(&dns_req->ipaddr, &entry->ipaddr);
    dns_req->err = NET_ERR_OK;
    return NET_ERR_OK;
  }

  dns_req->wait_sem = sys_sem_create(0);
  if (dns_req->wait_sem == SYS_SEM_INVALID) {
    dbg_error(DBG_DNS, "create sem failed.");
    return NET_ERR_SYS;
  }
  dns_req_add(dns_req);
  net_err_t err = dns_send_query(dns_req);
  if (err < 0) {
    dbg_error(DBG_DNS, "snd dns query failed.");
    goto dns_req_end;
  }

  return NET_ERR_OK;
dns_req_end:
  if (dns_req->wait_sem != SYS_SEM_INVALID) {
    sys_sem_free(dns_req->wait_sem);
    dns_req->wait_sem = SYS_SEM_INVALID;
  }
  return err;
}
