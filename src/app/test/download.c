#include <WinSock2.h>
#include <stdio.h>
#include <string.h>

#include "net_plat.h"

void download(const char *filename, int port) {
  printf("try to download %s from %s: %d\n", filename, friend0_ip, port);
  SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (sockfd < 0) {
    printf("create socket failed.");
    return;
  }
  FILE *file = fopen(filename, "wb");
  if (file == (FILE *)0) {
    printf("open file failed.\n");
    goto failed;
  }

  struct sockaddr_in server_addr;
  plat_memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(friend0_ip);
  server_addr.sin_port = htons(port);
  if (connect(sockfd, (const struct sockaddr *)&server_addr,
              sizeof(server_addr)) < 0) {
    plat_printf("connect error\n");
    goto failed;
  }

  char buf[8192];
  int rcv_size;
  ssize_t total_size = 0;
  while ((rcv_size = recv(sockfd, buf, sizeof(buf), 0)) > 0) {
    fwrite(buf, 1, rcv_size, file);
    printf(".");
    total_size += rcv_size;
  }

  if (rcv_size < 0) {
    printf("rcv file size: %d\n", (int)total_size);
    goto failed;
  }

  printf("rcv file size: %d\n", (int)total_size);
  printf("rcv file ok.\n");
  closesocket(sockfd);
  fclose(file);
  return;

failed:
  if (sockfd) {
    closesocket(sockfd);
  }
  if (file) {
    fclose(file);
  }
}