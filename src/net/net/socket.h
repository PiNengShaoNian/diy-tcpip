#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>

#include "ipv4.h"

#undef INADDR_ANY
#define INADDR_ANY 0x00000000

#undef AF_INET
#define AF_INET 2

struct x_in_addr {
  union {
    struct {
      uint8_t addr0;
      uint8_t addr1;
      uint8_t addr2;
      uint8_t addr3;
    };

    uint8_t addr_array[IPV4_ADDR_SIZE];
#undef s_addr
    uint32_t s_addr;
  };
};

struct x_sockaddr_in {
  uint8_t sin_len;
  uint8_t sin_family;
  uint16_t sin_port;
  struct x_in_addr sin_addr;
  char sin_zero[8];
};

#endif