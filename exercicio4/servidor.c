#include "wrap.h"

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
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  Bind(listenfd, servaddr);

  printf("servidor iniciado ...\n");
  ipPortaServidor(servaddr);

  Listen(listenfd, LISTENQ);

  int c = 1;
  printf("\n---sessao %d---\nescutando clientes ...\n", c);

  pid_t waitcommandpid; //processo 1: processo que se executara em paralelo com o processo 2
  pid_t childpid;

  char comando[MAXDATASIZE];

  char *comando_aux; 
  comando_aux = mmap(NULL, sizeof *comando, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  char *pos = NULL;

  static int *wait;
  wait = mmap(NULL, sizeof *wait, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *wait = 0;
  static int *allowClients;
  allowClients = mmap(NULL, sizeof *allowClients, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *allowClients = 0;
  static int *sessao;
  sessao = mmap(NULL, sizeof *wait, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *sessao = 0;

  if ((waitcommandpid = fork()) == 0){ //processo 1 (processo de recepcao dos comandos)
    while (1){
      if (*wait == 1){
        memset(comando, '\0', sizeof(comando));
        //se le uma cadeia de caracteres do teclado
        printf("inserir o comando a enviar ao(s) cliente(s):\n");
        fgets(comando, MAXDATASIZE, stdin);

        //se apaga o caractere '\n'
        pos = NULL;
        if ((pos = strchr(comando, '\n')) != NULL) *pos = '\0';
        if (!(comando != NULL && (comando[0] == '\0'))){ //evitar erros no fgets (quando  usuario so da 'enter')
          strcpy(comando_aux, comando);
          //se habilita os 'child processes' do processo 2
          *allowClients = 1;
          sleep(1);
        }        
      }
      if (*sessao == 1) { //quando se termina o primeira sessao
        char saida[MAXDATASIZE];
        printf("Deseja iniciar outra sessao? (sim/nao)");
        fgets(saida, MAXDATASIZE, stdin);
        if (strcmp(saida, "sim\n") == 0){
          c ++;
          printf("\n---sessao %d---\nescutando clientes ...\n", c);
          *sessao = 0;  
        }
        if (strcmp(saida, "nao\n") == 0){
          printf("programa finalizado");
          exit(0); //se deveria fechar o sockets listenfd do processo 2
          return(0);
        }
      }
    }
  }

  for ( ; ; ) { //processo 2 (processo principal)
    connfd = Accept(listenfd, cliaddr);
    //ipPortaCliente(cliaddr); //ainda nao sei porque o ip e porta do cliente nao se pode imprimir aqui

    *wait = 1; //quando se aceite ao primeiro cliente, se comecara a pedir os comandos a enviar

    if ((childpid = fork()) == 0){
      close(listenfd);
      while(1){
        if (*allowClients == 1){
          if (comandoCliente(comando_aux, connfd) == -1){ //-1: se termina a conecao com os clientes
            close(connfd);
            *allowClients = 0;
            *wait = 0;
            *sessao = 1;
            exit(0); //culmina o child process
          }
          *allowClients = 0;
        }
      }
    }

    close(connfd);
    c++;

  }
  return(0);
}
