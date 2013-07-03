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
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

// default port
#define PORT 7
#define S_PORT "7"

// maximal length
#define MAXLEN 1024

// socket
int Socket(int, int, int);

// bind
void Bind(int, const struct sockaddr*, int);

// close
void Close(int);

// pointer to integer 
void p_to_i(char *, int*);

// dohvati trenutno vrijeme
uint64_t my_getcurrtime();

#endif
