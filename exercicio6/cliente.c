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

#include "socket_helper.h"


void doit(int sockfd, FILE *file);

int main(int argc, char **argv) {
   int sockfd; //socket descriptor for client
   int port;                  
   char * ip;             
   char error[MAXDATASIZE + 1];     
   struct sockaddr_in servaddr;  

   int running = 1, stdineof = 0;
   int maxDescriptor;
   fd_set rset; //read set of descriptors
   long timeout;
   struct timeval selTimeout;

   //reception and sending buffers (dynamic allocation)
   char *recvdata = malloc((MAXDATASIZE + 1)*sizeof(char));
   char *senddata = malloc((MAXDATASIZE + 1)*sizeof(char));
   if ((recvdata == NULL) || (senddata == NULL)) {
      printf("The allocation has failed\n");
      exit(1);
   }

   if (argc != 4) {
      strcpy(error,"usage: ");
      strcat(error,argv[0]);
      strcat(error," <timeout, ip_address_server, port_server>");
      perror(error);
      exit(1);
   }

   timeout = atol(argv[1]);

   maxDescriptor = -1;

   sockfd = Socket(AF_INET, SOCK_STREAM, 0);

   ip = argv[2];
   port = atoi(argv[3]);

   servaddr = ClientSockaddrIn(AF_INET, ip, port);

   Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   while (running) {
      FD_ZERO(&rset);
      if (stdineof == 0) {
         FD_SET(fileno(stdin), &rset);
         if (fileno(stdin) > maxDescriptor) maxDescriptor = fileno(stdin);
      }
      FD_SET(sockfd, &rset);
      if (sockfd > maxDescriptor) maxDescriptor = sockfd;

      selTimeout.tv_sec = timeout;
      selTimeout.tv_usec = 0;
      
      /* Suspend program until descriptor is ready or timeout */
      if (select(maxDescriptor + 1, &rset, NULL, NULL, &selTimeout) == 0)
         printf("No echo requests for %ld secs...Server still alive\n", timeout);
      else {
         if (FD_ISSET(fileno(stdin), &rset)) { /* input is readable */
            if (read(fileno(stdin), recvdata, MAXDATASIZE) == 0) {
               fclose(stdin);
               stdineof = 1;
               shutdown(sockfd, SHUT_WR); //close for writing
               FD_CLR(fileno(stdin), &rset);
               continue;               
            }
            if (writen(sockfd, recvdata, strlen(recvdata)) != strlen(recvdata)) {
               perror("write");
               exit(1);
            }

            memset(recvdata, 0, MAXDATASIZE); //clear the buffer
         }

         if (FD_ISSET(sockfd, &rset)) {
            if ((read(sockfd, senddata, MAXDATASIZE)) > 0) {
               printf(senddata);
               fflush(stdout);

               memset(senddata, 0, MAXDATASIZE);
            }
            else {
               if (stdineof == 1) {
                  running = 0;
                  shutdown(sockfd, SHUT_RD); //close for reading
                  FD_CLR(sockfd, &rset);
                  continue;
               }
            }
         }
      }
   }

   free(recvdata);
   free(senddata);
   exit(0);
}


