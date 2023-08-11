#include "dns.h"

#include "dbg.h"
#include "mblock.h"
#include "net_cfg.h"
#include "nlist.h"
#include "tools.h"

static nlist_t req_list;
static mblock_t req_mblock;
static dns_req_t dns_req_tbl[DNS_REQ_SIZE];

static char working_buf[DNS_WORKING_BUF_SIZE];

void dns_init(void) {
  dbg_info(DBG_DNS, "DNS init");

  nlist_init(&req_list);
  mblock_init(&req_mblock, dns_req_tbl, sizeof(dns_req_t), DNS_REQ_SIZE,
              NLOCKER_THREAD);

  dbg_info(DBG_DNS, "DNS init done");
}

dns_req_t *dns_alloc_req(void) {
  static dns_req_t req;
  return &req;
}

void dns_free_req(dns_req_t *req) {}

static void dns_req_add(dns_req_t *req) {}

static net_err_t dns_send_query(dns_req_t *req) {
  dns_hdr_t *dns_hdr = (dns_hdr_t *)working_buf;
  dns_hdr->id = x_htons(0);
  dns_hdr->flags.all = 0;
  dns_hdr->flags.qr = 0;
  dns_hdr->flags.rd = 1;
  dns_hdr->qdcount = x_htons(1);
  dns_hdr->ancount = 0;
  dns_hdr->nscount = 0;
  dns_hdr->arcount = 0;

  return NET_ERR_OK;
}

static dns_entry_t *dns_entry_find(const char *domain_name) {
  return (dns_entry_t *)0;
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