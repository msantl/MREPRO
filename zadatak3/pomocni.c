#include "pomocni.h"

int Socket(int domain, int type, int protocol) {
  int fd = socket(domain, type, protocol);

  if (fd < 0) {
    err(1, "socket error");
  }

  return fd;
}

void Close(int sockfd) {
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
}

void Bind(int sockfd, struct sockaddr* addr, socklen_t addr_len) {
  if (bind(sockfd, addr, addr_len) < 0) {
    Close(sockfd);
    err(1, "bind error");
  }
}

void Connect(int sockfd, struct sockaddr* addr, socklen_t addr_len) {
  if (connect(sockfd, addr, addr_len) < 0) {
    Close(sockfd);
    err(1, "connect error");
  }
}

int Accept(int sockfd, struct sockaddr* addr, socklen_t *addr_len) {
  int fd = accept(sockfd, addr, addr_len);

  if (fd < 0) {
    Close(sockfd);
    err(1, "accept error");
  }

  return fd;
}

void Listen(int sockfd, int backlog) {
  if (listen(sockfd, backlog) < 0) {
    Close(sockfd);
    err(1, "listen error");
  }
}

int WriteAll(int sockfd, char *buff, int len) {
  int nleft, nwritten;
  nleft = len;

  while (nleft > 0) {
    if ( (nwritten = write(sockfd, buff, nleft)) <= 0) {
      if (nwritten < 0 && errno == EINTR) {
        nwritten = 0; /* and call write again */
      } else {
        errx(1, "write error");
      }
    }
    nleft -= nwritten;
    buff += nwritten;
  }

  return len;
}

int ReadAll(int sockfd, char *buff, int len) {
  int nleft, nread;
  nleft = len;

  while (nleft > 0) {
    if ( (nread = read(sockfd, buff, nleft)) < 0) {
      if (errno == EINTR) {
        nread = 0; /* and call read() again */
      } else {
        errx(1, "read error");
      }
    } else if (nread == 0) {
      break; /* EOF */
    }
    nleft -= nread;
    buff += nread;
  }
  return (len - nleft);
}

int Send(int sockfd, char *buff, int len, int flags) {
  int status = send(sockfd, buff, len, flags);
  if (status < 0) {
    errx(1, "send error");
  }
  return status;
}

int Recv(int sockfd, char *buff, int len, int flags) {
  int status = recv(sockfd, buff, len, flags);
  if (status < 0) {
    errx(1, "recv error");
  }
  return status;
}

