#include "tcp_buf.h"

void tcp_buf_init(tcp_buf_t *buf, uint8_t *data, int size) {
  buf->in = buf->out = 0;
  buf->count = 0;
  buf->size = size;
  buf->data = data;
}

void tcp_buf_write_send(tcp_buf_t *buf, const uint8_t *buffer, int len) {
  while (len > 0) {
    buf->data[buf->in++] = *buffer++;
    if (buf->in >= buf->size) {
      buf->in = 0;
    }
    len--;
    buf->count++;
  }
}