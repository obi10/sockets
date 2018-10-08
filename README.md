# Exercícios - MC833 2S2018

Repositório que contém os exercícios 4 e 5 do curso MC833
IC-UNICAMP

## Estrutura de diretórios

* `exercicio4`
    * Aqui estão os arquivos: servidor.c, cliente.c e Makefile do exercício __Monitorando a rede__
    * URL do exercicio (<http://www.lrc.ic.unicamp.br/mc833/exercicios/servidor_concorrente_2018/index.html>)
* `exercicio5`
    * Aqui estão os arquivos: servidor.c, cliente.c e Makefile do exercício __Backlog e Processos zumbis__
    * URL do exercicio (<http://www.lrc.ic.unicamp.br/mc833/exercicios/backlog_e_zumbis_2018/index.html>)

### Steps

1. clone the repository `git clone https://github.com/obi10/sockets.git`
2. go to the folder named exercicio4 `cd exercicio4`
3. compile the programs `make`
4. In one terminal execute the server program (the program will use the port 8000 for the communication) `./servidor 8000`
5. In another terminal execute the client program (must be specified the ip of the server) `./cliente 127.0.0.1`
6. In a third terminal execute the client program (must be specified the ip of the server) `./cliente 127.0.0.1`
7. Sen a command to a one client trought the server terminal  `1!pwd`
