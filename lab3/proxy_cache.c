#include "pomocni.h"


int noviSocket(char *node, char *port, struct sockaddr *server, socklen_t *server_l, int povezi) {
  struct addrinfo hints, *res;

  int status;
  int sockfd;

  // host and port should be defined from here on
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;      // it's ignored if the node is not NULL

  if ((status = getaddrinfo(node, port, &hints, &res)) != 0) {
    errx(1, "%s", gai_strerror(status));
  } else if (res == NULL) {
    errx(1, "getaddrinfo returned no results");
  }

  sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (server != NULL) {
    *server = *(res->ai_addr);
  }

  if (server_l != NULL) {
    *server_l = sizeof(*server);
  }

  if (povezi) {
    Bind(sockfd, res->ai_addr, res->ai_addrlen);
  }

  freeaddrinfo(res);

  return sockfd;
}

void getHostname(char *host, int n, char *hostname) {
  int i = 0; // pojava //
  int j;

  for (i = 3; i < n; ++i) {
    if (host[i-3] == ':' && host[i-2] == '/' && host[i-1] == '/') {
      break;
    }
  }

  for (j = i; j < n && host[j] != '/'; ++j) {
    hostname[j - i] = host[j];
  }
}

void getFilename(char *host, int n, char *filename) {
  int i;

  for (i = 0; i < n; ++i) {
    if (host[i] == '/') filename[i] = '_';
    else if (host[i] == ':') filename[i] = '_';
    else if (host[i] == '%') filename[i] = '_';
    else if (host[i] == '.') filename[i] = '_';
    else filename[i] = host[i];
  }
}

