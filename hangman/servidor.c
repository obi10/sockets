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


void doit(int connfd, struct sockaddr_in clientaddr, char words[MAXNUMWORDS][MAXWORDSIZE + 1], int *clientsMP, int *clientsHM);

int main (int argc, char **argv) {
   int    listenfd,              
          connfd,            
          port,
          backlog;
   char *ip;                  
   struct sockaddr_in servaddr;  
   char   error[MAXDATASIZE + 1];     
   char words[MAXNUMWORDS][MAXWORDSIZE + 1];
   FILE *f;

   //arrays of sockets (childs processes must share memory)
   int *clientsMP;
   clientsMP = mmap(NULL, MAXNUMMP*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   int *clientsHM;
   clientsHM = mmap(NULL, MAXNUMHM*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

   if (argc != 4) {
      strcpy(error,"usage: ");
      strcat(error,argv[0]);
      strcat(error," <ip_address, port, backlog>");
      perror(error);
      exit(1);
   }

   f = fopen("dicionario.txt", "r");
   if (f == NULL) {
      perror("fopen");
      exit(1);
   }

   
   if (chargeFileMatrix(f, words) == 0) {
      perror("charge file");
      exit(1);
   }

   for (int i = 0; i < MAXNUMMP; ++i) clientsMP[i] = -1; //-1 indicates available entry
   for (int i = 0; i < MAXNUMHM; ++i) clientsHM[i] = -1;

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
         
         doit(connfd, clientaddr, words, clientsMP, clientsHM);

         Close(connfd);

         exit(0);
      }

      Close(connfd);
   }
   
   return(0);
}

void doit(int connfd, struct sockaddr_in clientaddr, char words[MAXNUMWORDS][MAXWORDSIZE + 1], int *clientsMP, int *clientsHM) {
   int numVidas = 6;
   int running = 1;

   int random_number = 1; //just for testing the word is "linux mint"

   char recvline[SIZE + 1];
   char sendline[SIZE + 1];

   char word[MAXWORDSIZE + 1]; memset(word, '\0', sizeof word);

   while (running) {

      memset(recvline, '\0', sizeof recvline);
      memset(sendline, '\0', sizeof sendline);


      //inform the beginning of the game
      sendline[0] = 'i';
      writen(connfd, sendline, strlen(sendline));
      sendline[0] = '\0';

      //wait the selection of the user
      Read(connfd, recvline, SIZE);

      if (strcmp(recvline, "#1") == 0) { //individual mode
         memset(recvline, '\0', sizeof recvline);

         //send the number of lifes and the size of the word
         strcpy(word, words[random_number]);
         sendline[0] = '!';
         sendline[1] = numVidas;
         sendline[2] = strlen(word);
         for (int i = 0; i < strlen(word); ++i) if (word[i] == ' ') sendline[2]--;
         writen(connfd, sendline, strlen(sendline));
         memset(sendline, '\0', sizeof sendline);

         //wait until the confirmation of the client
         Read(connfd, recvline, SIZE);
         if (strcmp(recvline, "1ini") != 0) break; //go out of the while(true)
         memset(recvline, '\0', sizeof recvline);

         //send the model of the word
         for (int i = 0; i < strlen(word); ++i)
         {
            sendline[2*i] = word[i];
            sendline[2*i+1] = ' ';
         }
         for (int i = 0; i < strlen(sendline); ++i)
         {
            if (sendline[i] != ' ' && sendline != '\0') sendline[i] = '_';
         }
         writen(connfd, sendline, strlen(sendline)); //the representation of the word is send


         while (numVidas > 0) {

            Read(connfd, recvline, SIZE);
            int count = 0;
            for (int i = 0; i < strlen(word); ++i)
            {
               if (recvline[0] == word[i]) {
                  sendline[2*i] = word[i];
                  word[i] = '+';
                  count ++;
               }
            }

            if (count == 0) {
               char newBuffer[SIZE + 1];
               memset(newBuffer, '\0', sizeof newBuffer);
               newBuffer[0] = '#';
               numVidas--;
               newBuffer[1] = recvline[0];
               newBuffer[2] = numVidas;
               writen(connfd, newBuffer, strlen(newBuffer));
            }
            else {
               writen(connfd, sendline, strlen(sendline));
            }
            recvline[0] = '\0';

            for (int i = 0; i < strlen(sendline); ++i)
            {
               //if (word[i] != ' ' && word[i] != '_') cmp = 0; 
            }

         }

         running = 0;
      }

      if (strcmp(recvline, "#2") == 0) { //client like a hangman

         for (int i = 0; i < MAXNUMHM; ++i)
            if (clientsHM[i] < 0) {
               clientsHM[i] = connfd;
               break;
            }

         do {
            memset(recvline, '\0', sizeof recvline);

            int num = 0; 
            for (int i = 0; i < MAXNUMMP; ++i) if (clientsMP[i] > 0) num ++;

            sendline[0] = '!';
            sendline[1] = num;

            printf("%s\n", sendline); //debug
            writen(connfd, sendline, strlen(sendline));
            memset(sendline, '\0', sizeof sendline);

            Read(connfd, recvline, SIZE);
         } while (strcmp(recvline, "2keep") == 0);

         if (strcmp(recvline, "2reset") == 0) continue;

         if (strcmp(recvline, "2ini") == 0) {
            //logica
         }

         running = 0;
      }

      if (strcmp(recvline, "#3") == 0) { //client like a multiplayer
         memset(recvline, '\0', sizeof recvline);

         for (int i = 0; i < MAXNUMMP; ++i)
            if (clientsMP[i] < 0) {
               clientsMP[i] = connfd;
               break;
            }

         int num = 0;
         for (int i = 0; i < MAXNUMHM; ++i) if (clientsHM[i] == 0) num++;

         if (num > 0) {
            sendline[0] = '3';
            sendline[1] = '!';
            writen(connfd, sendline, strlen(sendline));
         }

         running = 0;
      }

      
   }

   /*
   while(read(connfd, recvline, MAXDATASIZE) > 0) {
      if (writen(connfd, recvline, strlen(recvline)) != strlen(recvline)) {
         perror("write");
         exit(1);
      }
      memset(recvline, 0, MAXDATASIZE); //important
   }
   */
}
