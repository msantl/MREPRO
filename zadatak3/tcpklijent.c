#include "pomocni.h"

char filename[MAXLENGTH];
char file[MAXLENGTH];

int main(int argc, char** argv) {
  int ch;
  int status;
  int sockfd;

  socklen_t server_l;
  struct sockaddr server;
  struct addrinfo hints, *res;

  char* port = (char *)malloc(sizeof(char) * 8);
  char* host = (char *)malloc(sizeof(char) * 64);

  strcpy(port, S_PORT);

  while ((ch = getopt(argc, argv, "p:")) != -1) {
    switch(ch) {
      case 'p':
        strcpy(port, optarg);
        break;
      default:
        errx(1, "Usage: ./tcpklijent [-p port] server_IP");
        break;
    }
  }

  if (argc - optind != 1) {
    errx(1, "Usage: ./tcpklijent [-p port] server_IP");
  } else {
    strcpy(host, argv[optind]);
  }

  // host and port should be defined from here on
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;      // it's ignored if the node is not NULL

  if ((status = getaddrinfo(host, port, &hints, &res)) != 0) {
    errx(1, gai_strerror(status));
  } else if (res == NULL) {
    errx(1, "getaddrinfo error");
  }

  sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  server = *(res->ai_addr);
  server_l = sizeof(server);

  Connect(sockfd, &server, server_l);

  // posalji filename
  scanf("%s", filename);
#ifdef DEBUG
  printf("Saljem: %s\n", filename);
#endif
  WriteAll(sockfd, filename, strlen(filename));
#ifdef DEBUG
  printf("Poslao\n");
#endif

#ifdef DEBUG
  printf("Primam\n");
#endif

  while (1) {
    memset(file, 0, sizeof file);
    if ((status = Recv(sockfd, file, MAXLENGTH - 1, 0)) == 0) break;

#ifdef DEBUG
  printf("%d\n", status);
#endif

    write(1, file, status);
  }

#ifdef DEBUG
  printf("Primio\n");
#endif

  Close(sockfd);
  freeaddrinfo(res);
  free(port);
  free(host);

  return 0;
}
