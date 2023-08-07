#include "tcp_buf.h"

void tcp_buf_init(tcp_buf_t *buf, uint8_t *data, int size) {
  buf->in = buf->out = 0;
  buf->count = 0;
  buf->size = size;
  buf->data = data;
}