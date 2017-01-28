#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define NUM_STR 1000
#define STR_LEN 50
#define READ 0
#define WRITE 1
char theArray[NUM_STR][STR_LEN];

typedef struct {
	int arrayID;
	int RW;
} message_t;

void *clientThreadHandler(void *args)
{
	long clientFileDescriptor = (long)args;
	message_t draft;
	char str[STR_LEN];

	read(clientFileDescriptor, &draft, sizeof(draft));
	if (draft.RW == WRITE) {
		snprintf(str, STR_LEN, "String %i has been modified by a write request\n", draft.arrayID);
		*theArray[draft.arrayID] = *str;
	}
	else {
		*str = *theArray[draft.arrayID];
	}
	printf("\nsending to client:%s\n",str);
	write(clientFileDescriptor,str,NUM_STR);
	close(clientFileDescriptor);
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("%s\n", "Invalid number of arguements, please enter 2 inputs");
		exit(0);
	}

	int array_size = (long) argv[2];

	struct sockaddr_in sock_var;
	int serverFileDescriptor = socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;
	int i;
	pthread_t t[1000];

	sock_var.sin_addr.s_addr = inet_addr("127.0.0.1");
	sock_var.sin_port = (long) argv[1];
	sock_var.sin_family = AF_INET;
	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("socket has been created\n");
		listen(serverFileDescriptor,2000);
		while(1)        //loop infinity
		{
			for(i=0; i < 1000; i++)      //can support 1000 clients at a time
			{
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				printf("nConnected to client %d\n",clientFileDescriptor);
				pthread_create(&t[i],NULL,clientThreadHandler,(void *)clientFileDescriptor);
			}
		}
		close(serverFileDescriptor);
	}
	else{
		printf("socket creation failed\n");
	}
	return 0;
}
