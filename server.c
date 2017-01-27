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
char theArray[NUM_STR][STR_LEN];

void *ServerEcho(void *args)
{
	int clientFileDescriptor=(int)args;
	char str[20];

	read(clientFileDescriptor,str,20);
	printf("nreading from client:%s",str);
	write(clientFileDescriptor,str,20);
	printf("nechoing back to client");
	close(clientFileDescriptor);
}

int main(int argc, char *argv[])
{
	int array_size = (int) argv[2];

	if (argc != 3) {
		printf("%s\n", "Invalid number of arguements, please enter 2 inputs");
		exit(0);
	}

	struct sockaddr_in sock_var;
	int serverFileDescriptor = socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;
	int i;
	pthread_t t[1000];

	sock_var.sin_addr.s_addr = inet_addr("127.0.0.1");
	sock_var.sin_port = (int) argv[1];
	sock_var.sin_family = AF_INET;
	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("nsocket has been created");
		listen(serverFileDescriptor,2000);
		while(1)        //loop infinity
		{
			for(i=0; i < 1000; i++)      //can support 1000 clients at a time
			{
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				printf("nConnected to client %dn",clientFileDescriptor);
				pthread_create(&t[i],NULL,ServerEcho,(void *)clientFileDescriptor);
			}
		}
		close(serverFileDescriptor);
	}
	else{
		printf("nsocket creation failed");
	}
	return 0;
}
