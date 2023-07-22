﻿#include <stdio.h>

#include "dbg.h"
#include "echo/tcp_echo_client.h"
#include "echo/tcp_echo_server.h"
#include "mblock.h"
#include "net.h"
#include "nlist.h"
#include "pktbuf.h"
#include "sys_plat.h"

static sys_sem_t sem;
static sys_mutex_t mutex;
static int count;

static char buffer[100];
static int write_index, read_index, total;
static sys_sem_t read_sem;
static sys_sem_t write_sem;

void thread1_entry(void *arg) {
  for (int i = 0; i < 2 * sizeof(buffer); i++) {
    sys_sem_wait(read_sem, 0);

    uint8_t data = buffer[read_index++];

    if (read_index >= sizeof(buffer)) {
      read_index = 0;
    }

    sys_mutex_lock(mutex);
    total--;
    sys_mutex_unlock(mutex);

    plat_printf("thread 1: read data=%d\n", data);
    sys_sem_notify(write_sem);
    sys_sleep(100);
  }

  plat_printf("count: %d\n", count);

  while (1) {
    plat_printf("this is thread1: %s\n", (char *)arg);
    sys_sleep(1000);
    sys_sem_notify(sem);
    sys_sleep(1000);
  }
}

void thread2_entry(void *arg) {
  sys_sleep(100);
  for (int i = 0; i < 2 * sizeof(buffer); i++) {
    sys_sem_wait(write_sem, 0);
    buffer[write_index++] = i;

    if (write_index >= sizeof(buffer)) {
      write_index = 0;
    }

    sys_mutex_lock(mutex);
    total++;
    sys_mutex_unlock(mutex);

    plat_printf("thread 2: write data = %d\n", i);
    sys_sem_notify(read_sem);
  }

  while (1) {
    sys_sem_wait(sem, 0);
    plat_printf("this is thread2: %s\n", (char *)arg);
  }
}

#include "netif_pcap.h"

net_err_t netdev_init(void) {
  netif_pcp_open();

  return NET_ERR_OK;
}

typedef struct _tnode_t {
  int id;
  nlist_node_t node;
} tnode_t;

void nlist_test(void) {
#define NODE_CNT 4

  tnode_t node[NODE_CNT];
  nlist_t list;

  plat_printf("insert first\n");
  nlist_init(&list);
  for (int i = 0; i < NODE_CNT; i++) {
    node[i].id = i;
    nlist_insert_first(&list, &node[i].node);
  }

  nlist_node_t *p;
  nlist_for_each(p, &list) {
    tnode_t *tnode = nlist_entry(p, tnode_t, node);
    plat_printf("%d\n", tnode->id);
  }

  plat_printf("remove first\n");
  for (int i = 0; i < NODE_CNT; i++) {
    p = nlist_remove_first(&list);
    tnode_t *tnode = nlist_entry(p, tnode_t, node);
    plat_printf("%d\n", tnode->id);
  }

  plat_printf("insert last\n");
  nlist_init(&list);
  for (int i = 0; i < NODE_CNT; i++) {
    node[i].id = i;
    nlist_insert_last(&list, &node[i].node);
  }

  nlist_for_each(p, &list) {
    tnode_t *tnode = nlist_entry(p, tnode_t, node);
    plat_printf("%d\n", tnode->id);
  }

  plat_printf("remove last\n");
  for (int i = 0; i < NODE_CNT; i++) {
    p = nlist_remove_last(&list);
    tnode_t *tnode = nlist_entry(p, tnode_t, node);
    plat_printf("%d\n", tnode->id);
  }

  plat_printf("insert after\n");
  for (int i = 0; i < NODE_CNT; i++) {
    node[i].id = i;
    nlist_insert_after(&list, nlist_first(&list), &node[i].node);
  }

  nlist_for_each(p, &list) {
    tnode_t *tnode = nlist_entry(p, tnode_t, node);
    plat_printf("%d\n", tnode->id);
  }
}

void mblock_test(void) {
  mblock_t blist;
  static uint8_t buffer[100][10];

  mblock_init(&blist, buffer, 100, 10, NLOCKER_THREAD);

  void *temp[10];
  for (int i = 0; i < 10; i++) {
    temp[i] = mblock_alloc(&blist, 0);

    plat_printf("block: %p, free_count: %d\n", temp[i],
                mblock_free_cnt(&blist));
  }

  for (int i = 0; i < 10; i++) {
    mblock_free(&blist, temp[i]);
    plat_printf("free count: %d\n", mblock_free_cnt(&blist));
  }

  mblock_destroy(&blist);
}

void pktbuf_test(void) {
  pktbuf_t *buf = pktbuf_alloc(2000);
  pktbuf_free(buf);

  buf = pktbuf_alloc(2000);
  for (int i = 0; i < 16; i++) {
    pktbuf_add_header(buf, 33, 1);
  }

  for (int i = 0; i < 16; i++) {
    pktbuf_remove_header(buf, 33);
  }

  for (int i = 0; i < 16; i++) {
    pktbuf_add_header(buf, 33, 0);
  }

  for (int i = 0; i < 16; i++) {
    pktbuf_remove_header(buf, 33);
  }

  pktbuf_free(buf);

  buf = pktbuf_alloc(8);
  pktbuf_resize(buf, 32);
  pktbuf_resize(buf, 288);
  pktbuf_resize(buf, 4922);
}

void basic_test(void) {
  nlist_test();
  mblock_test();
  pktbuf_test();
}

#define DBG_TEST DBG_LEVEL_INFO

int main(int argc, char **argv) {
  dbg_info(DBG_TEST, "info");
  dbg_warning(DBG_TEST, "warning");
  dbg_error(DBG_TEST, "error");

  dbg_assert(1 == 1, "failed");

  net_init();

  basic_test();

  net_start();

  netdev_init();

  while (1) {
    sys_sleep(10);
  }

  return 0;
}