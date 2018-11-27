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


void doit(int connfd, struct sockaddr_in clientaddr, char words[MAXNUMWORDS][MAXWORDSIZE + 1]);

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
         
         doit(connfd, clientaddr, words);

         Close(connfd);

         exit(0);
      }

      Close(connfd);
   }
   
   return(0);
}

void doit(int connfd, struct sockaddr_in clientaddr, char words[MAXNUMWORDS][MAXWORDSIZE + 1]) {
   int numVidas = 6;
   //int numVitorias = 0;
   //int game_running = 1;

   int random_number = 1; //just for testing the word is "linux mint"

   char word[MAXWORDSIZE + 1]; memset(word, '\0', sizeof word);

   while (true) {

      char recvline[SIZE + 1]; memset(recvline, '\0', sizeof recvline);
      char sendline[SIZE + 1]; memset(sendline, '\0', sizeof sendline);


      //inform the beginning of the game
      sendline[0] = '#'; //inicio do jogo
      writen(connfd, sendline, strlen(sendline));
      sendline[0] = '\0';

      //wait the confirmation of the user
      Read(connfd, recvline, SIZE);
      if (strcmp(recvline, "#ini") != 0) break;
      memset(recvline, '\0', sizeof recvline);

      //send the number of lifes and the size of the word
      strcpy(word, words[random_number]);
      sendline[0] = '%';
      sendline[1] = numVidas;
      sendline[2] = strlen(word);
      writen(connfd, sendline, strlen(sendline));
      memset(word, '\0', sizeof word);
      memset(sendline, '\0', sizeof sendline);

      //the character inserted by the user is 

      //debe seguir un while que abarque el for
      Read(connfd, recvline, SIZE);
      printf("%s\n", recvline);
      for (int i = 0; i < strlen(word); ++i)
      {
         if (recvline[0] == word[i]) printf("%s\n", "OK"); //nao funcionas, revisar ...
         recvline[0] = '\0';
      }




      break;
      
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
