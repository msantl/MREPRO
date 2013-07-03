#include "pomocni.h"

int Socket(int ai_family, int ai_socktype, int ai_protocol) {
  int fd = socket(ai_family, ai_socktype, ai_protocol);
  if (fd < 0) {
    errx(1, "Could not initialize socket");
  }
  return fd;
}

void Bind(int sockfd, const struct sockaddr* myaddr, int addrlen) {
  int status = bind(sockfd, myaddr, addrlen);
  if (status < 0) {
    errx(1, "Could not bind to socket");
  }
  return;
}

void Close(int sockfd) {
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
}
