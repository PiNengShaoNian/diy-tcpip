#include "ipaddr.h"

void ipaddr_set_any(ipaddr_t* ip) {
  ip->type = IPADDR_V4;
  ip->q_addr = 0;
}