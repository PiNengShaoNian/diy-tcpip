#ifndef DNS_H
#define DNS_H

#include "exmsg.h"
#include "ipaddr.h"
#include "net_cfg.h"
#include "net_err.h"
#include "sys.h"

typedef struct _dns_req_t {
  char domain_name[DNS_DOMAIN_NAME_MAX];
  net_err_t err;
  ipaddr_t ipaddr;
  sys_sem_t wait_sem;
} dns_req_t;

void dns_init(void);

dns_req_t *dns_alloc_req(void);
void dns_free_req(dns_req_t *req);

net_err_t dns_req_in(func_msg_t *msg);

#endif