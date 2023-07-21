#include <stdio.h>

#include "dbg.h"
#include "echo/tcp_echo_client.h"
#include "echo/tcp_echo_server.h"
#include "net.h"
#include "nlist.h"
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

  nlist_init(&list);
  for (int i = 0; i < NODE_CNT; i++) {
    node[i].id = i;
    nlist_insert_first(&list, &node[i].node);
  }
}

void basic_test(void) { nlist_test(); }

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