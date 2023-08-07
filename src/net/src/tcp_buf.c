#include "tcp_buf.h"

#include "dbg.h"

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

void tcp_buf_read_send(tcp_buf_t *buf, int offset, pktbuf_t *dest, int count) {
  int free_cnt = buf->count - offset;
  if (count > free_cnt) {
    dbg_warning(DBG_TCP, "resize for send: %d -> %d", count, free_cnt);
    count = free_cnt;
  }

  int start = buf->out + offset;
  if (start >= buf->size) {
    start -= buf->size;
  }

  while (count > 0) {
    int end = start + count;
    if (end >= buf->size) {
      end = buf->size;
    }

    int copy_size = (int)(end - start);

    net_err_t err = pktbuf_write(dest, buf->data + start, (int)copy_size);
    dbg_assert(err >= 0, "write buffer failed.");

    start += copy_size;
    if (start >= buf->size) {
      start -= buf->size;
    }
    count -= copy_size;
  }
}