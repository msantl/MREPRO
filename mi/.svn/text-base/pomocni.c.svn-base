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

void p_to_i(char *c, int *i) {
  *i = 0;
  while(*c) {
    *i = (*i) * 10 + ((*c) - '0');
    ++c;
  }
  return;
}


uint64_t my_getcurrtime() {
  struct timeval tv;
  uint64_t curtime;
  gettimeofday(&tv, NULL);
  curtime = tv.tv_sec * 1000000 + tv.tv_usec;
  return curtime;
}

