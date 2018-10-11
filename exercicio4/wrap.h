#ifndef WRAP_H_   /* Include guard */
#define WRAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h> //avoid implicit declaration of inet_ntop function
#include <sys/mman.h>
#include <sys/wait.h>

#include <signal.h> //SIGKILL (cliente.c)

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXLINE 4096 //(cliente.c)

int Socket(int family, int type, int flags);
void Bind(int listen_socket, struct sockaddr_in server_address);
void Listen(int listen_socket, int backlog);
int Accept(int listen_socket, struct sockaddr_in client_address, FILE *fp, time_t ticks);
void ipPortaServidor(struct sockaddr_in server_address, FILE *fp);
void ipPortaCliente(struct sockaddr_in client_address, FILE *fp, time_t ticks);
void Send(int socket, char *buffer, size_t length, int flags);
void Read(int socket, char *buffer, size_t length);
int comandoCliente(char *command, int connfd, FILE *fp, time_t ticks);
void Connect(int client_socket, struct sockaddr_in server_address);

#endif // WRAP_H_