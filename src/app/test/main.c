#include <stdio.h>

#include "sys_plat.h"

static sys_sem_t sem;
static sys_mutex_t mutex;
static int count;

static char buffer[100];
static int write_index, read_index;
static sys_sem_t read_sem;

void thread1_entry(void *arg) {
  for (int i = 0; i < 2 * sizeof(buffer); i++) {
    sys_sem_wait(read_sem, 0);

    uint8_t data = buffer[read_index++];

    if (read_index >= sizeof(buffer)) {
      read_index = 0;
    }

    plat_printf("thread 1: read data=%d\n", data);
    sys_sleep(200);
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
    buffer[write_index++] = i;

    if (write_index >= sizeof(buffer)) {
      write_index = 0;
    }

    plat_printf("thread 2: write data = %d\n", i);
    sys_sem_notify(read_sem);
    sys_sleep(100);
  }

  while (1) {
    sys_sem_wait(sem, 0);
    plat_printf("this is thread2: %s\n", (char *)arg);
  }
}

int main(int argc, char **argv) {
  sem = sys_sem_create(0);
  mutex = sys_mutex_create();
  read_sem = sys_sem_create(0);

  sys_thread_create(thread1_entry, "AAAA");
  sys_thread_create(thread2_entry, "BBBB");

  // 以下是测试代码，可以删掉
  // 打开物理网卡，设置好硬件地址
  static const uint8_t netdev0_hwaddr[] = {0x00, 0x50, 0x56, 0xc0, 0x00, 0x11};
  pcap_t *pcap = pcap_device_open("192.168.74.1", netdev0_hwaddr);
  while (pcap) {
    static uint8_t buffer[1024];
    static int counter = 0;
    struct pcap_pkthdr *pkthdr;
    const uint8_t *pkt_data;
    plat_printf("begin test: %d\n", counter++);
    for (int i = 0; i < sizeof(buffer); i++) {
      buffer[i] = i;
    }

    if (pcap_next_ex(pcap, &pkthdr, &pkt_data) != 1) {
      continue;
    }

    int len = pkthdr->len > sizeof(buffer) ? sizeof(buffer) : pkthdr->len;
    plat_memcpy(buffer, pkt_data, len);
    buffer[0] = 1;
    buffer[1] = 2;

    if (pcap_inject(pcap, buffer, len) == -1) {
      plat_printf("pcap send: send packet failed %s\n", pcap_geterr(pcap));
      break;
    }
  }

  printf("Hello, world");
  return 0;
}