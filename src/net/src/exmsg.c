#include "exmsg.h"

#include "dbg.h"
#include "fixq.h"
#include "mblock.h"
#include "sys_plat.h"

static void *msg_tbl[EXMSG_MSG_CNT];
static fixq_t msg_queue;

static exmsg_t msg_buffer[EXMSG_MSG_CNT];
static mblock_t msg_block;

net_err_t exmsg_init(void) {
  dbg_info(DBG_MSG, "exmsg init");

  net_err_t err = fixq_init(&msg_queue, msg_tbl, EXMSG_MSG_CNT, EXMSG_LOCKER);
  if (err < 0) {
    dbg_error(DBG_MSG, "fixq init failed");
    return err;
  }

  err = mblock_init(&msg_block, msg_buffer, sizeof(exmsg_t), EXMSG_MSG_CNT,
                    EXMSG_LOCKER);

  if (err < 0) {
    dbg_error(DBG_MSG, "mblock init failed.");
    return err;
  }

  dbg_info(DBG_MSG, "init done");
  return NET_ERR_OK;
}

static void work_thread(void *arg) {
  plat_printf("exmsg running...\n");

  while (1) {
    sys_sleep(1);
  }
}

net_err_t exmsg_start(void) {
  sys_thread_t thread = sys_thread_create(work_thread, (void *)0);

  if (thread == SYS_THREAD_INVALID) {
    return NET_ERR_SYS;
  }

  return NET_ERR_OK;
}