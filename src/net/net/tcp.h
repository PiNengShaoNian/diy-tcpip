#ifndef TCP_H
#define TCP_H

#include "net_err.h"
#include "nlist.h"
#include "sock.h"

#pragma pack(1)
typedef struct _tcp_hdr_t {
  uint16_t sport;
  uint16_t dport;
  uint32_t seq;
  uint32_t ack;
  union {
    uint16_t flags;
#if NET_ENDIAN_LITTLE
    struct {
      uint16_t resv : 4;
      uint16_t shdr : 4;
      uint16_t f_fin : 1;
      uint16_t f_syn : 1;
      uint16_t f_rst : 1;
      uint16_t f_psh : 1;
      uint16_t f_ack : 1;
      uint16_t f_urg : 1;
      uint16_t f_ece : 1;
      uint16_t f_cwr : 1;
    };
#else
    struct {
      uint16_t shdr : 4;
      uint16_t resv : 4;
      uint16_t f_cwr : 1;
      uint16_t f_ece : 1;
      uint16_t f_urg : 1;
      uint16_t f_ack : 1;
      uint16_t f_psh : 1;
      uint16_t f_rst : 1;
      uint16_t f_syn : 1;
      uint16_t f_fin : 1;
    };
#endif
  };
  uint16_t win;
  uint16_t checksum;
  uint16_t urgptr;
} tcp_hdr_t;

typedef struct _tcp_pkt_t {
  tcp_hdr_t hdr;
  uint8_t data[1];
} tcp_pkt_t;

#pragma pack()

typedef struct _tcp_t {
  sock_t base;

  nlist_t recv_list;

  sock_wait_t recv_wait;
} tcp_t;

net_err_t tcp_init(void);
sock_t *tcp_create(int family, int protocol);
static inline int tcp_hdr_size(tcp_hdr_t *hdr) { return hdr->shdr * 4; }

#endif