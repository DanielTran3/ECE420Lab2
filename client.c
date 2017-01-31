#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include"timer.h"

#define READ 0
#define WRITE 1
#define thread_count 3
#define STR_LEN 50

int *seed;
int clientFileDescriptor;
long array_size;

typedef struct {
	int arrayID;
	int RW;
} message_t;

void *Operate(void* rank) {
	long my_rank = (long) rank;
	char server_msg[50];

	// Find a random position in theArray for read or write
	int pos = rand_r(&seed[my_rank]) % array_size;
	int randNum = rand_r(&seed[my_rank]) % 20;	// write with 5% probability

	// struct message_t *draft = malloc(sizeof(struct message));
	message_t draft;

	draft.arrayID = pos;
	// 5% are write operations, others are reads
	if (randNum >= 19) {
		// Replace sprintf with a write function to server.c
		draft.RW = WRITE;
		write(clientFileDescriptor, &draft, sizeof(draft));
		read(clientFileDescriptor, server_msg, STR_LEN);
	}

	else {
		// Perform read operation
		draft.RW = READ;
		read(clientFileDescriptor, server_msg, STR_LEN);
	}

	printf("Thread %ld: randNum = %i\n", my_rank, randNum);
	printf("%s\n\n", server_msg); // return the value read or written

	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("%s\n", "Invalid number of arguements, please enter 2 inputs");
		exit(0);
	}

	long       thread;  /* Use long in case of a 64-bit system */
	pthread_t* thread_handles;
	int i;
	double start, finish, elapsed;
	long ipAddress = (long) argv[1];
	printf("%ld\n", ipAddress);
	array_size = (long) argv[2];

	/* Intializes random number generators */
	seed = malloc(thread_count*sizeof(int));
	for (i = 0; i < thread_count; i++)
		seed[i] = i;

	struct sockaddr_in sock_var;
	clientFileDescriptor=socket(AF_INET,SOCK_STREAM,0);

	sock_var.sin_addr.s_addr = inet_addr("127.0.0.1");
	sock_var.sin_port = (int) ipAddress;
	sock_var.sin_family = AF_INET;

	if(connect((int) clientFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("Connected to server %dn",clientFileDescriptor);

		GET_TIME(start);
		for (thread = 0; thread < thread_count; thread++)
			pthread_create(&thread_handles[thread], NULL, Operate, (void*) thread);

		for (thread = 0; thread < thread_count; thread++)
			pthread_join(thread_handles[thread], NULL);
		GET_TIME(finish);
		elapsed = finish - start;
	 	printf("The elapsed time is %e seconds\n", elapsed);

		close(clientFileDescriptor);
	}
	else{
		printf("socket creation failed");
	}
	return 0;
}
