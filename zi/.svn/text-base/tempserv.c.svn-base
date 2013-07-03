#include "pomocni.h"

int noviSocket(char *node, char *port, struct sockaddr *server, socklen_t *server_l, int proto, int povezi) {
  struct addrinfo hints, *res;

  int status;
  int sockfd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = proto;        // moze biti SOCK_STREAM ili SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;      // it's ignored if the node is not NULL

  if ((status = getaddrinfo(node, port, &hints, &res)) != 0) {
    Errx(1, "%s", gai_strerror(status));
  } else if (res == NULL) {
    Errx(1, "getaddrinfo returned no results");
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

int main(int argc, char **argv) {
  // spremnizi za zahtjev, odgovor i datagram
  char request[MAXLENGTH];
  char answer[MAXLENGTH];
  char datagram[MAXLENGTH];

  // uzorci prema kojima nalazimo ime stanice i temperaturu
  const char* name_pattern = "<td colspan=\"2\" id=\"mobilestanica\">";
  const char* temp_pattern = "<td colspan=\"2\" id=\"temp\">";

  // spremnici u koje spremamo vrijednosti iz HTTP odgovora
  char temperatura[MAXLENGTH];
  char stanica[MAXLENGTH];

  int i, len;
  int ch;
  int sockfd, newfd;
  int status;
  int ispravan_odgovor;

  char* char_p;

  socklen_t server_l;
  struct sockaddr server;

  socklen_t client_l;
  struct sockaddr client;

  // pomocni.h, koristi funkcija omotac Errx
  daemon_proc = 0;

  char* port = (char *)malloc(sizeof(char) * 8);
  strcpy(port, S_PORT);

  // dohvati dane argumente naredbenog retka
  while ((ch = getopt(argc, argv, "p:")) != -1) {
    switch (ch) {
      case 'p':
        strcpy(port, optarg);
        break;
      default:
        Errx(1, "Usage: ./tempserv [-p port]");
        break;
      }
    }

  // ako ih ima viska, dojavi pogresku
  if (argc - optind != 0) {
    Errx(1, "Usage: ./tempserv [-p port]");
  }

  // ako je navedena nepostojeca opcija, server zavrsava s radom

  sockfd = noviSocket(NULL, port, NULL, NULL, SOCK_DGRAM, 1);

  while (1) {
    // cekaj na datagram od klijenta
    memset(datagram, 0, sizeof datagram);
    client_l = sizeof(client);
    status = recvfrom(sockfd, datagram, sizeof datagram, 0, &client, &client_l);

    if (status <= 1) {
      // prazan (\n) datagram
      strcpy(datagram, "dhmz_maksimir");
    } else {
      // obrisi newline znak
      datagram[strlen(datagram) - 1] = 0;
    }

    // sastavi zaglavlje
    memset(request, 0, sizeof request);
    sprintf(request, "GET /mobile.php?stanica=%s HTTP/1.1\r\n", datagram);
    strcat(request, "Host: m.pljusak.com\r\n");
    strcat(request, "Connection: close\r\n");
    strcat(request, "\r\n");

    // spoji se na posluzitelj m.pljusak.com na http (80)
    newfd = noviSocket("m.pljusak.com", "http", &server, &server_l, SOCK_STREAM, 0);
    Connect(newfd, &server, server_l);

    // posalji HTTP GET zahtjev posluzitelju m.pljusak.com
    WriteAll(newfd, request, strlen(request));

    // ocisti privremene spreminke
    memset(stanica, 0, sizeof stanica);
    memset(temperatura, 0, sizeof temperatura);
    // kontrolna varijabla koja nam govori koliko smo informacija izlucili
    ispravan_odgovor = 0;

    while (1) {
      // citaj odgovor servera m.pljusak.com
      memset(answer, 0, sizeof answer);
      status = ReadAll(newfd, answer, sizeof(answer));

      // izvuci iz odgovora temperaturu i naziv stanice

      // nasli smo uzorak koji sadrzi ime stanice
      if ((char_p = strstr(answer, name_pattern)) != NULL) {
        len = 0;
        // +4 da ne odemo izvan skupa primljenih podataka
        for (i = char_p - answer; i + 4 < status; ++i) {
          if (strncmp(answer + i, "</td>", 5) == 0) {
            // kopiraj samo koristan dio
            strncpy(stanica, char_p + strlen(name_pattern), len - strlen(name_pattern));
            // nasli smo jos jednu znacajku
            ispravan_odgovor += 1;
            break;
          } else {
            len += 1;
          }
        }
      }

      // nasli smo uzorak koji sadrzi temperaturu
      if ((char_p = strstr(answer, temp_pattern)) != NULL) {
        len = 0;
        // +4 da ne odemo izvan skupa primljenih podataka
        for (i = char_p - answer; i + 4 < status; ++i) {
          if (strncmp(answer + i, "</td>", 5) == 0) {
            // kopiraj samo koristan dio
            strncpy(temperatura, char_p + strlen(temp_pattern), len - strlen(temp_pattern));
            // nasli smo jos jednu znacajku
            ispravan_odgovor += 1;
            break;
          } else {
            len += 1;
          }
        }
      }

      // citaj odgovor tako dugo dok ne procitamo manje od spremnika
      if (status != sizeof(answer)) break;
    }

    // zavrsili smo sa serverom m.pljusak.com
    close(newfd);

    // ocisti datagram
    memset(datagram, 0, sizeof datagram);

    if (ispravan_odgovor == 2) {
      // ako smo nasli oba podatka u HTTP dokumentu spremi ih u datagram

      // +1 da maknemo razmak
      strcpy(datagram, stanica + 1);
      strcat(datagram, ", ");
      strcat(datagram, temperatura);
      strcat(datagram, "\n");
    } else {
      // inace vrati odgovarajucu poruku
      strcpy(datagram, "Odgovor od posluzitelja nema trazene podatke\n");
    }

    // posalji datagram klijentu
    sendto(sockfd, datagram, strlen(datagram), 0, &client, client_l);
  }

  Close(sockfd);
  free(port);

  return 0;
}
