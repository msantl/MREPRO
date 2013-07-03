#ifndef POMOCNI_H
#define POMOCNI_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <unistd.h>
#include <syslog.h>
#include <signal.h>

#include <fcntl.h>

#include <stdarg.h>
#include <getopt.h>
#include <ctype.h>

#define S_PORT "1234"
#define I_PORT 1234

#define MAXLENGTH 4096
#define BACKLOG 10

#define MAXFD 64

int daemon_proc;

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

void Errx(int, char *, ...);

void Warnx(char *, ...);

#endif

