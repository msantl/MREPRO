#include "pomocni.h"

/*
 * provjera ako je datoteka iz radnog kazala
 */
int check_filename(char *filename) {
  char *p;
  for (p = filename; *p; ++p) {
    if (*p == '/') {
      return 0;
    }
  }
  return 1;
}

int daemon_init(const char *pname, int facility) {
  int i;
  pid_t pid;

  if ((pid = fork()) < 0) {
    return -1;
  } else if (pid > 0) {
    /*
     * proces roditelj - mora zavrsiti s radom
     */
    _exit(0);
  }

  if (setsid() < 0) {
    return -1;
  }

  /*
   * ignoriramo SIGHUP signal kojeg djete dobije
   */
  signal(SIGHUP, SIG_IGN);

  if ((pid = fork()) < 0) {
    return -1;
  } else if (pid > 0) {
    /*
     * proces roditelj - mora zavrsiti s radom
     */
    _exit(0);
  }

  daemon_proc = 1;

  /*
   * zatvaramo opisnike datoteka
   */
  for (i = 0; i < MAXFD; ++i) {
    Close(i);
  }

  open("/dev/null", O_RDONLY);
  open("/dev/null", O_RDWR);
  open("/dev/null", O_RDWR);

  /*
   * pripremi syslog za izlaz demon servera
   */
  openlog(pname, LOG_PID, facility);

  return 0;
}

int main(int argc, char **argv) {
  char buffer[MAXLENGTH];
  char file[MAXLENGTH];
  char filename[MAXLENGTH];
  char adresa[32];

  int ch;
  int status;
  int sockfd, newfd;
  FILE* datoteka;

  uint32_t offset, last_change;

  pid_t pid;

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

  /*
   * napravi ga demonom
   */
  if (daemon_init("MrePro tcpserver", LOG_FTP) < 0) {
    errx(1, "daemon failed to initialize!");
  }

  /*
   * NOTE: dalje se sva upozorenja/pogreske pisu sa
   * syslog(LOG_ALERT, poruka, )
   */

  // host and port should be defined from here on
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;      // it's ignored if the node is not NULL

  if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
    syslog(LOG_ALERT, "Greska: %s", gai_strerror(status));
  } else if (res == NULL) {
    syslog(LOG_ALERT, "Greska: getaddrinfo error");
  }

  sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  server = *(res->ai_addr);
  server_l = sizeof(server);

  Bind(sockfd, res->ai_addr, res->ai_addrlen);

  Listen(sockfd, BACKLOG);

  while (1) {
    client_l = sizeof(client);
    newfd = Accept(sockfd, &client, &client_l);

    if ((pid = fork()) < 0) {
      syslog(LOG_ALERT, "Greska: fork() error");
    } else if (pid == 0) {
      /*
       * dijete
       * zatvara sve ostale sockete
       */
      Close(sockfd);
      memset(buffer, 0, sizeof buffer);

      /*
       * primi pomak, vrijeme posljednje izmjene i naziv trazene datoteke
       */
      status = Recv(newfd, buffer, MAXLENGTH - 1, 0);

      /*
       * read out the offset
      */
      memcpy(&offset, buffer, 4);

      /*
       * read out the last_change
      */
      memcpy(&last_change, buffer + 4, 4);

      /*
       * convert the network to host byte order
       */
      offset = ntohl(offset);
      last_change = ntohl(last_change);

      memset(filename, 0, sizeof(filename));
      memset(file, 0, sizeof(file));

      /*
       * read out the filename
       */
      strcpy(filename, &buffer[8]);

      /*
       * provjeri ispravnost naziva i datum
       */
      if (check_filename(filename) == 0) {

        syslog(LOG_ALERT, "Greska: zadan je nevaljan put %s", filename);

        Close(newfd);
        _exit(0);

      } else if (last_change >= file_last_change(filename)) {

        file[0] = DATOTEKA_NEPROMIJENJENA;
        WriteAll(newfd, file, 1);

        syslog(LOG_ALERT, "Greska: datoteka %s se nije promijenila", filename);

        Close(newfd);
        _exit(0);
      }

      /*
       * posalji trazenu datoteku
       * ako je last_change stariji od postojece
       */
      datoteka = fopen(filename, "rb");
      if (datoteka == NULL) {

        file[0] = DATOTEKA_NE_POSTOJI;
        WriteAll(newfd, file, 1);

        syslog(LOG_ALERT, "Greska: datoteka %s ne postoji", filename);

        Close(newfd);
        _exit(0);
      }

      /*
       * napravi pomak
       */
      fseek(datoteka, offset, SEEK_SET);

      file[0] = DATOTEKA_OK;
      status = fread(file + 1, 1, sizeof(file) - 1, datoteka);
      WriteAll(newfd, file, status + 1);

      while (status > 0) {
        memset(file, 0, sizeof(file));
        status = fread(file, 1, sizeof(file), datoteka);

        WriteAll(newfd, file, status);
      }

      fclose(datoteka);

      /*
       * napravi zapis u syslog
       */
      inet_ntop(
        client.sa_family,
        &((struct sockaddr_in *)&client)->sin_addr,
        adresa,
        sizeof(adresa)
      );

      syslog(LOG_INFO, "Klijentu %s poslan %s:%d, %d", adresa, filename, offset, file_last_change(filename));

      /*
       * zatvori socket
       */
      Close(newfd);
      _exit(0);
    } else {
      /*
       * inace smo u roditeljskom procesu
       * zatvaramo djetetov fd
       */

      Close(newfd);
    }

  }

  Close(sockfd);
  freeaddrinfo(res);
  free(port);

  return 0;
}
