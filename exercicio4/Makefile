CC=gcc
FLAGS = -Wall

all : cliente servidor

cliente : cliente.c
	$(CC) $(FLAGS) -o cliente cliente.c wrap.c
servidor : servidor.c
	$(CC) $(FLAGS) -o servidor servidor.c wrap.c

clean :
	@rm cliente servidor 