#include "fixq.h"

#include "dbg.h"

net_err_t fixq_init(fixq_t *q, void **buf, int size, nlocker_type_t type) {
  q->size = size;
  q->in = q->out = q->cnt = 0;
  q->buf = buf;
  q->send_sem = SYS_SEM_INVALID;
  q->recv_sem = SYS_SEM_INVALID;

  net_err_t err = nlocker_init(&q->locker, type);
  if (err < 0) {
    dbg_error(DBG_QUEUE, "init locker failed.");
    return err;
  }

  q->send_sem = sys_sem_create(size);
  if (q->send_sem == SYS_SEM_INVALID) {
    dbg_error(DBG_QUEUE, "create sem failed");
    err = NET_ERR_SYS;
    goto init_failed;
  }

  q->recv_sem = sys_sem_create(0);
  if (q->recv_sem == SYS_SEM_INVALID) {
    dbg_error(DBG_QUEUE, "create sem failed");
    err = NET_ERR_SYS;
    goto init_failed;
  }

  return NET_ERR_OK;

init_failed:
  if (q->send_sem != SYS_SEM_INVALID) {
    sys_sem_free(q->send_sem);
  }

  if (q->recv_sem != SYS_SEM_INVALID) {
    sys_sem_free(q->recv_sem);
  }

  nlocker_destroy(&q->locker);
  return err;
}