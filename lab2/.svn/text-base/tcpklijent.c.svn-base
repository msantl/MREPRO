#include "pomocni.h"

int main(int argc, char **argv) {
  char buffer[MAXLENGTH];
  char filename[MAXLENGTH];
  char filename_write[MAXLENGTH];
  char file[MAXLENGTH];

  int ch;
  int status;
  int continue_flag = 0;
  int sockfd;
  FILE* datoteka;

  uint32_t offset, last_change;

  socklen_t server_l;
  struct sockaddr server;
  struct addrinfo hints, *res;

  char* port = (char *)malloc(sizeof(char) * 8);
  char* host = (char *)malloc(sizeof(char) * 64);

  strcpy(port, S_PORT);

  while ((ch = getopt(argc, argv, "p:c")) != -1) {
    switch(ch) {
      case 'p':
        strcpy(port, optarg);
        break;
      case 'c':
        continue_flag = 1;
        break;
      default:
        errx(1, "Usage: ./tcpklijent [-p port] [-c] server datoteka");
        break;
    }
  }

  if (argc - optind != 2) {
    errx(1, "Usage: ./tcpklijent [-p port] [-c] server datoteka");
  } else {
    strcpy(host, argv[optind]);
    strcpy(filename, argv[optind + 1]);
    strcpy(filename_write, filename);
  }

  if (continue_flag) {
    // ako treba nastaviti
    if (file_exists(filename) == 0) {
      // a ne postoji ta datoteka
      offset = 0;
    } else {
      // inace procitaj velicinu datoteke
      offset = file_size(filename);
    }
  } else {
    // inace provjeri ako u kazalu postoji ta datoteka
    if (file_exists(filename) == 1) {
      // ako datoteka postoji a nema zastavice c, spremamo u .novo
      strcat(filename_write, ".novo");
    }

    offset = 0;
  }

  if (file_exists(filename) == 1) {
    last_change = file_last_change(filename);
  } else {
    last_change = 0;
  }

  // pomocni.h
  daemon_proc = 0;

  // host and port should be defined from here on
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;      // it's ignored if the node is not NULL

  if ((status = getaddrinfo(host, port, &hints, &res)) != 0) {
    errx(1, "%s", gai_strerror(status));
  } else if (res == NULL) {
    errx(1, "getaddrinfo error");
  }

  sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  server = *(res->ai_addr);
  server_l = sizeof(server);

  Connect(sockfd, &server, server_l);

  /*
   * posalji pomak, vrijeme posljednje izmjene i naziv trazene datoteke
   */
  last_change = htonl( last_change );
  offset = htonl( offset );

  memset(buffer, 0, sizeof(buffer));
  /*
   * prepare buffer for offset
   */
  memcpy(buffer, &offset, 4);

  /*
   * prepare buffer for last_change
  */
  memcpy(buffer + 4, &last_change, 4);

  /*
   * put filename in buffer
   */
  strcpy(&buffer[8], filename);

  Send(sockfd, buffer, sizeof(buffer), 0);

  /*
   * primi prvi paket koji sadrzi status na prvom oktetu
   */
  memset(file, 0, sizeof(file));
  status = Recv(sockfd, file, MAXLENGTH - 1, 0);

  if (file[0] == DATOTEKA_NE_POSTOJI) {
    warnx("Datoteka ne postoji na posluzitelju");
  } else if (file[0] == DATOTEKA_NEPROMIJENJENA) {
    warnx("Datoteka na posluzitelju je starija");
    // da li je to zbilja bitno ako imamo -c opciju :/
  } else {
    /*
     * offset 1 zbog statusnog koda
     */
    datoteka = fopen(filename_write, "ab");

    if (datoteka == NULL) {
      errx(1, "Ne mogu otvoriti %s", filename_write);
    }

    fwrite(file + 1, 1, status - 1, datoteka);

    while (status != 0) {
      memset(file, 0, sizeof(file));
      status = Recv(sockfd, file, MAXLENGTH - 1, 0);

      fwrite(file, 1, status, datoteka);
    }

    fclose(datoteka);
  }

  Close(sockfd);
  freeaddrinfo(res);
  free(port);
  free(host);

  return 0;
}
