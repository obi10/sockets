#include "wrap.h"

//*******************************************************************************************
int Socket(int family, int type, int flags){
  int sockfd;
  if ((sockfd = socket(family, type, flags)) < 0) {
    perror("socket");
    exit(1);
  }
	else return sockfd;
}

void Bind(int listen_socket, struct sockaddr_in server_address){
  if (bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
    perror("bind");
    exit(1);
  }
}

void Listen(int listen_socket, int backlog){
  if (listen(listen_socket, backlog) == -1) {
    perror("listen");
    exit(1);
  }
}

int Accept(int listen_socket, struct sockaddr_in client_address, FILE *fp, time_t ticks){
  int sockfd;
  socklen_t client_address_len = sizeof(client_address);
  if ((sockfd = accept(listen_socket, (struct sockaddr *)&client_address, &client_address_len)) == -1) {
    perror("accept");
    exit(1);
  }
  else{
    ipPortaCliente(client_address, fp, ticks);
	  return sockfd;
	}
}

void ipPortaServidor(struct sockaddr_in server_address, FILE *fp){
  //obter o ip address - servidor
  char myIP[16]; //variavel onde se almacenara o ipaddress do servidor
  unsigned int myPort; //variavel onde se almacenara a porta que usa o servidor

  inet_ntop(AF_INET, &server_address.sin_addr, myIP, sizeof(myIP));
  myPort = ntohs(server_address.sin_port);

  printf("IP_servidor: %s\n", myIP);
  printf("PORTA_servidor: %u\n", myPort);
  
  if (fp != NULL){
    fprintf(fp, "\n---Dados do servidor---\nIP:%s\nPORTA:%u\n", myIP, myPort);
    fflush(fp);
  }
}

void ipPortaCliente(struct sockaddr_in client_address, FILE *fp, time_t ticks){
  //obter o ip address do novo cliente aceitado
  printf("\nnovo cliente aceitado\n");
  char clientIP[16];
  unsigned int clientPort;

  inet_ntop(AF_INET, &client_address.sin_addr, clientIP, sizeof(clientIP));
  clientPort = ntohs(client_address.sin_port);

  printf("IP_cliente: %s\n", clientIP);
  printf("PORTA_cliente: %u\n", clientPort);
  ticks = time(NULL);
  fprintf(fp, "IP:%sPORTA:%uCONECAO:%.24s\n", clientIP, clientPort, ctime(&ticks));
  fflush(fp);
}

void Send(int socket, char *buffer, size_t length, int flags){
	if (send(socket, buffer, length, flags) == -1){
		perror("send");
		exit(1);
	}
}

void Read(int socket, char *buffer, size_t length){
	if (read(socket, buffer, length) == -1){
		perror("read");
		exit(1);
	}
}

int comandoCliente(char *command, int connfd, FILE *fp, time_t ticks){
  char mensagem[MAXDATASIZE];
  strcpy(mensagem, command);

  //se envia o comando ao cliente
  Send(connfd, mensagem, strlen(mensagem), 0);

  if (strcmp(command, "quit") == 0) return -1;

  //se recebe a resposta do cliente
  memset(mensagem, 0, sizeof mensagem);
  Read(connfd, mensagem, MAXDATASIZE);
  printf("%s", mensagem);
  ticks = time(NULL);
  fprintf(fp, "%sT:%.24s\n", mensagem, ctime(&ticks));
  fflush(fp);
  return 0;
}

void Connect(int client_socket, struct sockaddr_in server_address){
  if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
    perror("connect");
    exit(1);
  }
}