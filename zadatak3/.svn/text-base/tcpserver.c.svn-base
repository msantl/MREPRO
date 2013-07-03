#include "pomocni.h"

char filename[MAXLENGTH];
char file[MAXLENGTH];

int checkFilename(char *ptr, int len) {
  int i;
  for (i = 0; i < len; ++i) {
    if (ptr[i] == '/') {
    return 0;
    }
  }
  return 1;
}

int main(int argc, char** argv) {
  int datoteka;

  int ch;
  int status;
  int sockfd, newfd;

  socklen_t server_l;
  struct sockaddr server;

  socklen_t client_l;
  struct sockaddr client;

  struct addrinfo hints, *res;

  char* port = (char *)malloc(sizeof(char) * 8);

  strcpy(port, S_PORT);

  while ((ch = getopt(argc, argv, "p:")) != -1) {
    switch(ch) {
      case 'p':
        strcpy(port, optarg);
        break;
      default:
        errx(1, "Usage: ./tcpserver [-p port]");
        break;
    }
  }

  if (argc - optind != 0) {
    errx(1, "Usage: ./tcpserver [-p port]");
  }

  // host and port should be defined from here on
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;      // it's ignored if the node is not NULL

  if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
    errx(1, gai_strerror(status));
  } else if (res == NULL) {
    errx(1, "getaddrinfo error");
  }

  sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  server = *(res->ai_addr);
  server_l = sizeof(server);

  Bind(sockfd, res->ai_addr, res->ai_addrlen);

  Listen(sockfd, BACKLOG);

  while (1) {
    memset(file, 0, sizeof file);
    memset(filename, 0, sizeof filename);

    client_l = sizeof(client);
    newfd = Accept(sockfd, &client, &client_l);

#ifdef DEBUG
    printf("Primam\n");
#endif
    Recv(newfd, filename, sizeof(filename), 0);
#ifdef DEBUG
    printf("Primio: %s\n", filename);
#endif

    if (strcmp(filename, "Kraj") == 0) {
      Close(newfd);
      // exit
      break;
    } else if (checkFilename(filename, strlen(filename)) == 0) {
      Close(newfd);
      // print error
      errx(1, "Nedozvoljena putanja datoteke");
    } else {

#ifdef DEBUG
      printf("Saljem: %s\n", filename);
#endif

      datoteka = open(filename, O_RDONLY);
      if (datoteka < 0) {
        errx(1, "open: datoteka ne postoji");
      }

      while ((status = read(datoteka, file, sizeof(file))) > 0) {
#ifdef DEBUG
        printf("%d\n", status);
#endif
        WriteAll(newfd, file, status);
        memset(file, 0, sizeof(file));
      }

      close(datoteka);

#ifdef DEBUG
      printf("Posalo\n");
#endif

      Close(newfd);
    }
  }

  Close(sockfd);
  freeaddrinfo(res);
  free(port);

  return 0;
}
