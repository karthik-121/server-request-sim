all: server
server: server.c
	gcc -Wall -Werror -pthread -Wvla -fsanitize=address,undefined -std=c99 server.c -o server

clean: 
	rm -f server