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

   char recvline[SIZE + 1];
   char sendline[MAXWORDSIZE + 1];

   char word[MAXWORDSIZE + 1]; memset(word, '\0', sizeof word);

   while (true) {

      memset(recvline, '\0', sizeof recvline);
      memset(sendline, '\0', sizeof sendline);


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
      for (int i = 0; i < strlen(sendline); ++i) if (word[i] == ' ') sendline[2]--;

      printf("%ld\n", strlen(sendline));

      writen(connfd, sendline, strlen(sendline));
      memset(sendline, '\0', sizeof sendline);

      strcpy(sendline, word);
      printf("%s\n", sendline);
      for (int i = 0; i < strlen(sendline); ++i)
      {
         if (sendline[i] != ' ' && sendline != '\0') sendline[i] = '_';
      }
      printf("%s\n", sendline);
      printf("%ld\n", strlen(sendline));
      sleep(1); //deve ser eleminado
      writen(connfd, sendline, strlen(sendline)); //the representation of the word is send


      while (numVidas > 0) {

         Read(connfd, recvline, SIZE);
         int count = 0;
         for (int i = 0; i < strlen(word); ++i)
         {
            if (recvline[0] == word[i]) {
               sendline[i] = word[i];
               word[i] = '+';
               count ++;
            }
         }

         if (count == 0) {
            char newBuffer[SIZE + 1];
            memset(newBuffer, '\0', sizeof newBuffer);
            newBuffer[0] = '!';
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

      //enviar fim da partida e perguntar se deseja jogar outra vez



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
