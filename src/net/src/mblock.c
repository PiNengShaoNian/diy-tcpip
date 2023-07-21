#include "mblock.h"

#include "dbg.h"
#include "nlist.h"

net_err_t mblock_init(mblock_t *mblock, void *mem, int blk_size, int cnt,
                      nlocker_type_t nlocker_type) {
  uint8_t *buf = (uint8_t *)mem;

  nlist_init(&mblock->free_list);
  for (int i = 0; i < cnt; i++, buf += blk_size) {
    nlist_node_t *block = (nlist_node_t *)buf;

    nlist_node_init(block);
    nlist_insert_last(&mblock->free_list, block);
  }

  nlocker_init(&mblock->locker, nlocker_type);

  if (nlocker_type != NLOCKER_NONE) {
    mblock->alloc_sem = sys_sem_create(cnt);
    if (mblock->alloc_sem == SYS_SEM_INVALID) {
      dbg_error(DBG_MBLOCK, "create sem failed.");
      nlocker_destroy(&mblock->locker);
      return NET_ERR_SYS;
    }
  }

  return NET_ERR_OK;
}