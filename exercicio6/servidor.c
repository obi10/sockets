#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <arpa/inet.h>

#include "basic.h"
#include "socket_helper.h"

//#define LISTENQ 10              
#define MAXDATASIZE 4096         
#define EXIT_COMMAND "exit\n"

void doit(int connfd, struct sockaddr_in clientaddr);

int main (int argc, char **argv) {
   int    listenfd,              
          connfd,            
          port,
          backlog;
   char *ip;                  
   struct sockaddr_in servaddr;  
   char   error[MAXDATASIZE + 1];     

   if (argc != 4) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <Port>");
      perror(error);
      exit(1);
   }

   ip = argv[1];
   port = atoi(argv[2]);
   backlog = atoi(argv[3]);

   listenfd = Socket(AF_INET, SOCK_STREAM, 0);


   servaddr = ServerSockaddrIn(AF_INET, ip, port);


   Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   Listen(listenfd, backlog); //valor do backlog inserido na linha do comando

   Signal(SIGCHLD, sig_chld); //para chamar waitpid()
   for ( ; ; ) {
      pid_t pid;

      struct sockaddr_in clientaddr;
      socklen_t clientaddr_len = sizeof(clientaddr);

      connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientaddr_len);
      printf("#cliente aceitado\n");

      if ((pid = fork()) == 0) {
         Close(listenfd);
         
         doit(connfd, clientaddr);

         Close(connfd);
         printf("%s\n", "\n#task finished");

         exit(0);
      }

      Close(connfd);
   }
   
   return(0);
}

void doit(int connfd, struct sockaddr_in clientaddr) {
   char recvline[MAXDATASIZE];
   memset(recvline, 0, sizeof recvline);

   ssize_t bytes_read;
   while((bytes_read = read(connfd, recvline, MAXDATASIZE)) > 0) {
      printf("%zu\n", bytes_read);
      if (send(connfd, recvline, strlen(recvline), 0) == -1) {
         perror("send");
         exit(1);
      } 
      printf("%sok", recvline);
      memset(recvline, 0, sizeof recvline);
   }

}
