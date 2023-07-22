#include "pktbuf.h"

#include "dbg.h"
#include "mblock.h"
#include "nlocker.h"

static nlocker_t locker;
static pktblk_t block_buffer[PKTBUF_BLK_CNT];
static mblock_t block_list;
static pktbuf_t pktbuf_buffer[PKTBUF_BUF_CNT];
static mblock_t pktbuf_list;

net_err_t pktbuf_init(void) {
  dbg_info(DBG_BUF, "init pktbuf");
  nlocker_init(&locker, NLOCKER_THREAD);
  mblock_init(&block_list, block_buffer, sizeof(pktblk_t), PKTBUF_BLK_CNT,
              NLOCKER_THREAD);
  mblock_init(&pktbuf_list, pktbuf_buffer, sizeof(pktbuf_t), PKTBUF_BUF_CNT,
              NLOCKER_THREAD);

  dbg_info(DBG_BUF, "init done");
  return NET_ERR_OK;
}
