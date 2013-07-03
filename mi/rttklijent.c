#include "pomocni.h"

int main(int argc, char **argv) {
  uint32_t i, check_num;;
  uint32_t pid = getpid(), check_id;;
  uint64_t start, end, result;

  int status;
  int ch;
  int socket_fd;

  char *port = (char *)malloc(sizeof(char) * 8);
  char *host = (char *)malloc(sizeof(char) * 64);
  int TTL;
  int broj_paketa;

  socklen_t server_l;
  struct sockaddr server;
  struct addrinfo hints, *res;

  char paket[17];
  char odakle[32];

  uint64_t avg = 0;

  // default vrijednosti
  strcpy(port, S_PORT);
  broj_paketa = 4;
  TTL = -1;

  while ((ch = getopt(argc, argv, "p:t:c:")) != -1) {
    switch(ch) {
      case 'c':
        p_to_i(optarg, &broj_paketa);
        break;
      case 't':
        p_to_i(optarg, &TTL);
        break;
      case 'p':
        strcpy(port, optarg);
        break;
      default:
        errx(1, "Usage: ./rttklijent [-p naziv_ili_broj_porta] [-t TTL] [-c broj_paketa] naziv_ili_IP_servera");
        break;
    }
  }

  if (argc - optind != 1) {
    errx(1, "Usage: ./rttklijent [-p naziv_ili_broj_porta] [-t TTL] [-c broj_paketa] naziv_ili_IP_servera");
  } else {
    strcpy(host, argv[optind]);
  }

#ifdef DEBUG
  printf("Trying to connect to %s:%s\n", host, port);
  printf("Parametri: broj paketa:%d, TTL:%d\n", broj_paketa, TTL);
#endif

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo(host, port, &hints, &res);

  if (status != 0) {
    errx(1, "%s", gai_strerror(status));
  } else if(res == NULL) {
    errx(1, "getaddrinfo nije nista vratio!");
  }

  // pokreni klijenta
  socket_fd = Socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  server = *(res->ai_addr);

  // ako je TTL = -1 ostavi default
  if (TTL != -1) {
    // postavi TTL na danu vrijednost
    setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &TTL, sizeof(TTL));
  }

#ifdef DEBUG
  printf("Client is up and running\n");
#endif

  printf("%-3s %-7s %-17s\n", "#", "rtt(ms)", "IP");

  for (i = 1; i <= broj_paketa; ++i) {
    memset(paket, 0, sizeof paket); 
    start = my_getcurrtime();

    // spremi pid
    paket[0] = ((pid >> 24) & 0xff);
    paket[1] = ((pid >> 16) & 0xff);
    paket[2] = ((pid >> 8) & 0xff);
    paket[3] = ((pid) & 0xff);

    // spremi redni broj
    paket[4] = ((i >> 24) & 0xff);
    paket[5] = ((i >> 16) & 0xff);
    paket[6] = ((i >> 8) & 0xff);
    paket[7] = ((i) & 0xff);

    // spremi start
    paket[8]  = ((start >> 56) & 0xff);
    paket[9]  = ((start >> 48) & 0xff);
    paket[10] = ((start >> 40) & 0xff);
    paket[11] = ((start >> 32) & 0xff);
    paket[12] = ((start >> 24) & 0xff);
    paket[13] = ((start >> 16) & 0xff);
    paket[14] = ((start >> 8) & 0xff);
    paket[15] = ((start) & 0xff);

    server_l = sizeof(server);
    // posalji podatke
    sendto(socket_fd, paket, sizeof(paket), 0, &server, server_l);

    // ocisti spremink
    memset(paket, 0, sizeof paket);
    // cekaj na odgovor
    recvfrom(socket_fd, paket, sizeof(paket), 0, &server, &server_l);

    // iscitaj identifikator za provjeru
    check_id = 0;
    check_id |= (uint8_t)paket[0]; check_id <<= 8;
    check_id |= (uint8_t)paket[1]; check_id <<= 8;
    check_id |= (uint8_t)paket[2]; check_id <<= 8;
    check_id |= (uint8_t)paket[3]; 

    // iscitaj id datagrama
    check_num = 0;
    check_num |= (uint8_t)paket[4]; check_num <<= 8;
    check_num |= (uint8_t)paket[5]; check_num <<= 8;
    check_num |= (uint8_t)paket[6]; check_num <<= 8;
    check_num |= (uint8_t)paket[7]; 

    // iscitaj start iz datagrama
    start = 0;
    start |= (uint8_t)paket[8]; start <<= 8;
    start |= (uint8_t)paket[9]; start <<= 8;
    start |= (uint8_t)paket[10]; start <<= 8;
    start |= (uint8_t)paket[11]; start <<= 8;
    start |= (uint8_t)paket[12]; start <<= 8;
    start |= (uint8_t)paket[13]; start <<= 8;
    start |= (uint8_t)paket[14]; start <<= 8;
    start |= (uint8_t)paket[15];

    end = my_getcurrtime();
    result = (end - start);

#ifdef DEBUG
    printf("%llu us\n", result);
#endif

    // racunaj statistiku
    avg += (end - start);

    // izvuci ip iz podatak o serveru
    inet_ntop(
	res->ai_family,
	&((struct sockaddr_in *)&server)->sin_addr,
	odakle,
	sizeof(odakle)
    );

    // cisto da ne bude unused :)
    if (check_id != pid) {
     warnx("Kontrolni brojevi se razlikuju!");
    }

    // ispis rezultata
    printf("%-3d %-.3lf   %-17s\n", check_num, result / 1000.00, odakle);
  }

  printf("%s - Average rtt = %.3lf ms\n", host, avg / ((double)broj_paketa * 1000));

  // oslobodi resurse
  Close(socket_fd);
  freeaddrinfo(res);
  free(host);
  free(port);

  return 0;
}
