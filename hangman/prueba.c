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

#include <stdlib.h>
#include <time.h> 

#include "basic.h"

#define MAXDATASIZE 4096
#define MAXWORDSIZE 5
#define MAXNUMWORDS 3

ssize_t
my_read(int fd, char *ptr, char *read_buf)
{
   static int  read_cnt;
   static char *read_ptr;
   
   if (read_cnt <= 0) {
again:
      if ( (read_cnt = read(fd, read_buf, MAXDATASIZE)) < 0) {
         if (errno == EINTR)
            goto again;
         return(-1);
      } else if (read_cnt == 0)
         return(0);
      read_ptr = read_buf;
   }

   read_cnt--;
   *ptr = *read_ptr++;
   return(1);
}

ssize_t
readline(int fd, void *vptr, size_t maxlen, char *read_buf)
{
   ssize_t  n, rc;
   char  c, *ptr;
   int new_line = 0;

   ptr = vptr;
   for (n = 0; n < maxlen + 1; n++) {
      if ( (rc = my_read(fd, &c, read_buf)) == 1) {
         if (c == '\n') {
            new_line = 1;
            break; //new line is not stored
         }
         *ptr++ = c;
      } else if (rc == 0) {
         *ptr = 0;
         return(n); /* EOF, n - 1 bytes were read */
      } else
         return(-1); /* error, errno set by read() */
      //printf("%zu", n);
   }
   if (new_line == 0) {
      ptr--;
      while ((rc = my_read(fd, &c, read_buf)) == 1) {
         //*ptr++ = 0;
         //printf("%s\n", (char *)vptr);
         if (c == '\n') break;
      }
      n--;
   }
   *ptr = 0;   /* null terminate like fgets() */
   return(n + 1);
}

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
   ssize_t n;
   static char read_buf[MAXDATASIZE + 1];

   if ((n = readline(fd, ptr, maxlen, read_buf)) < 0) {
      perror("readline error");
      exit(1);
   }
   return(n);
}

ssize_t
Read(int fd, void *buf, size_t nbyte)
{
   ssize_t read_cnt = 0;

   if (read_cnt <= 0) {
again:
      if ( (read_cnt = read(fd, buf, nbyte)) < 0) {
         if (errno == EINTR)
            goto again;
         //return(-1);
         perror("read error");
         exit(1);
      } else if (read_cnt == 0)
         return(0);
   }
   return(read_cnt);
}





int main (int argc, char **argv) {

   /*
	FILE *file;
	file  = fopen("dicionario.txt", "r");
	if (file == NULL) {
		perror("fopen");
		exit(1);
	}

	char buf[MAXWORDSIZE + 1]; memset(buf, '\0', sizeof buf);

	char words[MAXNUMWORDS][MAXWORDSIZE + 1];
	int n = 0;

	while (Readline(fileno(file), buf, MAXWORDSIZE) > 0) {
		strcpy(words[n], "jo");
		printf("%s%zu%zu\n", buf, strlen(buf), sizeof buf);
		printf("%s%zu%zu\n", words[n], strlen(words[n]), sizeof words[n]);
		n++;
		if (n == MAXNUMWORDS) break;
	}
   */

   /*
   struct timespec ts;
   if (timespec_get(&ts, TIME_UTC) == 0) {
      exit(1);
   }
   srandom(ts.tv_nsec ^ ts.tv_sec);
   */


   time_t t;

   int n = 5;

   //initializes random number generator
   srand((unsigned) time(&t));

   for (int i = 0; i < n; ++i)
   {
      printf("%d\n", rand() % 50);
   }


	exit(0);
}




