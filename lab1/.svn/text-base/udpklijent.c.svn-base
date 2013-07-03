#include "pomocni.h"

int main(int argc, char **argv) {
  int status;
  int ch;
  int socket_fd;
  int msglen;
  char question[MAXLEN], answer[MAXLEN];
  char *port = (char *)malloc(sizeof(char) * 8);
  char *host = (char *)malloc(sizeof(char) * 64);

  socklen_t server_l;
  struct sockaddr server;
  struct addrinfo hints, *res;

  memset(question, 0, sizeof question);
  memset(answer, 0, sizeof answer);

  strcpy(port, S_PORT);

  while ((ch = getopt(argc, argv, "p:")) != -1) {
    switch(ch) {
      case 'p':
        strcpy(port, optarg);
        break;
      default:
        errx(1, "Usage: ./udpklijent [-p port] server_IP");
        break;
    }
  }

  if (argc - optind != 1) {
    errx(1, "Usage: ./udpklijent [-p port] server_IP");
  } else {
    strcpy(host, argv[optind]);
  }

#ifdef DEBUG
  printf("Trying to connect to %s:%s\n", host, port);
#endif

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo(host, port, &hints, &res);

  if (status != 0) { errx(1, gai_strerror(status)); }

  // pokreni klijenta
  socket_fd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  server = *(res->ai_addr);

#ifdef DEBUG
  printf("Client is up and running\n");
#endif

  while (scanf("%s", question) != EOF) {
    server_l = sizeof(server);
    sendto(socket_fd, question, strlen(question), 0, &server, server_l);
    msglen = recvfrom(socket_fd, answer, sizeof(answer), 0, &server, &server_l);

    printf("A: %s\n", answer);
    memset(question, 0, sizeof question);
    memset(answer, 0, sizeof answer);
  }

  // slanje praznog datagrama
#ifdef DEBUG
  printf("Slanje praznog datagrama\n");
#endif
  server_l = sizeof(server);
  sendto(socket_fd, question, 0, 0, &server, server_l);

  // oslobodi resurse
  Close(socket_fd);
  freeaddrinfo(res);
  free(host);
  free(port);

  return 0;
}
