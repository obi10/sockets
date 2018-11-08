CC=gcc
FLAGS = -Wall

all : cliente servidor

cliente : cliente.c
	$(CC) $(FLAGS) -o cliente cliente.c socket_helper.c
servidor : servidor.c
	$(CC) $(FLAGS) -o servidor servidor.c socket_helper.c

clean :
	@rm cliente servidor 
