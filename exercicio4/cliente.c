#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h> //SIGKILL

#define MAXLINE 4096
#define MAXDATASIZE 100

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

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port   = htons(8000);

  inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
  connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  pid_t childpid;
  int aux;
  while(1){
    //se recebe o comando do servidor e se executa
    memset(buffer, 0, sizeof buffer);
    if (read(sockfd, buffer, MAXDATASIZE) == -1){
      perror("read");
      exit(1);
    }
    printf("comando recibido: %s\n", buffer);

    //se o comando e igual ao quit, se fecha a conecao, e se termina o programa
    if (strcmp(buffer, "quit") == 0){
      close(sockfd);
      if (childpid > 0) kill(childpid, SIGKILL); //se poderia identificar o child com o pid (se se quer apagar um child process especifico)
      printf("o servidor termino o processo de comunicacao\n");
      exit(0);
    }
    

    if ((childpid = fork()) == 0){
      //close(sockfd);
      //aux = execl("/bin/sh", "sh", "-c", strcat(buffer, " 2> /dev/null || exit 1"), (char *) NULL);
      aux = execl("/bin/sh", "sh", "-c", buffer, (char *) NULL);
    }

    printf("%d/n", aux);
    if (aux != 0) {
      memset(buffer, 0, sizeof buffer);
      strcpy(buffer, "Comando invalido");
    }

    //se envia o comando ao servidor se o comando foi executado corretamente, e se envia 'Comando invalido'
    //quando aconteceu algum erro
    if (send(sockfd, buffer, strlen(buffer), 0) == -1){
      perror("send");
      exit(1);
    }
    printf("se envio a mensagem: %s\n", buffer);
  }
  
}
