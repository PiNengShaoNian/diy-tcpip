#include <WinSock2.h>

#include "net_api.h"
#include "sys_plat.h"
#include "tcp_echo_server.h"

void udp_echo_server(void *arg) {
  int port = (unsigned long long)arg & 0xFFFF;
  plat_printf("udp server start, port: %d\n", port);

  WSADATA wsdata;
  WSAStartup(MAKEWORD(2, 2), &wsdata);

  int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s < 0) {
    plat_printf("udp echo server: open socket error\n");
    return;
  }

  struct sockaddr_in server_addr;
  plat_memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  //   if (bind(s, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
  //   0) {
  //     plat_printf("bind error\n");
  //     goto end;
  //   }

  while (1) {
    struct sockaddr_in client_addr;
    char buf[125];
    int addr_len = sizeof(client_addr);
    ssize_t size = recvfrom(s, buf, sizeof(buf), 0,
                            (struct sockaddr *)&client_addr, &addr_len);

    if (size < 0) {
      printf("recv error\n");
      goto end;
    }

    plat_printf("udp echo server: connect ip: %s, port: %d\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    sendto(s, buf, size, 0, (struct sockaddr *)&client_addr, addr_len);
    if (size < 0) {
      printf("sendto error\n");
      goto end;
    }
  }

end:
  close(s);
  //   closesocket(s);
}

void udp_echo_server_start(int port) {
  unsigned long long l_port = port;
  if (sys_thread_create(udp_echo_server, (void *)l_port) ==
      SYS_THREAD_INVALID) {
    printf("create udp server thread failed.\n");
    return;
  }
}