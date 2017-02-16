#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include"timer.h"

#define READ 0
#define WRITE 1
#define thread_count 1000
#define STR_LEN 50

int *seed;
int array_size;
struct sockaddr_in sock_var;

typedef struct {
	int arrayID;
	int RW;
} message_t;

void *Operate(void* rank) {
	long my_rank = (long) rank;

	int clientFileDescriptor;
	clientFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	if(connect(clientFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0) {
		char server_msg[STR_LEN];

		// Find a random position in theArray for read or write
		int pos = rand_r(&seed[my_rank]) % array_size;
		int randNum = rand_r(&seed[my_rank]) % 20;	// write with 5% probability

		message_t draft;
		draft.arrayID = pos;
		// 5% are write operations, others are reads
		if (randNum >= 19) {
			// Replace sprintf with a write function to server.c
			draft.RW = WRITE;
		}
		else {
			// Perform read operation
			draft.RW = READ;

		}
		write(clientFileDescriptor, &draft, sizeof(draft));
		read(clientFileDescriptor, server_msg, STR_LEN);

		//printf("Client %ld: Recieved : %s\n", my_rank, server_msg); // return the value read or written

		close(clientFileDescriptor);
	}
	else{
		printf("socket creation failed for %i\n", rank);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("%s\n", "Invalid number of arguements, please enter 2 inputs");
		exit(0);
	}

	long       thread;  /* Use long in case of a 64-bit system */
	pthread_t thread_handles[thread_count];
	int i;
	double start, finish, elapsed;

	array_size = atoi(argv[2]);

	/* Intializes random number generators */
	seed = malloc(thread_count*sizeof(int));
	for (i = 0; i < thread_count; i++)
		seed[i] = i;

	int clientFileDescriptor;
	clientFileDescriptor=socket(AF_INET,SOCK_STREAM,0);

	sock_var.sin_addr.s_addr = inet_addr("127.0.0.1");
	sock_var.sin_port = atoi(argv[1]);
	sock_var.sin_family = AF_INET;

	GET_TIME(start);
	for (thread = 0; thread < thread_count; thread++) {
		pthread_create(&thread_handles[thread], NULL, Operate, (void*) thread);
	}

	for (i = 0; i < thread_count; i++) {
		pthread_join(thread_handles[i], NULL);
	}
	GET_TIME(finish);
	elapsed = finish - start;
 	//printf("The elapsed time is %e seconds\n", elapsed);
	printf("%e\n", elapsed);	

	close(clientFileDescriptor);
	return 0;
}
