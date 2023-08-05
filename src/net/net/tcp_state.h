#ifndef TCP_STATE_T
#define TCP_STATE_T

#include "tcp.h"

const char *tcp_state_name(tcp_state_t state);
void tcp_set_state(tcp_t *tcp, tcp_state_t state);

#endif