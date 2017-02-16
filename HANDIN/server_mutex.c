#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>

#define NUM_STR 1000
#define thread_count 1000
#define STR_LEN 50
#define READ 0
#define WRITE 1

typedef struct {
	int arrayID;
	int RW;
} message_t;

char **theArray;
int     count = 0;
pthread_mutex_t mutex;

	
void *clientThreadHandler(void *args)
{
	long private_clientFileDescriptor = (long)args;
	message_t draft;
	char str[STR_LEN];

	read(private_clientFileDescriptor, &draft, sizeof(draft));
	if (draft.RW == WRITE) {
		pthread_mutex_lock(&mutex);
		snprintf(theArray[draft.arrayID], STR_LEN, "String %i has been modified by a write request\n", draft.arrayID);
		//printf("Recieved Write Request from Client: %ld\n", private_clientFileDescriptor);
	}
	else {
		pthread_mutex_lock(&mutex);
		//printf("Recieved Read Request from Client: %ld\n", private_clientFileDescriptor);
	}
	snprintf(str, STR_LEN, "%s", theArray[draft.arrayID]);
	pthread_mutex_unlock(&mutex);
	write(private_clientFileDescriptor, str, STR_LEN);
	close(private_clientFileDescriptor);
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("%s\n", "Invalid number of arguements, please enter 2 inputs");
		exit(0);
	}

	int array_size = atoi(argv[2]);

	pthread_mutex_init(&mutex, NULL);
	struct sockaddr_in sock_var;
	int serverFileDescriptor = socket(AF_INET,SOCK_STREAM,0);
	int i, j, p;
	pthread_t thread_handles[1000];
	theArray = malloc(array_size * sizeof(char *));
	for (j = 0; j < array_size; j++) {
		theArray[j] = malloc(50 * sizeof(char));
		snprintf(theArray[j], STR_LEN, "String %i: the initial value\n", j);
	}

	sock_var.sin_addr.s_addr = inet_addr("127.0.0.1");
	sock_var.sin_port = atoi(argv[1]);
	sock_var.sin_family = AF_INET;

	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("socket has been created\n");
		listen(serverFileDescriptor,2000);
		while(1)        //loop infinity
		{
			for(i=0; i < thread_count; i++)      //can support 1000 clients at a time
			{
				long clientFileDescriptor= accept(serverFileDescriptor,NULL,NULL);
				pthread_create((void *) &thread_handles[i], NULL, clientThreadHandler, (void *) clientFileDescriptor);
			}
			for (p = 0; p < thread_count; p++) {
				pthread_join(thread_handles[p], NULL);
			}
		}
		close(serverFileDescriptor);
	}
	else{
		printf("socket creation failed\n");
	}
	return 0;
}
