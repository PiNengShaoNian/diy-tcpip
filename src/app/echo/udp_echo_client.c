#include "udp_echo_client.h"

#include <WinSock2.h>

#include "net_api.h"
#include "sys_plat.h"

int udp_echo_client_start(const char *ip, int port) {
  plat_printf("udp echo client, ip: %s, port: %d\n", ip, port);

  int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s < 0) {
    plat_printf("udp echo client: open socket error\n");
    goto end;
  }

  struct sockaddr_in server_addr;
  plat_memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);

  connect(s, (const struct sockaddr *)&server_addr, sizeof(server_addr));

  char buf[128];
  plat_printf(">>");
  while (fgets(buf, sizeof(buf), stdin) != NULL) {
    if (strncmp(buf, "quit", 4) == 0) {
      break;
    }

    if (sendto(s, buf, (int)plat_strlen(buf), 0,
               (const struct sockaddr *)&server_addr,
               sizeof(server_addr)) <= 0) {
      plat_printf("write error\n");
      goto end;
    }

    plat_memset(buf, 0, sizeof(buf));

    struct sockaddr_in remote_addr;
    int addr_len = sizeof(remote_addr);
    int len = recvfrom(s, buf, sizeof(buf) - 1, 0,
                       (struct sockaddr *)&remote_addr, &addr_len);
    if (len <= 0) {
      plat_printf("read error\n");
      goto end;
    }

    buf[sizeof(buf) - 1] = '\0';
    plat_printf("%s\n", buf);
    plat_printf(">>");
  }

  //   closesocket(s);
  close(s);

  return 0;

end:
  if (s >= 0) {
    // closesocket(s);
    close(s);
  }
  return -1;
}