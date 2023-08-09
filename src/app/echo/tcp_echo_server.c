#include "tcp_echo_server.h"

#include <WinSock2.h>

#include "net_api.h"
#include "sys_plat.h"

int tcp_echo_server_start(int port) {
  plat_printf("tcp server start, port: %d\n", port);

  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    plat_printf("tcp echo client: open socket error\n");
    goto end;
  }

  struct sockaddr_in server_addr;
  plat_memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(s, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    plat_printf("bind error\n");
    goto end;
  }

  listen(s, 2);

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client = accept(s, (struct sockaddr *)&client_addr, &addr_len);
    if (client < 0) {
      printf("accept error\n");
      break;
    }

    plat_printf("tcp echo server: connect ip: %s, port: %d\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

#if 1
    char buffer[192];
    fgets(buffer, sizeof(buffer), stdin);
#else
    char buf[125];
    size_t size;
    while ((size = recv(client, buf, sizeof(buf), 0)) > 0) {
      plat_printf("recv size: %d\n", (int)size);
      send(client, buf, (int)size, 0);
    }
#endif

    close(client);
  }

  close(s);

  return 0;

end:
  if (s >= 0) {
    close(s);
  }
  return -1;
}