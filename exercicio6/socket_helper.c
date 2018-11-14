#include "socket_helper.h"


int Socket(int domain, int type, int protocol) {
   int sockfd;

   if ((sockfd = socket(domain, type, protocol)) < 0) {
      perror("socket error");
      exit(1);
   }

   return sockfd;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
   int result;
   
   if ((result = bind(sockfd, addr, addrlen)) == -1) {
      perror("bind");
      exit(1);
   }

   return result;
}

int Listen(int sockfd, int backlog) {
   int result;
   
   if ((result = listen(sockfd, backlog)) == -1) {
      perror("listen");
      exit(1);
   }

   return result;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
   int connfd;

   if ((connfd = accept(sockfd, addr, addrlen)) < 0) {
      if (errno == EINTR) return connfd;
      else {
         perror("accept");
         exit(1);
      }
   }

   return connfd;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
   int result;

   if ((result = connect(sockfd, addr, addrlen)) < 0) {
      perror("connect error");
      exit(1);
   }

   return result;
}

int Close(int fd) {
   int result;
   
   if ((result = close(fd)) < 0) {
      perror("connect error");
   }

   return result;
}

int Getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
   int result = getsockname(sockfd, addr, addrlen);
   
   if (result < 0) {
      perror("getsockname() failed");
   }

   return result;
}

struct sockaddr_in ServerSockaddrIn(int family, const char *ip, unsigned short port) {
   struct sockaddr_in addr;

   bzero(&addr, sizeof(addr));         
   addr.sin_family      = family;      
   addr.sin_addr.s_addr = inet_addr(ip);   
   addr.sin_port        = htons(port); 

   return addr;
}

struct sockaddr_in ClientSockaddrIn(int family, const char *ip, unsigned short port) {
   struct sockaddr_in addr;

   bzero(&addr, sizeof(addr));         
   addr.sin_family      = family; 
   addr.sin_port        = htons(port); 

   if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
      perror("inet_pton error");
      exit(1);
   }

   return addr;
}

Sigfunc * Signal (int signo, Sigfunc *func) {
   struct sigaction act, oact;
   act.sa_handler = func;
   sigemptyset (&act.sa_mask); /* Outros sinais não são bloqueados*/
   act.sa_flags = 0;
   if (signo == SIGALRM) { /* Para reiniciar chamadas interrompidas */
   #ifdef SA_INTERRUPT
   act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */
   #endif
   } else {
   #ifdef SA_RESTART
   act.sa_flags |= SA_RESTART; /* SVR4, 4.4BSD */
   #endif
   }
   if (sigaction (signo, &act, &oact) < 0)
   return (SIG_ERR);
   return (oact.sa_handler);
}



void sig_chld(int signo) {
   pid_t pid;
   int stat;
   while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
      printf("child %d terminated\n", pid);
   return;
}


ssize_t
my_read(int fd, char *ptr, char *read_buf)
{
   static int  read_cnt;
   static char *read_ptr;
   
   if (read_cnt <= 0) {
again:
      if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
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

   ptr = vptr;
   for (n = 1; n < maxlen; n++) {
      if ( (rc = my_read(fd, &c, read_buf)) == 1) {
         *ptr++ = c;
         if (c == '\n')
            break;   /* newline is stored, like fgets() */
      } else if (rc == 0) {
         *ptr = 0;
         return(n - 1); /* EOF, n - 1 bytes were read */
      } else
         return(-1);    /* error, errno set by read() */
   }

   *ptr = 0;   /* null terminate like fgets() */
   return(n);
}

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
   ssize_t n;
   static char read_buf[MAXDATASIZE];

   if ( (n = readline(fd, ptr, maxlen, read_buf)) < 0)
      perror("readline error");
   return(n);
}



ssize_t                 /* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
   size_t      nleft;
   ssize_t     nwritten;
   const char  *ptr;

   ptr = vptr;
   nleft = n;
   while (nleft > 0) {
      if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
         if (nwritten < 0 && errno == EINTR)
            nwritten = 0;     /* and call write() again */
         else
            return(-1);       /* error */
      }

      nleft -= nwritten;
      ptr   += nwritten;
   }
   return(n);
}
/* end writen */
