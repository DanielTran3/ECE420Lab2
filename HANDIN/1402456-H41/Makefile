all: server.c client.c server_mutex.c
	gcc server.c -o server -lpthread
	gcc server_mutex.c -o server_mutex -lpthread
	gcc client.c -o client -lpthread

server: server.c
	gcc server.c -o server -lpthread
client: client.c
	gcc client.c -o client -lpthread
server_mutex: server_mutex.c
	gcc server_mutex.c -o server_mutex -lpthread