int main(int argc, char **argv) {
  char buffer[MAXLENGTH];

  char request[MAXLENGTH];
  char host[MAXLENGTH], hostname[MAXLENGTH];
  char protokol[MAXLENGTH];

  char adresa[MAXLENGTH];

  char blacklist[32][MAXLENGTH];
  int blacklist_size = 0;

  char *char_p;

  int i;
  int ch;
  int sockfd, newfd;
  int msglen, status;
  static int cache;

  FILE *cache_file;
  char filename[MAXLENGTH];

  pid_t pid;

  socklen_t server_l;
  struct sockaddr server;

  socklen_t client_l;
  struct sockaddr client;

  char* port = (char *)malloc(sizeof(char) * 8);
  strcpy(port, S_PORT);

  int option_index = 0;
  static struct option long_options[] = {
    {"cache", no_argument, &cache,  1},
    {0,       0,           0,       0},
  };

  while ((ch = getopt_long(argc, argv, "", long_options, &option_index)) != -1);

  if (argc - optind < 1) {
    errx(1, "Usage: ./proxy_cache [--cache] port <black_list>");
  } else {
    strcpy(port, argv[optind]);
    for (i = optind + 1; i < argc; ++i) {
      strcpy(blacklist[blacklist_size], argv[i]);
      blacklist_size += 1;
    }
  }

  sockfd = noviSocket(NULL, port, NULL, NULL, 1);

  Listen(sockfd, BACKLOG);

  while (1) {
    // cekamo novu konekciju
    client_l = sizeof(client);
    newfd = Accept(sockfd, &client, &client_l);

    if ((pid = fork()) < 0) {
      errx(1, "fork error");
    } else if (pid == 0) {
      Close(sockfd);
      /*
       * IP adresa klijenta
       */
      inet_ntop(
        client.sa_family,
        &((struct sockaddr_in *)&client)->sin_addr,
        adresa,
        sizeof adresa
      );

      /*
       * Primi zahtjev od klijenta
       */
      memset(buffer, 0, sizeof buffer);
      msglen = Recv(newfd, buffer, sizeof(buffer), 0);

      /*
       * ukloni
       * Proxy-Connection ili Connection
       * te dodati u zaglavlje redak
       * Connection: close
       */
      /*
      printf("==================================================\n");
      puts(buffer);
      printf("==================================================\n");
      */
      if ((char_p = strstr(buffer, "Proxy-Connection:")) != NULL) {
        for (i = char_p - buffer; i < strlen(buffer); ++i) {
          if (buffer[i] == '\r' && buffer[i+1] == '\n') {
            break;
          }
        }
        strcpy(char_p, char_p + i + 2);
      }

      if ((char_p = strstr(buffer, "Connection:")) != NULL) {
        for (i = char_p - buffer; i < strlen(buffer); ++i) {
          if (buffer[i] == '\r' && buffer[i+1] == '\n') {
            break;
          }
        }
        strcpy(char_p, char_p + i + 2);
      }

      strcat(buffer + strlen(buffer) - 2, "Connection: close\r\n");
      strcat(buffer + strlen(buffer), "\r\n");

      /*
      printf("==================================================\n");
      puts(buffer);
      printf("==================================================\n");
      */

      /*
       * izdvoji podatke
       */
      memset(request, 0, sizeof request);
      status = 0;
      for (i = 0; buffer[i] != ' '; ++i) {
        request[status++] = buffer[i];
      }

      memset(host, 0, sizeof host);
      status = 0;
      for (++i; buffer[i] != ' '; ++i) {
        host[status++] = buffer[i];
      }

      memset(protokol, 0, sizeof protokol);
      status = 0;
      for (++i; buffer[i] != '\r' && buffer[i+1] != '\n'; ++i) {
        protokol[status++] = buffer[i];
      }

      /*
       * Zahtjev
       */
      if (strlen(request) != 3 || strcmp(request, "GET") != 0) {
        memset(buffer, 0, sizeof buffer);

        strcpy(buffer, protokol);
        strcat(buffer, " 405 Method Not Allowed\r\n");
        strcat(buffer, "Content-Type: text/html\r\n");
        strcat(buffer, "\r\n");
        strcat(buffer, "<html><body><h1>405 Method Not Allowed</h1></body></html>");

        WriteAll(newfd, buffer, strlen(buffer));

        fprintf(stdout, "%s: KRIVO HEAD %s\n", adresa, host);
        Close(newfd);
        _exit(0);
      }

      /*
       * Filtriraj
       */
      for (i = 0; i < blacklist_size; ++i) {
        if (strstr(host, blacklist[i])) {
          memset(buffer, 0, sizeof buffer);

          strcpy(buffer, protokol);
          strcat(buffer, " 403 Forbidden\r\n");
          strcat(buffer, "Content-Type: text/html\r\n");
          strcat(buffer, "\r\n");
          strcat(buffer, "<html><body><h1>403 Forbidden</h1></body></html>");

          WriteAll(newfd, buffer, strlen(buffer));

          fprintf(stdout, "%s: FILTRIRANO %s %s\n", adresa, request, host);
          Close(newfd);
          _exit(0);
        }
      }


      memset(hostname, 0, sizeof hostname);
      getHostname(host, strlen(host), hostname);

      memset(filename, 0, sizeof filename);
      getFilename(host, strlen(host), filename);

      /*
      printf("REQ: %s\n", request);
      printf("HST: %s\n", host);
      printf("HNM: %s\n", hostname);
      printf("FNM: %s\n", filename);
      printf("PRT: %s\n", protokol);
      */

      /*
       * Provjeri cache
       */
      if (cache) {
        /*
         * Provjeri ako postoji cacheirani odgovor
         */

        cache_file = fopen(filename, "rb");

        if (cache_file != NULL) {

          while (1) {
            memset(buffer, 0, sizeof buffer);
            status = fread(buffer, 1, sizeof(buffer), cache_file);

            WriteAll(newfd, buffer, status);

            if (status != sizeof(buffer)) break;
          }

          fclose(cache_file);

          fprintf(stdout, "%s: %s %s (CACHE)\n", adresa, request, host);
          Close(newfd);
          _exit(0);
        }
      }

      /*
       * Generiraj odgovor
       */
      sockfd = noviSocket(hostname, "http", &server, &server_l, 0);
      Connect(sockfd, &server, server_l);

      /*
       * Prosljedi zahtjev
       */
      WriteAll(sockfd, buffer, strlen(buffer));

      /*
       * Primi odgovor
       */
      if (cache) {
        cache_file = fopen(filename, "wb");
      }

      while (1) {
        memset(buffer, 0, sizeof buffer);
        /*
         * citaj
         */
        msglen = ReadAll(sockfd, buffer, sizeof(buffer));
        // msglen = Recv(sockfd, buffer, sizeof(buffer), 0);

        /*
         * pisi
         */
        status = WriteAll(newfd, buffer, msglen);

        /*
         * spremi u cache
         */
        if (cache) {
          fwrite(buffer, 1, msglen, cache_file);
        }

        // procitali smo manje od velicine buffera
        if (msglen != sizeof(buffer)) break;
      }

      if (cache) {
        fclose(cache_file);
      }

      Close(sockfd);

      fprintf(stdout, "%s: %s %s\n", adresa, request, host);
      Close(newfd);
      _exit(0);
    } else {
      Close(newfd);
    }

  }

  Close(sockfd);
  free(port);

  return 0;
}
