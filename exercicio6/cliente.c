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


void doit(int sockfd, FILE *file);

int main(int argc, char **argv) {
   int sockfd; //socket descriptor for client
   int port;                  
   char * ip;             
   char error[MAXLINE + 1];     
   struct sockaddr_in servaddr;  

   int running = 1;
   int maxDescriptor;
   fd_set rset; //read set of descriptors
   long timeout;
   struct timeval selTimeout;

   //buffers de recepcao e envio
   char buffer_in[MAXLINE]; memset(buffer_in, 0, sizeof buffer_in);
   //char *buffer_out = NULL; size_t buffer_out_size = MAXLINE;
   char buffer_out[MAXLINE];

   if (argc < 6) {
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
   printf("#Conectado ao servidor\n");

   char const* const readFile = argv[4];
   FILE* fread = fopen(readFile, "r");
   if (fread == NULL){
      perror("file-read open");
      exit(1);
   }
   char const* const writeFile = argv[5];
   FILE* fwrite = fopen(writeFile, "a+");
   if (fwrite == NULL){
      perror("file-write open");
      exit(1);
   }

   int count = 0;
   int stdineof = 0;

   while (running) {
      FD_ZERO(&rset);
      FD_SET(STDIN_FILENO, &rset);
      if (stdineof == 0) {
         FD_SET(fileno(fread), &rset);
         if (fileno(fread) > maxDescriptor) maxDescriptor = fileno(fread);
      }
      FD_SET(sockfd, &rset);
      if (sockfd > maxDescriptor) maxDescriptor = sockfd;

      selTimeout.tv_sec = timeout;
      selTimeout.tv_usec = 0;
      
      /* Suspend program until descriptor is ready or timeout */
      if (select(maxDescriptor + 1, &rset, NULL, NULL, &selTimeout) == 0)
         printf("No echo requests for %ld secs...Server still alive\n", timeout);
      else {
         if (FD_ISSET(0, &rset)) { /* Check keyboard */
            printf("#Shutting down client\n");
            getchar();
            running = 0;
         }

         if (FD_ISSET(fileno(fread), &rset)) { /* input is readable */
            while (fgets(buffer_out, sizeof(buffer_out), fread)) {
               if (send(sockfd, buffer_out, strlen(buffer_out), 0) == -1) {
                  perror("send");
                  exit(1);
               } 
               printf("%sok", buffer_out);
               count++;
               printf("%d\n", count);
            }

            fclose(fread);
            stdineof = 1;
            shutdown(sockfd, SHUT_WR); //close for writing
            FD_CLR(fileno(fread), &rset);
            continue;
         }

         if (FD_ISSET(sockfd, &rset)) {
            if ((read(sockfd, buffer_in, MAXLINE)) > 0) {
               printf("OK\n");
               printf("%d\n", count);
               printf(buffer_in);
               fprintf(fwrite, buffer_in);
               fflush(fwrite);

               memset(buffer_in, 0, sizeof buffer_in); //clear the buffer
            }
            else {
               if (stdineof == 1) {
                  running = 0;
                  shutdown(sockfd, SHUT_RD); //close for writing
                  FD_CLR(sockfd, &rset);
                  continue;
               }
            }
         }
      }
   }

   printf("\n#progam ended\n");
   exit(0);
}


