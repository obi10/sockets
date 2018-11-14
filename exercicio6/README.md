### Steps (socket program - localhost)
### Multiplexing exercise

1. clone the repository `git clone https://github.com/obi10/sockets.git`
2. go to the folder named exercicio6 `cd exercicio6`
3. compile the programs `make`
4. In one terminal execute the server program `./servidor 127.0.0.1 8000 4`
5. In another terminal execute the client program `time ./cliente 5 127.0.0.1 8000 < texto1.txt > texto2.txt`
6. In the client terminal appears the total time that the echo task takes
7. Execute the next command to see if the differences between the text1 and the text2 `diff texto1.txt texto2.txt`