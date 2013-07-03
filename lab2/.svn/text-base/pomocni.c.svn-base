#include "pomocni.h"

int Socket(int domain, int type, int protocol) {
  int fd = socket(domain, type, protocol);

  if (fd < 0) {

    if (daemon_proc) {
      syslog(LOG_ALERT, "Greska: socket error");
    } else {
      err(1, "socket error");
    }
  }

  return fd;
}

void Close(int sockfd) {
  close(sockfd);
}

void Bind(int sockfd, struct sockaddr* addr, socklen_t addr_len) {
  if (bind(sockfd, addr, addr_len) < 0) {
    Close(sockfd);

    if (daemon_proc) {
      syslog(LOG_ALERT, "Greska: bind error");
    } else {
      err(1, "bind error");
    }

  }
}

void Connect(int sockfd, struct sockaddr* addr, socklen_t addr_len) {
  if (connect(sockfd, addr, addr_len) < 0) {
    Close(sockfd);

    if (daemon_proc) {
      syslog(LOG_ALERT, "Greska: connect error");
    } else {
      err(1, "connect error");
    }

  }
}

int Accept(int sockfd, struct sockaddr* addr, socklen_t *addr_len) {
  int fd = accept(sockfd, addr, addr_len);

  if (fd < 0) {
    Close(sockfd);

    if (daemon_proc) {
      syslog(LOG_ALERT, "Greska: accept error");
    } else {
      err(1, "accept error");
    }

  }

  return fd;
}

void Listen(int sockfd, int backlog) {
  if (listen(sockfd, backlog) < 0) {
    Close(sockfd);

    if (daemon_proc) {
      syslog(LOG_ALERT, "Greska: listen error");
    } else {
      err(1, "listen error");
    }

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

        if (daemon_proc) {
          syslog(LOG_ALERT, "Greska: write error");
        } else {
          errx(1, "write error");
        }

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

        if (daemon_proc) {
          syslog(LOG_ALERT, "Greska: read error");
        } else {
          errx(1, "read error");
        }

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

    if (daemon_proc) {
      syslog(LOG_ALERT, "Greska: send error");
    } else {
      errx(1, "send error");
    }

  }
  return status;
}

int Recv(int sockfd, char *buff, int len, int flags) {
  int status = recv(sockfd, buff, len, flags);
  if (status < 0) {

    if (daemon_proc) {
      syslog(LOG_ALERT, "Greska: recv  error");
    } else {
      errx(1, "recv error");
    }
  }
  return status;
}

int file_last_change(char *filename) {
  struct stat fileState;
  int ftime;
  stat(filename, &fileState);

  ftime = (int)fileState.st_mtime;
  return ftime;
}

int file_size(char *filename) {
  struct stat fileState;
  int fsize;
  stat(filename, &fileState);

  fsize = (int)fileState.st_size;
  return fsize;
}

int file_exists(char *filename) {
  struct stat fileState;
  return (stat(filename, &fileState) == 0);
}
