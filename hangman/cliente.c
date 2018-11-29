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

   int running = 1;
   //int stdineof = 0;
   int maxDescriptor;
   fd_set rset; //read set of descriptors
   long timeout;
   struct timeval selTimeout;

   char c = '\0'; //character inserted by the keyboard
   char gameMode = '\0';

   int initGame = 1;

   char arrivedData[SIZE + 1];
   char willSendData[SIZE + 1];

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

      memset(arrivedData, '\0', sizeof arrivedData);
      memset(willSendData, '\0', sizeof willSendData);

      FD_ZERO(&rset);
      FD_SET(STDIN_FILENO, &rset); //add keyboard to descriptor vector
      /*
      if (stdineof == 0) {
         FD_SET(fileno(stdin), &rset);
         if (fileno(stdin) > maxDescriptor) maxDescriptor = fileno(stdin);
      }
      */
      FD_SET(sockfd, &rset);
      if (sockfd > maxDescriptor) maxDescriptor = sockfd;

      selTimeout.tv_sec = timeout; //see how to avoid using timeout
      selTimeout.tv_usec = 0;

      // Suspend program until descriptor is ready or timeout
      if (select(maxDescriptor + 1, &rset, NULL, NULL, &selTimeout) == 0)
         printf("No echo requests for %ld secs...Server still alive\n", timeout);
      else {
         if (FD_ISSET(0, &rset)) { //check keyboard
            c = getchar();

            if (initGame) {
               gameMode = c;
               switch (c) {
                  case '1':
                     strcpy(willSendData, "#1");
                     break;
                  case '2':
                     strcpy(willSendData, "#2");
                     break;
                  case '3':
                     strcpy(willSendData, "#3");
                     break;
                  default:
                     continue; //skip the next code
               }
               writen(sockfd, willSendData, strlen(willSendData));
               initGame = 0;
               goto clearBuffer;
            }


            if (!initGame) {
               switch (gameMode) {
                  case '1':
                     willSendData[0] = c;
                     writen(sockfd, willSendData, strlen(willSendData));
                     willSendData[0] = '\0';
                     break;
                  case '2':
                     if (c != 's' && c != 'n') printf("Digite um caracter valido\n");
                     else {
                        if (c == 's') strcpy(willSendData, "2keep");
                        if (c == 'n') {
                           if ((int)arrivedData[1] == 0) {
                              strcpy(willSendData, "2reset");
                              initGame = 1;
                           }
                           if ((int)arrivedData[1] != 0)
                              strcpy(willSendData, "2ini");
                        }
                        writen(sockfd, willSendData, strlen(willSendData));
                        memset(willSendData, '\0', sizeof willSendData);                  
                     }                     
                     break;
                  case '3':
                     strcpy(willSendData, "#3");
                     writen(sockfd, willSendData, strlen(willSendData));
                     break;
                  default:
                     continue; //skip the next code
               }               
            }

clearBuffer:

            while ((c = getchar()) != '\n' && c != EOF) c = '\0'; //clear stdin buffer


            /*
            if (read(fileno(stdin), recvdata, MAXDATASIZE) == 0) {
               fclose(stdin);
               stdineof = 1;
               shutdown(sockfd, SHUT_WR); //close for writing
               FD_CLR(fileno(stdin), &rset);
               continue;
            }
            if (writen(sockfd, recvdata, strlen(recvdata)) != strlen(recvdata)) { //is not sending the null termination
               perror("write");
               exit(1);
            }

            memset(recvdata, 0, MAXDATASIZE); //clear the buffer
            */
         }



         if (FD_ISSET(sockfd, &rset)) {
            if ((Read(sockfd, arrivedData, SIZE)) > 0) {
               if (arrivedData[0] == 'i') {
                  printf("Bem vindo no jogo da forca!\n-----\n");
                  printf("1)Iniciar partidas simples\n");
                  printf("2)Ser carrasco ao iniciar partida\n");
                  printf("3)Jogar no modo multiplayer\n");                  
               }
               if (gameMode == '1') { 
                  switch (arrivedData[0]) {
                     case '!':
                        printf("\nA partida de jogo da forca comecou!\n");
                        printf("-----\n");
                        printf("Voce possui %d vidas\n", arrivedData[1]);
                        printf("A palavra possui %d caracteres\n", arrivedData[2]);

                        strcpy(willSendData, "1ini"); //send the confirmation
                        writen(sockfd, willSendData, strlen(willSendData));
                        break;
                     case '#':
                        printf("A palavra nao tem nehuma letra '%c'\n", arrivedData[1]);
                        printf("Você agora possui %d vidas\n\n", arrivedData[2]);
                        break;
                     default:
                        printf("%s\n\n", arrivedData);
                        break;
                  }
               }
               if (gameMode == '2') {
                  switch (arrivedData[0]) {
                     case '!':
                        printf("%d\n", arrivedData[1]); //debug
                        if ((int)arrivedData[1] == 0) printf("Nao se encontrou multiplayer(s). Continuar procurando? (s/n)\n");
                        else printf("Se encontrou %d multiplayer(s). Continuar procurando? (s/n)\n", arrivedData[1]);
                        break;
                     case '#':
                        break;
                     default:

                        break;
                  }
               }
               if (gameMode == '3') {
                  switch (arrivedData[0]) {
                     case '!':
                        printf("Se encontrou %d carrasco(s)\n", arrivedData[1]);
                        break;
                     case '#':
                        break;
                     default:

                        break;
                  }
               }
            }
            else { //if the server close the connection
               //not sure if it necessary close the stdin (FD_CLR(STDIN_FILENO, &rset))
               running = 0;
               close(sockfd);
               //shutdown(sockfd, SHUT_RD); //close for reading
               FD_CLR(sockfd, &rset);
               continue;
            }
         }
      }
   }



   exit(0);
}
