#ifndef TCP_BUFF_H
#define TCP_BUFF_H

#include <stdint.h>

typedef struct _tcp_buf_t {
  uint8_t *data;
  int count;
  int size;
  int in, out;
} tcp_buf_t;

void tcp_buf_init(tcp_buf_t *buf, uint8_t *data, int size);

static inline int tcp_buf_size(tcp_buf_t *buf) { return buf->size; }

static inline int tcp_buf_cnt(tcp_buf_t *buf) { return buf->count; }

static inline int tcp_buf_free_cout(tcp_buf_t *buf) {
  return buf->size - buf->count;
}

#endif