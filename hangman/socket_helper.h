#ifndef SOCKET_HELPER
#define SOCKET_HELPER

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h> //declaration of memset

#include "basic.h"

#define MAXDATASIZE 4096
#define MAXWORDSIZE 10 //tamanho maximo da palavra
#define MAXNUMWORDS 3
#define SIZE 5

#define EXIT_COMMAND "exit\n"

int Socket(int domain, int type, int protocol);

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int Listen(int sockfd, int backlog);

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int Close(int fd);

int Getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

struct sockaddr_in ServerSockaddrIn(int family, const char *ip, unsigned short port);

struct sockaddr_in ClientSockaddrIn(int family, const char *ip, unsigned short port);

typedef void Sigfunc(int);
Sigfunc * Signal (int signo, Sigfunc *func);

void sig_chld(int signo);

ssize_t my_read(int fd, char *ptr, char *read_buf);

ssize_t readline(int fd, void *vptr, size_t maxlen, char *read_buf);

ssize_t
Readline(int fd, void *ptr, size_t maxlen);

ssize_t
writen(int fd, const void *vptr, size_t n);

ssize_t
Read(int fd, void *buf, size_t nbyte);

ssize_t
chargeFileMatrix(FILE *file, char matrix[MAXNUMWORDS][MAXWORDSIZE + 1]);

#endif