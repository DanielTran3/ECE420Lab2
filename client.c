#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

#define READ = 0;
#define WRITE = 1;

typedef struct message {
	int arrayID;
	int RW;
	char * updateArray;
} message_t

void *Operate(void* rank) {
	long my_rank = (long) rank;

	// Find a random position in theArray for read or write
	int pos = rand_r(&seed[my_rank]) % NUM_STR;
	int randNum = rand_r(&seed[my_rank]) % 20;	// write with 5% probability

	pthread_mutex_lock(&mutex);

	// 5% are write operations, others are reads
	if (randNum >= 19) {
		// Replace sprintf with a write function to server.c
		sprintf(theArray[pos], "theArray[%i] modified by thread %ld", pos, my_rank);
	}

	else {
		// Perform read operation
	}

	printf("Thread %ld: randNum = %i\n", my_rank, randNum);
	printf("%s\n\n", theArray[pos]); // return the value read or written
	pthread_mutex_unlock(&mutex);

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

	/* Get number of threads from command line */
	thread_count = strtol(argv[1], NULL, 10);

	/* Intializes random number generators */
	seed = malloc(thread_count*sizeof(int));
	for (i = 0; i < thread_count; i++)
		seed[i] = i;

	struct sockaddr_in sock_var;
	int clientFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	char str_clnt[20],str_ser[20];

	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=argv[1];
	sock_var.sin_family=AF_INET;

	if(connect(clientFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("Connected to server %dn",clientFileDescriptor);
		printf("nEnter Srting to send");
		scanf("%s",str_clnt);
		write(clientFileDescriptor,str_clnt,20);
		read(clientFileDescriptor,str_ser,20);
		printf("String from Server: %s",str_ser);
		close(clientFileDescriptor);
	}
	else{
		printf("socket creation failed");
	}
	return 0;
}
