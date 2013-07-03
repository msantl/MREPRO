#ifndef POMOCNI_H
#define POMOCNI_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

#include <err.h>
#include <errno.h>

#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

// default port
#define PORT 3737
#define S_PORT "3737"

// maximal length
#define MAXLEN 1024

// define service
#define SERVICE   80
#define S_SERVICE "80"

// socket
int Socket(int, int, int);

// bind
void Bind(int, const struct sockaddr*, int);

// close
void Close(int);

#endif
