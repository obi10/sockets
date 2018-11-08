#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h> //for struct timeval {}

#include "basic.h"
#include "socket_helper.h"

#define MAXLINE 4096
#define EXIT_COMMAND "exit\n"    


void doit(int sockfd, char **argv);

int main(int argc, char **argv) {
   int sockfd; //socket descriptor for client
   int port;                  
   char * ip;
   char buffer[MAXLINE]; //se almacena as mensagens recebidas                
   char error[MAXLINE + 1];       
   struct sockaddr_in servaddr;  

   int running = 1;
   int maxDescriptor;
   fd_set sockSet; //set of socket descriptors for select
   long timeout;
   struct timeval selTimeout;

   if (argc < 5) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress, Port>");
      perror(error);
      exit(1);
   }

   timeout = atol(argv[1]);

   maxDescriptor = -1;

   sockfd = Socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd > maxDescriptor) maxDescriptor = sockfd;

   ip = argv[2];
   port = atoi(argv[3]);

   servaddr = ClientSockaddrIn(AF_INET, ip, port);

   Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
   printf("Conectado ao servidor\n");

   while (running) {
      FD_ZERO(&sockSet);
      FD_SET(STDIN_FILENO, &sockSet);
      FD_SET(sockfd, &sockSet);

      selTimeout.tv_sec = timeout;
      selTimeout.tv_usec = 0;
      
      /* Suspend program until descriptor is ready or timeout */
      if (select(maxDescriptor + 1, &sockSet, NULL, NULL, &selTimeout) == 0)
      printf("No echo requests for %ld secs...Server still alive\n", timeout);
      else {
         if (FD_ISSET(0, &sockSet)) { /* Check keyboard */
            printf("Shutting down client\n");
            getchar();
            running = 0;
         }

         if (FD_ISSET(sockfd, &sockSet)) {
            memset(buffer, 0, sizeof buffer);
            Read(sockfd, buffer, MAXLINE);
            doit(sockfd, argv);
         }
      }
   }

   exit(0);
}

void doit(int sockfd, char **argv) {

   char response[MAXLINE + 1];
   char const* const readFile = argv[4];
   FILE* file = fopen(readFile, "r");
   char line[MAXLINE + 1];

   while(fgets(line, sizeof(line), file)) {
      printf("enviando ao servidor:%s", line);
      send(sockfd, line, strlen(line), 0);

      while(read(sockfd, response, MAXLINE) > 0) {
         printf("%s", response);
      }
   }

   fclose(file);

}


