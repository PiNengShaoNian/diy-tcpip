#include "dns.h"

dns_req_t *dns_alloc_req(void) {
  static dns_req_t req;
  return &req;
}

void dns_free_req(dns_req_t *req) {}

net_err_t dns_req_in(func_msg_t *msg) { return NET_ERR_OK; }