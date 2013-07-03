#include "pomocni.h"

#include <sys/time.h>
#include <string.h>
#include <ctype.h>

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

int noviSocket(char *node, char *port, struct sockaddr *server, socklen_t *server_l) {
  struct addrinfo hints, *res;

  int status;
  int sockfd;

  // host and port should be defined from here on
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;      // it's ignored if the node is not NULL

  if ((status = getaddrinfo(node, port, &hints, &res)) != 0) {
    Errx(daemon_proc ? LOG_ALERT : 1, "TFTP ERROR 0 ms45889: %s", gai_strerror(status));
  } else if (res == NULL) {
    Errx(daemon_proc ? LOG_ALERT : 1, "TFTP ERROR 0 ms45889: getaddrinfo error");
  }

  sockfd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (server != NULL) {
    *server = *(res->ai_addr);
  }

  if (server_l != NULL) {
    *server_l = sizeof(*server);
  }

  Bind(sockfd, res->ai_addr, res->ai_addrlen);

  freeaddrinfo(res);

  return sockfd;
}

int main(int argc, char **argv) {
  char buffer[MAXLENGTH];

  char file[MAXLENGTH];

  char filename[MAXLENGTH];
  char filename_path[MAXLENGTH];
  char filetype[MAXLENGTH];

  char server_adresa[32];
  char klijent_adresa[32];

  char *char_p, znak;
  FILE *datoteka;

  int ch;
  int sockfd, newfd;
  int msglen, status;
  int should_be_daemon = 0;
  int should_be_binary = 1;
  int zadnji_znak_oznaka_novog_reda;
  int oznaka_kraja_datoteke;
  int retransmission_count;

  uint16_t kod_poruke, broj_bloka, tip_zahtjeva, brojac_blokova;

  pid_t pid;

  socklen_t server_l;
  struct sockaddr server;

  socklen_t client_l;
  struct sockaddr client;

  struct sockaddr_in dijete;

  struct timeval timeout;
  fd_set readfs;

  char* port = (char *)malloc(sizeof(char) * 8);

  strcpy(port, S_PORT);

  while ((ch = getopt(argc, argv, "d")) != -1) {
    switch(ch) {
      case 'd':
        should_be_daemon = 1;
        break;
      default:
        errx(1, "Usage: ./tftpserver [-d] port_name_or_number");
        break;
    }
  }

  if (argc - optind != 1) {
    errx(1, "Usage: ./tcpserver [-d] port_name_or_number");
  } else {
    strcpy(port, argv[optind]);
    /*
     * provjeri ispravnost danog porta
     * da li je ok staviti NULL ili bas moram navesti ime protokola
     */
    if (sscanf(port, "%d", &status) == 0 && getservbyname(port, NULL) == NULL) {
      errx(1, "Invalid port name given");
    }
  }

  /*
   * ako treba napravi ga demonom
   */
  if (should_be_daemon) {
    if (daemon_init("MrePro tftpserver", LOG_FTP) < 0) {
      errx(1, "daemon failed to initialize!");
    }
  } else {
    daemon_proc = 0;
  }

  /*
   * NOTE: dalje se sva upozorenja/pogreske pisu sa
   * Errx(status, format, ...)
   * koja radi errx ili syslog ovisno o varijabli daemon_proc
   */

  sockfd = noviSocket(NULL, port, &server, &server_l);

  while (1) {
    // primi prvi datagram od klijenta koji opisuje njegove zelje
    // forkaj
    // neka child posluzi klijenta
    // mi nastavljamo u petlju
    memset(buffer, 0, sizeof buffer);

    client_l = sizeof(client);
    msglen = recvfrom(sockfd, buffer, sizeof(buffer), 0, &client, &client_l);

    if ((pid = fork()) < 0) {
      Errx(daemon_proc ? LOG_ALERT : 1, "TFTP ERROR 0 ms45889: fork error");
    } else if (pid == 0) {
      /*
       * dijete
       */
      Close(sockfd);

      // clear buffers
      memset(file, 0, sizeof file);
      memset(filename, 0, sizeof filename);
      memset(filetype, 0, sizeof filetype);

      // spremi klijentovu adresu
      inet_ntop(
        client.sa_family,
        &((struct sockaddr_in *)&client)->sin_addr,
        klijent_adresa,
        sizeof(klijent_adresa)
      );

      // spremi moju adresu
      inet_ntop(
        server.sa_family,
        &((struct sockaddr_in *)&server)->sin_addr,
        server_adresa,
        sizeof(server_adresa)
      );

      // napravi novi socket

      newfd = Socket(AF_INET, SOCK_DGRAM, 0);

      memset(&dijete, 0, sizeof(dijete));
      dijete.sin_family = AF_INET;
      dijete.sin_port = htons(0);
      dijete.sin_addr.s_addr = INADDR_ANY;

      Bind(newfd, (struct sockaddr *)&dijete, sizeof (struct sockaddr));

      /*
       * prvi zahtjev dolazi na dogovoreni port
       * odgovor saljemo klijentu s nekog drugog porta i na tom portu
       * nastavljamo priamti klijentove poruke
       */
      // read = 1
      // write = 2
      // data = 3
      // ack = 4
      // error = 5

      // 1) izdvoji tip zahtjeva
      memcpy(&tip_zahtjeva, buffer, 2);
      tip_zahtjeva = ntohs(tip_zahtjeva);

      // 2) izdvoji naziv datoteke u filename
      strcpy(filename, buffer + 2);

      // 3) izdvoji nacin prijenosa u filetype
      strcpy(filetype, buffer + 2 + strlen(filename) + 1);

      for (char_p = filetype; *char_p; ++char_p) {
        *char_p = tolower(*char_p);
      }

      if (tip_zahtjeva == 1) {
        // ovisno o filetype napuni file buffer
        // za netascii treba svaki \n pretvoriti u \r\n
        // za binary samo trpaj
        // velicina datagrama je 4 + 512
        // radi najvise 3 retransmisije, 3 sekunde izmedju svake
        // jedini dozvoljeni direktorij je /tftpboot

        if (strstr(filename, "/tftpboot/") == NULL) {
          strcpy(filename_path, "/tftpboot/");
        }
        strcat(filename_path, filename);

        if (strstr(filetype, "netascii")) {
          should_be_binary = 0;
          datoteka = fopen(filename_path, "r");
        } else {
          should_be_binary = 1;
          datoteka = fopen(filename_path, "rb");
        }

        // ako datoteka ne postoji ili nemamo prava otvaranja iste
        if (datoteka == NULL) {
          status = errno;

          kod_poruke = htons(5);
          memcpy(file, &kod_poruke, 2);

          broj_bloka = htons(3 - status);
          memcpy(file + 2, &broj_bloka, 2);

          strcpy(file + 4, strerror(status));

          sendto(newfd, file, 516, 0, &client, client_l);
          Close(newfd);
          Errx(daemon_proc ? LOG_ALERT : 1, "TFTP ERROR %d ms45889 %s", 3 - status , strerror(status));
        }

        // kreni s citanjem datoteke ovisno o should_be_binary zastavici
        // te svaki paket posalji najvise 3 puta dok ne dobis potvrdu ili
        // odustani

        // spremi podatke u datagram
        oznaka_kraja_datoteke = 0;
        brojac_blokova = 1;

        while (oznaka_kraja_datoteke == 0) {

          memset(file, 0, sizeof(file));
          memset(buffer, 0, sizeof(buffer));

          kod_poruke = htons(3);
          memcpy(file, &kod_poruke, 2);

          broj_bloka = htons(brojac_blokova);
          memcpy(file + 2, &broj_bloka, 2);

          ch = 0;
          char_p = file + 4;
          zadnji_znak_oznaka_novog_reda = 0;

          while (ch < 512) {
            if (zadnji_znak_oznaka_novog_reda == 0) {
              znak = fgetc(datoteka);
            }
            zadnji_znak_oznaka_novog_reda = 0;

            if (feof(datoteka)) {
              oznaka_kraja_datoteke = 1;
              break;
            }

            if (should_be_binary == 0 && zadnji_znak_oznaka_novog_reda == 0 && znak == '\n') {
              *char_p = '\r';
              ++ch;
              ++char_p;

              if (ch == 512) {
                zadnji_znak_oznaka_novog_reda = 1;
                break;
              }
            }

            *char_p = znak;

            ++ch;
            ++char_p;
          }

          retransmission_count = 0;

          // postavimo timeout na 3 sekunde
          timeout.tv_usec = 0;
          timeout.tv_sec = 3;

          while (retransmission_count < 3) {

            sendto(newfd, file, 4 + ch, 0, &client, client_l);
            FD_ZERO(&readfs);
            FD_SET(newfd, &readfs);

            // tu sad ide timeout
            status = select(newfd + 1, &readfs, NULL, NULL, &timeout);

            if (FD_ISSET(newfd, &readfs) == 0) {
              ++ retransmission_count;
              continue;

            } else {

              msglen = recvfrom(newfd, buffer, sizeof(buffer), 0, &client, &client_l);
              retransmission_count = 5;
            }

            memcpy(&tip_zahtjeva, buffer, 2);
            tip_zahtjeva = ntohs(tip_zahtjeva);

            memcpy(&broj_bloka, buffer + 2, 2);
            broj_bloka = ntohs(broj_bloka);

            // ako je ack i ne podudaraju se brojevi posalji ponovno
            if (tip_zahtjeva == 4 && broj_bloka != brojac_blokova) {
              retransmission_count = -1;
            }
          }

          // 3 puta smo timeoutali
          if (retransmission_count == 3) {
            memset(file, 0, sizeof file);
            // pogreska
            kod_poruke = htons(5);
            memcpy(file, &kod_poruke, 2);
            // illegal TFTP option
            broj_bloka = htons(0);
            memcpy(file + 2, &broj_bloka, 2);

            strcpy(file + 4, "TFTP timeout nad istim blokom 3 puta\0");

            sendto(newfd, file, 516, 0, &client, client_l);

            Errx(daemon_proc ? LOG_ALERT : 1, "TFTP ERROR %d ms45889: timeout 3 puta");
          }

          // pogledaj sta je klijent poslao ako nije ack (mozda error)
          // tip zahtjeva
          if (tip_zahtjeva == 5 ) {
            Close(newfd);
            Errx(daemon_proc ? LOG_ALERT : 1, "TFTP ERROR %d ms45889: klijent", broj_bloka);
          }

          ++ brojac_blokova;
        }

        fclose(datoteka);

        if (daemon_proc) {
          syslog(LOG_INFO, "%s->%s", klijent_adresa, filename);
        } else {
          printf("%s->%s\n", klijent_adresa, filename);
        }

      } else {
        // pogreska
        kod_poruke = htons(5);
        memcpy(file, &kod_poruke, 2);
        // illegal TFTP option
        broj_bloka = htons(4);
        memcpy(file + 2, &broj_bloka, 2);

        strcpy(file + 4, "TFTP server podrzava samo GET request\0");

        sendto(newfd, file, 516, 0, &client, client_l);
        Warnx("TFTP ERROR 4 ms45889: podrzano samo citanje");
      }

      /*
       * ubij proces dijeteta
       */
      Close(newfd);
      _exit(0);
    }
  }

  Close(sockfd);
  free(port);

  return 0;
}
