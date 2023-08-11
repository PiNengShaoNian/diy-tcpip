#ifndef DNS_H
#define DNS_H

#include "exmsg.h"
#include "ipaddr.h"
#include "net_cfg.h"
#include "net_err.h"
#include "nlist.h"
#include "sys.h"
#include "udp.h"

#define DNS_PORT_DEFAULT 53
#define DNS_QUERY_ClASS_INET 1
#define DNS_QUERY_TYPE_A 1

#pragma pack(1)
typedef struct _dns_hdr_t {
  uint16_t id;  // 事务ID
  union {
    uint16_t all;

#if NET_ENDIAN_LITTLE
    struct {
      uint16_t rcode : 4;  // 响应码
      uint16_t cd : 1;     // 禁用安全检查（1）
      uint16_t ad : 1;     // 信息已授权（1）
      uint16_t z : 1;      // 保底为0
      uint16_t ra : 1;     // 服务器是否支持递归查询(1)
      uint16_t rd : 1;  // 告诉服务器执行递归查询(1)，0 容许迭代查询
      uint16_t tc : 1;  // 可截断(1)，即UDP长超512字节时，可只返回512字节
      uint16_t aa : 1;      // 授权回答
      uint16_t opcode : 4;  // 操作码(缺省为0)
      uint16_t qr : 1;
    };
#else
    struct {
      uint16_t qr : 1;
      uint16_t opcode : 4;  // 操作码(缺省为0)
      uint16_t aa : 1;      // 授权回答
      uint16_t tc : 1;  // 可截断(1)，即UDP长超512字节时，可只返回512字节
      uint16_t rd : 1;  // 告诉服务器执行递归查询(1)，0 容许迭代查询
      uint16_t ra : 1;     // 服务器是否支持递归查询(1)
      uint16_t z : 1;      // 保底为0
      uint16_t ad : 1;     // 信息已授权（1）
      uint16_t cd : 1;     // 禁用安全检查（1）
      uint16_t rcode : 5;  // 响应码
    };
#endif
  } flags;
  uint16_t qdcount;  // 查询数/区域数
  uint16_t ancount;  // 回答/先决条件数
  uint16_t nscount;  // 授权纪录数/更新数
  uint16_t arcount;  // 额外信息数
} dns_hdr_t;

typedef struct _dns_qfield_t {
  uint16_t type;
  uint16_t class;
} dns_qfield_t;

#pragma pack()

typedef struct _dns_entry_t {
  ipaddr_t ipaddr;
  char domain_name[DNS_DOMAIN_NAME_MAX];
} dns_entry_t;

typedef struct _dns_req_t {
  char domain_name[DNS_DOMAIN_NAME_MAX];
  net_err_t err;
  ipaddr_t ipaddr;
  sys_sem_t wait_sem;

  int query_id;

  nlist_node_t node;
} dns_req_t;

void dns_init(void);

dns_req_t *dns_alloc_req(void);
void dns_free_req(dns_req_t *req);

net_err_t dns_req_in(func_msg_t *msg);

int dns_is_arrive(udp_t *udp);
void dns_in(void);

#endif