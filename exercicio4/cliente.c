#include "wrap.h"

int main(int argc, char **argv) {
  int sockfd;
  char buffer[MAXDATASIZE]; //onde se almacena o comando recebido
  char error[MAXLINE + 1];
  struct sockaddr_in servaddr;

  if (argc != 2) {
    strcpy(error,"uso: ");
    strcat(error,argv[0]);
    strcat(error," <IPaddress>");
    perror(error);
    exit(1);
  }

  sockfd = Socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port   = htons(8000);

  inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
  Connect(sockfd, servaddr);
  printf("conectado ao servidor...\n");
  ipPortaServidor(servaddr, NULL);

  pid_t childpid;
  while(1) {
    //se recebe o comando do servidor e se executa
    memset(buffer, 0, sizeof buffer);
    Read(sockfd, buffer, MAXDATASIZE);
    printf("comando recibido: %s\n", buffer);

    //se o comando e igual ao quit, se fecha a conecao, e se termina o programa
    if (strcmp(buffer, "quit") == 0) {
      close(sockfd);
      if (childpid > 0) kill(childpid, SIGKILL); //se apaga o child process en execucao (netstat -c -t -n)
      printf("o servidor termino o processo de comunicacao\n");
      //deveria apagarse os child processes antigos (previamente creados) antes de terminar o programa
      exit(0);
    }
    
    //pipe
    int link[2];
    if (pipe(link) == -1) {
      perror("pipe");
      exit(1);
    }

    if ((childpid = fork()) == 0) {
      close(sockfd);

      dup2(link[1], STDOUT_FILENO); //se duplica o file descriptor link[1]
      close(link[0]);
      close(link[1]);
      execl("/bin/sh", "sh", "-c", buffer, (char *) NULL); //o mesmo execl termina o child processs
                                                           //(nao se precisa exit(0))
    }

    close(link[1]);

    memset(buffer, 0, sizeof buffer); //se limpa o buffer
    Read(link[0], buffer, MAXDATASIZE); //se le a saida do execl

    if (buffer != NULL && buffer[0] == '\0') { //se verifica se o comando foi invalido
      memset(buffer, 0, sizeof buffer);
      strcpy(buffer, "comando invalido\n");
    }

    //se enviar a informacao do cliente mais a saida gerada pelo comando
    //se o comando foi invalido, se envia a mensagem "comando invalido"
    char myIP[16]; //variavel onde se almacenara o ipaddress do cliente (local)
    unsigned int myPort; //variavel onde se almacenara a porta que usa o cliente
    struct sockaddr_in my_addr;

    bzero(&my_addr, sizeof(my_addr));
    socklen_t len = sizeof(my_addr);
    getsockname(sockfd, (struct sockaddr *) &my_addr, &len);
    inet_ntop(AF_INET, &my_addr.sin_addr, myIP, sizeof(myIP));
    myPort = ntohs(my_addr.sin_port);

    char integer_string[5];
    sprintf(integer_string, "%d", myPort);

    int newSize = strlen(myIP) + strlen(integer_string) + strlen(buffer) + 10;
    char *newBuffer = (char *)malloc(newSize);
    strcpy(newBuffer, "IP:");
    strcat(newBuffer, myIP);
    strcat(newBuffer, "PORTA:");
    strcat(newBuffer, integer_string);
    strcat(newBuffer, "!");
    strcat(newBuffer, buffer);

    Send(sockfd, newBuffer, strlen(newBuffer), 0);
    printf("se envio a mensagem:\n%s", newBuffer);
  }  
}