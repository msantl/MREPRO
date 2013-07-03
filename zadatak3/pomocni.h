#ifndef POMOCNI_H
#define POMOCNI_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define S_PORT "1234"
#define I_PORT 1234

#define MAXLENGTH 65536
#define BACKLOG 10

int Socket(int, int ,int);
void Close(int);

void Bind(int, struct sockaddr*, socklen_t);

void Connect(int, struct sockaddr*, socklen_t);

void Listen(int, int);

int Accept(int, struct sockaddr*, socklen_t*);

int WriteAll(int, char*, int);

int ReadAll(int, char*, int);

int Send(int, char*, int, int);

int Recv(int, char*, int, int);

#endif

