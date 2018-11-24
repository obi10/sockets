#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <arpa/inet.h>

#include "socket_helper.h"


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
      strcpy(error,"usage: ");
      strcat(error,argv[0]);
      strcat(error," <ip_address, port, backlog>");
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

      if ((pid = fork()) == 0) {
         Close(listenfd);
         
         doit(connfd, clientaddr);

         Close(connfd);

         exit(0);
      }

      Close(connfd);
   }
   
   return(0);
}

void doit(int connfd, struct sockaddr_in clientaddr) {
   char *recvline = malloc((MAXDATASIZE + 1)*sizeof(char));

   //using Readline: the buffer could almacenate more than one line before enter
   //to the next line of the code, but the server will perform the echo task line by line
   while(read(connfd, recvline, MAXDATASIZE) > 0) {
      if (writen(connfd, recvline, strlen(recvline)) != strlen(recvline)) {
         perror("write");
         exit(1);
      }
      memset(recvline, 0, MAXDATASIZE); //important
   }
}
