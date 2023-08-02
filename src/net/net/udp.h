#ifndef UDP_H
#define UDP_H

#include "pktbuf.h"
#include "sock.h"

#pragma pack(1)

typedef struct _udp_hdr_t {
  uint16_t src_port;
  uint16_t dest_port;
  uint16_t total_len;
  uint16_t checksum;
} udp_hdr_t;

typedef struct _udp_pkt_t {
  udp_hdr_t hdr;
  uint8_t data[1];
} udp_pkt_t;

#pragma pack()

typedef struct _udp_t {
  sock_t base;

  nlist_t recv_list;

  sock_wait_t recv_wait;
} udp_t;

net_err_t udp_init(void);
sock_t *udp_create(int family, int protocol);
net_err_t udp_out(ipaddr_t *dest_ip, uint16_t dest_port, ipaddr_t *src_ip,
                  uint16_t src_port, pktbuf_t *buf);

#endif