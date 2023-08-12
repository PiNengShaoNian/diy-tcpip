#include "tcp_echo_client.h"

#include <WinSock2.h>

#include "net_api.h"
#include "sys_plat.h"

int tcp_echo_client_start(const char *ip, int port) {
  plat_printf("tcp echo client, ip: %s, port: %d\n", ip, port);

  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    plat_printf("tcp echo client: open socket error\n");
    goto end;
  }

  struct sockaddr_in server_addr;
  plat_memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);
  if (connect(s, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    plat_printf("connect error\n");
    goto end;
  }

#if 1
  char sbuf[128];
  fgets(sbuf, sizeof(sbuf), stdin);
  close(s);
  return 0;
#endif

#if 0
  char sbuf[4096];
  for (int i = 0; i < sizeof(sbuf); i++) {
    sbuf[i] = 'a' + i % 26;
  }

  for (int i = 0; i < 2; i++) {
    ssize_t size = send(s, sbuf, sizeof(sbuf), 0);
    if (size < 0) {
      printf("send error\n");
      break;
    }
  }

  // fgets(sbuf, sizeof(sbuf), stdin);
  close(s);
  return 0;
#endif

  char buf[128];
  plat_printf(">>");
  while (fgets(buf, sizeof(buf), stdin) != NULL) {
    if (send(s, buf, (int)plat_strlen(buf), 0) <= 0) {
      plat_printf("write error\n");
      goto end;
    }

    plat_memset(buf, 0, sizeof(buf));
    int len = recv(s, buf, sizeof(buf) - 1, 0);
    if (len <= 0) {
      plat_printf("read error\n");
      goto end;
    }

    plat_printf("%s\n", buf);
    plat_printf(">>");
  }

  // closesocket(s);
  close(s);

  return 0;

end:
  if (s >= 0) {
    // closesocket(s);
    close(s);
  }
  return -1;
}