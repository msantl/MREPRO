#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "prog.h"

int main(int argc, char **argv) {
  // variables
  int gai_error;

  uint16_t port;

  char adresa[100];

  struct addrinfo hints, *result;

  // we save infor about hostname and service here
  const char *hostname;
  const char *servicename;

  // get info about service and hostname from command line arguments
  if (argc < 3) {
    err(1, "Premalo argumenata\nUsage: prog [-t|-u] [-x] [-h|-n] hostname servicename");
  }

  hostname = argv[argc - 2];
  servicename = argv[argc - 1];

  // we save options here
  char ch;

  // store the given options here
  int protocol = -1;
  ENDIAN endian = E_NONE;
  VIEW view = V_NONE;

  // check given options
  while ((ch=getopt(argc, argv, "tuxnh")) != -1) {
    switch (ch) {
      case 't':
        if (protocol == IPPROTO_UDP) {
          err(1, "Protokol nije jednoznacno odreden opcijama!");
        }
        protocol = IPPROTO_TCP;
        break;
      case 'u':
        if (protocol == IPPROTO_TCP) {
          err(1, "Protokol nije jednoznacno odreden opcijama!");
        }
        protocol = IPPROTO_UDP;
        break;
      case 'x':
        if (view == DEC) {
          err(1, "Prikaz nije jednoznacno odreden opcijama!");
        }
        view = HEX;
        break;
      case 'n':
        if (endian == HOST) {
          err(1, "Endian nije jednoznacno odreden opcijama!");
        }
        endian = NETWORK;
        break;
      case 'h':
        if (endian == NETWORK) {
          err(1, "Endian nije jednoznacno odreden opcijama!");
        }
        endian = HOST;
        break;
      default:
        printf("Nepostojeca opcija %c !\n", ch);
    }
  }
  // setting the default value for protocol
  if (protocol == -1) {
    protocol = IPPROTO_TCP;
  }
  // setting the default value for endian
  if (endian == E_NONE) {
    endian = HOST;
  }
  // setting the default value for view
  if (view == V_NONE) {
    view = DEC;
  }

  // fetch info about hostname

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_flags |= AI_CANONNAME;
  hints.ai_protocol |= protocol;

  if ((gai_error = getaddrinfo(hostname, servicename, &hints, &result)) != 0) {
    errx(1, "%s", gai_strerror(gai_error));
  }

  if (result == NULL) {
    errx(1, "%s", "Lista rezultata je prazna");
  } else {

    inet_ntop(
      result->ai_family,
      &((struct sockaddr_in *)result->ai_addr)->sin_addr,
      adresa,
      sizeof(adresa)
    );

    port = ((struct sockaddr_in *)result->ai_addr)->sin_port;

    if (endian == HOST) {
      port = ntohs(port);
    }

    if (view == DEC) {
      printf("%s (%s) %d\n", adresa, result->ai_canonname, port);
    } else {
      printf("%s (%s) %.4x\n", adresa, result->ai_canonname, port);
    }
  }

  freeaddrinfo(result);

  return 0;
}
