all: server.c client.c
	gcc server.c -o server -lpthread
	gcc client.c -o client -lpthread

server: server.c
	gcc server.c -o server -lpthread
client: client.c
	gcc client.c -o client -lpthread
