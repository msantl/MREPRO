#include "pomocni.h"

void generate_server_response(char *q, char *a) {
  int status;
  char adresa[64], host[64], serv[64];
  struct sockaddr_in sa;
  struct addrinfo hints, *res;

  memset(&sa, 0, sizeof sa);
  status = inet_pton(AF_INET, q, &(sa.sin_addr));

  if (status < 0) {
    strcpy(a, "inet_pton returned error for this query");
  } else if (status == 0) {
    // hostname
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_flags |= AI_CANONNAME;

    status = getaddrinfo(q, S_SERVICE, &hints, &res);

    if (status != 0) {
      strcpy(a, gai_strerror(status));
    } else if (res == NULL) {
      strcpy(a, "getaddrinfo didnt find any records");
    } else {

      inet_ntop(
        res->ai_family,
        &((struct sockaddr_in *)res->ai_addr)->sin_addr,
        adresa,
        sizeof(adresa)
      );

      strcpy(a, q);
      strcat(a, " ");
      strcat(a, adresa);
    }

  } else if (status == 1) {
    // IP
    sa.sin_family = AF_INET;
    sa.sin_port = htons(SERVICE);
    status = getnameinfo((struct sockaddr *)&sa, sizeof(sa), host, sizeof host, serv, sizeof serv, 0);

    if (status != 0) {
      strcpy(a, gai_strerror(status));
    } else {

      strcpy(a, q);
      strcat(a, " ");
      strcat(a, host);
    }

  } else {
    errx(1, "Undefined behaviour for inet_pton");
  }

  return;
}

int main(int argc, char **argv) {
  int ch;
  int status;
  int socket_fd;
  int msglen;
  socklen_t client_l;
  struct sockaddr client;
  struct addrinfo hints, *res;

  char question[MAXLEN], answer[MAXLEN];

  char *port = (char *)malloc(sizeof(char) * 8);
  char *IP = (char *)malloc(sizeof(char) * 64);

  // default port
  strcpy(port, S_PORT);

  while ((ch = getopt(argc, argv, "p:")) != -1) {
    switch(ch) {
      case 'p':
        strcpy(port, optarg);
        break;
      default:
        errx(1, "Usage: ./udpserver [-p port]");
        break;
    }
  }

  // start the server
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;        // IPv4 family
  hints.ai_socktype = SOCK_DGRAM;   // UDP socket
  hints.ai_flags = AI_PASSIVE;      // moja IP adresa

  status = getaddrinfo(NULL, port, &hints, &res);

  if (status != 0) { errx(1, gai_strerror(status)); }

  socket_fd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  Bind(socket_fd, res->ai_addr, res->ai_addrlen);

#ifdef DEBUG
  printf("Server is up and running\n");
#endif

  while (1) {
    // clear buffers
    memset(question, 0, sizeof question);
    memset(answer, 0, sizeof answer);

    // wait for the client question
    client_l = sizeof(client);
    msglen = recvfrom(socket_fd, question, sizeof(question), 0, &client, &client_l);

    if (msglen == 0) {
      // prazni datagram
      break;
    } else {
      // remove the endline character
      if (question[msglen - 1] == '\n') {
        question[msglen - 1] = '\0';
      }

      // generiraj odgovor
      printf("Q: %s\n", question);

      generate_server_response(question, answer);

#ifdef DEBUG
      printf("A: %s\n", answer);
#endif
      // posalji natrag
      sendto(socket_fd, answer, strlen(answer), 0, &client, client_l);
    }
  }

  // oslobodi resurse
  Close(socket_fd);
  freeaddrinfo(res);
  free(port);
  free(IP);

  return 0;
}
