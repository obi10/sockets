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

Sigfunc * Signal (int signo, Sigfunc *func)
{
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

void Send(int socket, char *buffer, size_t length, int flags){
   if (send(socket, buffer, length, flags) == -1){
      perror("send");
      exit(1);
   }
}

void Read(int socket, char *buffer, size_t length){
   if (read(socket, buffer, length) == -1){
      perror("read");
      exit(1);
   }
}