#include <stdio.h>

#include "dbg.h"
#include "echo/tcp_echo_client.h"
#include "echo/tcp_echo_server.h"
#include "mblock.h"
#include "net.h"
#include "netif.h"
#include "nlist.h"
#include "pktbuf.h"
#include "sys_plat.h"
#include "timer.h"
#include "tools.h"

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

pcap_data_t netdev0_data = {.ip = netdev0_phy_ip, .hwaddr = netdev0_hwaddr};

net_err_t netdev_init(void) {
  netif_t *netif =
      netif_open("netif 0", (netif_ops_t *)&netdev_ops, &netdev0_data);
  if (!netif) {
    dbg_error(DBG_NETIF, "open netif 0 err");
    return NET_ERR_NONE;
  }

  ipaddr_t ip, mask, gw;
  ipaddr_from_str(&ip, netdev0_ip);
  ipaddr_from_str(&mask, netdev0_mask);
  ipaddr_from_str(&gw, netdev0_gw);

  netif_set_addr(netif, &ip, &mask, &gw);

  netif_set_active(netif);

  // pktbuf_t *buf = pktbuf_alloc(32);
  // pktbuf_fill(buf, 0x53, 32);

  // ipaddr_t dest;
  // ipaddr_from_str(&dest, friend0_ip);
  // netif_out(netif, &dest, buf);

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
  pktbuf_resize(buf, 1921);
  pktbuf_resize(buf, 288);
  pktbuf_resize(buf, 32);
  pktbuf_resize(buf, 0);
  pktbuf_free(buf);

  buf = pktbuf_alloc(689);
  pktbuf_t *sbuf = pktbuf_alloc(892);
  pktbuf_join(buf, sbuf);
  pktbuf_free(buf);

  buf = pktbuf_alloc(32);
  pktbuf_join(buf, pktbuf_alloc(4));
  pktbuf_join(buf, pktbuf_alloc(16));
  pktbuf_join(buf, pktbuf_alloc(54));
  pktbuf_join(buf, pktbuf_alloc(32));
  pktbuf_join(buf, pktbuf_alloc(38));

  pktbuf_set_cont(buf, 44);
  pktbuf_set_cont(buf, 60);
  pktbuf_set_cont(buf, 44);
  pktbuf_set_cont(buf, 128);
  pktbuf_set_cont(buf, 135);

  pktbuf_free(buf);

  buf = pktbuf_alloc(32);
  pktbuf_join(buf, pktbuf_alloc(4));
  pktbuf_join(buf, pktbuf_alloc(16));
  pktbuf_join(buf, pktbuf_alloc(54));
  pktbuf_join(buf, pktbuf_alloc(32));
  pktbuf_join(buf, pktbuf_alloc(38));
  pktbuf_join(buf, pktbuf_alloc(512));

  pktbuf_reset_acc(buf);
  static uint16_t temp[1000];
  for (int i = 0; i < 1000; i++) {
    temp[i] = i;
  }
  pktbuf_write(buf, (uint8_t *)temp, pktbuf_total(buf));

  static uint16_t read_temp[1000];
  plat_memset(read_temp, 0, sizeof(read_temp));

  pktbuf_reset_acc(buf);
  pktbuf_read(buf, (uint8_t *)read_temp, pktbuf_total(buf));
  if (plat_memcmp(temp, read_temp, pktbuf_total(buf)) != 0) {
    plat_printf("not equal");
    return;
  }

  plat_memset(read_temp, 0, sizeof(read_temp));
  pktbuf_seek(buf, 18 * 2);
  pktbuf_read(buf, (uint8_t *)read_temp, 56);
  if (plat_memcmp(temp + 18, read_temp, 56) != 0) {
    plat_printf("not equal");
    return;
  }

  plat_memset(read_temp, 0, sizeof(read_temp));
  pktbuf_seek(buf, 85 * 2);
  pktbuf_read(buf, (uint8_t *)read_temp, 256);
  if (plat_memcmp(temp + 85, read_temp, 256) != 0) {
    plat_printf("not equal");
    return;
  }

  pktbuf_t *dest = pktbuf_alloc(1024);
  pktbuf_seek(dest, 600);
  pktbuf_seek(buf, 200);
  pktbuf_copy(dest, buf, 122);

  plat_memset(read_temp, 0, sizeof(read_temp));
  pktbuf_seek(dest, 600);
  pktbuf_read(dest, (uint8_t *)read_temp, 122);
  if (plat_memcmp(temp + 100, read_temp, 122) != 0) {
    plat_printf("not equal");
    return;
  }

  pktbuf_seek(dest, 0);
  pktbuf_fill(dest, 0x53, pktbuf_total(dest));
  plat_memset(read_temp, 0, sizeof(read_temp));
  pktbuf_seek(dest, 0);
  pktbuf_read(dest, (uint8_t *)read_temp, pktbuf_total(dest));

  char *ptr = (char *)read_temp;
  for (int i = 0; i < pktbuf_total(dest); i++) {
    if (*ptr++ != 0x53) {
      plat_printf("not equal");
      return;
    }
  }

  pktbuf_free(dest);
  pktbuf_free(buf);
}

void timer0_proc(struct _net_timer_t *timer, void *arg) {
  static int count = 1;
  plat_printf("this is %s: %d\n", timer->name, count++);
}

void timer1_proc(struct _net_timer_t *timer, void *arg) {
  static int count = 1;
  plat_printf("this is %s: %d\n", timer->name, count++);
}

void timer2_proc(struct _net_timer_t *timer, void *arg) {
  static int count = 1;
  plat_printf("this is %s: %d\n", timer->name, count++);
}

void timer3_proc(struct _net_timer_t *timer, void *arg) {
  static int count = 1;
  plat_printf("this is %s: %d\n", timer->name, count++);
}

void timer_test(void) {
  static net_timer_t t0, t1, t2, t3;

  net_timer_add(&t0, "t0", timer0_proc, (void *)0, 200, 0);
  net_timer_add(&t1, "t1", timer1_proc, (void *)0, 1000, NET_TIMER_RELOAD);
  net_timer_add(&t2, "t2", timer2_proc, (void *)0, 1000, NET_TIMER_RELOAD);
  net_timer_add(&t3, "t3", timer3_proc, (void *)0, 4000, NET_TIMER_RELOAD);
}

void basic_test(void) {
  nlist_test();
  mblock_test();
  pktbuf_test();

  uint32_t v1 = x_ntohl(0x12345678);
  uint32_t v2 = x_ntohs(0x1234);

  timer_test();
}

#define DBG_TEST DBG_LEVEL_INFO

int main(int argc, char **argv) {
  dbg_info(DBG_TEST, "info");
  dbg_warning(DBG_TEST, "warning");
  dbg_error(DBG_TEST, "error");

  dbg_assert(1 == 1, "failed");

  net_init();

  // basic_test();

  net_start();

  netdev_init();

  while (1) {
    sys_sleep(10);
  }

  return 0;
}