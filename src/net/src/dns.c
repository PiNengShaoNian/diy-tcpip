#include "dns.h"

#include "dbg.h"
#include "mblock.h"
#include "net_cfg.h"
#include "nlist.h"

static nlist_t req_list;
static mblock_t req_mblock;
static dns_req_t dns_req_tbl[DNS_REQ_SIZE];

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

net_err_t dns_req_in(func_msg_t *msg) { return NET_ERR_OK; }