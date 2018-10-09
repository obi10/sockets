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

#define LISTENQ 10
#define MAXDATASIZE 100

int Socket(int family, int type, int flags);
void Bind(int listen_socket, struct sockaddr_in server_address);
void Listen(int listen_socket, int backlog);
int Accept(int listen_socket, struct sockaddr_in client_address, int c);
void ipPortaServidor(struct sockaddr_in server_address);
void ipPortaCliente(int c, struct sockaddr_in client_address);
int comandoCliente(char *command, int connfd, int c);
int verificarNumeroCliente(int numero);

int main (int argc, char **argv) {
  int	listenfd, connfd;
  struct sockaddr_in servaddr, cliaddr;

  if (argc != 2){ //o servidor deve receber a porta onde vai a escutar como argumento
    printf("ERROR - Indicar a porta\n");
    exit(1);
  }

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons((unsigned short)strtoul(argv[1], NULL, 0)); //se usa como porta o argumento inserido
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  Bind(listenfd, servaddr);

  printf("servidor himeros iniciado ...\n");
  ipPortaServidor(servaddr);

  Listen(listenfd, LISTENQ);

  printf("escutando clientes ...\n");

  int c = 1;
  pid_t waitcommandpid; //processo 1: processo que se executara em paralelo com o processo 2
  static int *wait;
  wait = mmap(NULL, sizeof *wait, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *wait = 0;
  pid_t childpid;
  char cadeia[MAXDATASIZE];

  char *cliente = NULL;
  char *comando = NULL;
  char *comando_aux; 
  comando_aux = mmap(NULL, sizeof *comando, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  char *pos = NULL;

  int *childProcesses2;
  childProcesses2 = mmap(NULL, LISTENQ*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  if ((waitcommandpid = fork()) == 0){ //processo 1
    while (1){
      if (*wait == 1){
        cliente = NULL;
        comando = NULL;
        pos = NULL;
        //se le uma cadeia de caracteres do teclado
        printf("inserir o comando a enviar (#cliente!comando): ");
        fgets(cadeia, MAXDATASIZE, stdin);
        //se verifica que o caractere '!' exista no comando
        if (((pos = strchr(cadeia, '!')) != NULL)){
          //se verifica que os caracteres antes do '!', sejan numeros e corresponda ao numero de um cliente aceitado
          cliente = strtok(cadeia, "!");
          //printf("%s\n", cliente);
          if (atoi(cliente) != 0 && verificarNumeroCliente(atoi(cliente) == 1)) {
            int numCliente = atoi(cliente);

            comando = strtok(NULL, "");
            //se apaga o caractere '\n'
            pos = NULL;
            if ((pos = strchr(comando, '\n')) != NULL) *pos = '\0';
            printf("comando: %s\n", comando);
            strcpy(comando_aux, comando);

            //se face a chamada ao child process respectivo do processo 2
            childProcesses2[numCliente-1] = 1;
            sleep(3);

          }
          else printf("verificar o numero do cliente inserido\n"); 
        }
        else printf("Seguir o padrao (nao esquecer !)\n");
      }
    }
  }

  for ( ; ; ) { //processo 2 (processo principal)
    connfd = Accept(listenfd, cliaddr, c);
    //ipPortaCliente(c, cliaddr);

    *wait = 1;

    if ((childpid = fork()) == 0){
      int numeroAsignado = c;
      close(listenfd);
      while(1){
        if (childProcesses2[c-1] == 1){
          //printf("valor do comando no processo 2: %s\n", comando_aux);
          if (comandoCliente(comando_aux, connfd, c) == -1){ //-1: se termina a conecao com ese cliente
            childProcesses2[numeroAsignado-1] = 0;
            close(connfd);
            exit(0);
          }
          childProcesses2[numeroAsignado-1] = 0;
        }
      }
    }

    close(connfd);
    c++;

  }
  return(0);
}



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

int Accept(int listen_socket, struct sockaddr_in client_address, int c){
  int sockfd;
  socklen_t client_address_len = sizeof(client_address);
  if ((sockfd = accept(listen_socket, (struct sockaddr *)&client_address, &client_address_len)) == -1) {
    perror("accept");
    exit(1);
  }
  else{
  printf("\ncliente#%d aceptado\n", c);
  char clientIP[16];
  unsigned int clientPort;

  inet_ntop(AF_INET, &client_address.sin_addr, clientIP, sizeof(clientIP));
  clientPort = ntohs(client_address.sin_port);

  printf("IP_cliente#%d: %s\n", c, clientIP);
  printf("PORTA_cliente#%d: %u\n", c, clientPort);

  return sockfd;
}
}

void ipPortaServidor(struct sockaddr_in server_address){
  //obter o ip address - servidor
  char myIP[16]; //variavel onde se almacenara o ipaddress do servidor
  unsigned int myPort; //variavel onde se almacenara a porta que usa o servidor

  inet_ntop(AF_INET, &server_address.sin_addr, myIP, sizeof(myIP));
  myPort = ntohs(server_address.sin_port);

  printf("IP_servidor: %s\n", myIP);
  printf("PORTA_servidor: %u\n", myPort);
}

void ipPortaCliente(int c, struct sockaddr_in client_address){
  //obter o ip address - cliente#c
  printf("cliente#%d aceptado\n", c);
  char clientIP[16];
  unsigned int clientPort;

  inet_ntop(AF_INET, &client_address.sin_addr, clientIP, sizeof(clientIP));
  clientPort = ntohs(client_address.sin_port);

  printf("IP_cliente#%d: %s\n", c, clientIP);
  printf("PORTA_cliente#%d: %u\n", c, clientPort);
}

int comandoCliente(char *command, int connfd, int c){ //se deve passar como parametro o 'connfd'
                                                      //do cliente que se quer manda a mensagem
  char mensagem[MAXDATASIZE];
  strcpy(mensagem, command);

  //se envia a cadeia ao cliente
  if (send(connfd, mensagem, strlen(mensagem), 0) == -1){
    perror("send");
    exit(1);
  }
  printf("comando enviado ao cliente#%d\n", c); 

  if (strcmp(command, "quit") == 0) return -1;

  //se recebe o caractere enviado pelo cliente
  memset(mensagem, 0, sizeof mensagem);
  if (read(connfd, mensagem, MAXDATASIZE) == -1){
    perror("read");
    exit(1);
  }
  printf("mensagem recebida do cliente#%d: %s\n", c, mensagem);
  return 0;
}

int verificarNumeroCliente(int numero){
  //se verifica que o numero inserido seja de alguns dos cliente aceitados
  return 1;
}
