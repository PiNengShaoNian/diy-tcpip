#include "ping.h"

#include <WinSock2.h>
#include <time.h>

#include "net_api.h"
#include "sys_plat.h"

static uint16_t x_checksum_16(void *buf, uint16_t len) {
  uint16_t *curr_buf = (uint16_t *)buf;
  uint32_t checksum = 0;

  while (len > 1) {
    checksum += *curr_buf++;
    len -= 2;
  }

  if (len > 0) {
    checksum += *(uint8_t *)curr_buf;
  }

  uint16_t high;
  while ((high = (checksum >> 16)) != 0) {
    checksum = high + (checksum & 0xFFFF);
  }

  return (uint16_t)~checksum;
}

void ping_run(ping_t *ping, const char *dest, int count, int size,
              int interval) {
  static uint16_t start_id = PING_DEFAULT_ID;
  // WSADATA wsdata;
  // WSAStartup(MAKEWORD(2, 2), &wsdata);

  struct hostent hent, *result;
  char buf[512];
  int err;
  if (gethostbyname_r(dest, &hent, buf, sizeof(buf), &result, &err) < 0) {
    printf("resolve host %s failed\n", dest);
    return;
  }

  int s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (s < 0) {
    plat_printf("ping: open socket error\n");
    return;
  }

// int tmo = 3000;
#if 1
  struct timeval tmo;
  tmo.tv_sec = 0;
  tmo.tv_usec = 0;
#endif
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tmo, sizeof(tmo));
  struct sockaddr_in addr;
  plat_memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = *(uint32_t *)hent.h_addr_list[0];
  addr.sin_port = 0;

#if 1
  err = connect(s, (const struct sockaddr *)&addr, sizeof(addr));
  if (err != 0) {
    printf("connect failed.\n");
    closesocket(s);
    return;
  }
#endif

  int fill_size = size > PING_BUFFER_SIZE ? PING_BUFFER_SIZE : size;
  for (int i = 0; i < fill_size; i++) {
    ping->req.buf[i] = i;
  }

  int total_size = sizeof(icmp_hdr_t) + fill_size;
  for (int i = 0, seq = 0; i < count; i++, seq++) {
    ping->req.echo_hdr.type = 8;
    ping->req.echo_hdr.code = 0;
    ping->req.echo_hdr.checksum = 0;
    ping->req.echo_hdr.id = start_id;
    ping->req.echo_hdr.seq = seq;
    ping->req.echo_hdr.checksum = x_checksum_16(&ping->req, total_size);

#if 0
    int size = sendto(s, (const char *)&ping->req, total_size, 0,
                      (const struct sockaddr *)&addr, sizeof(addr));
#endif
    int size = send(s, (const char *)&ping->req, total_size, 0);

    if (size < 0) {
      plat_printf("send ping request failed.\n");
      break;
    }

    clock_t time = clock();

    memset(&ping->reply, 0, sizeof(ping->reply));

    do {
      struct sockaddr_in from_addr;
      socklen_t addr_len = sizeof(from_addr);
#if 0
      size = recvfrom(s, (char *)&ping->reply, sizeof(ping->reply), 0,
                      (struct sockaddr *)&from_addr, &addr_len);
#endif
      size = recv(s, (char *)&ping->reply, sizeof(ping->reply), 0);

      if (size < 0) {
        plat_printf("ping recv tmo\n");
        break;
      }

      if ((ping->req.echo_hdr.id == ping->reply.echo_hdr.id) &&
          (ping->reply.echo_hdr.seq == ping->req.echo_hdr.seq)) {
        break;
      }
    } while (1);

    if (size > 0) {
      int recv_size = size - sizeof(ip_hdr_t) - sizeof(icmp_hdr_t);
      if (memcmp(ping->req.buf, ping->reply.buf, recv_size) != 0) {
        printf("recv data error\n");
        continue;
      }

      ip_hdr_t *iphdr = &ping->reply.iphdr;
      int send_size = fill_size;
      if (recv_size == send_size) {
        printf("reply from %s: bytes=%d, ", inet_ntoa(addr.sin_addr),
               recv_size);
      } else {
        printf("reply from %s: bytes=%d (send=%d), ", inet_ntoa(addr.sin_addr),
               recv_size, send_size);
      }

      int diff_ms = (clock() - time) / (CLOCKS_PER_SEC / 1000);

      if (diff_ms < 1) {
        printf(" time < 1ms, TTL=%d\n", iphdr->ttl);
      } else {
        printf(" time   %dms, TTL=%d\n", diff_ms, iphdr->ttl);
      }
    }

    sys_sleep(interval);
  }

  // closesocket(s);
  close(s);
}