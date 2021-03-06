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
#define thread_count 1001
#define STR_LEN 50
#define READ 0
#define WRITE 1

typedef struct {
	int arrayID;
	int RW;
} message_t;

typedef struct {
	int readers;
	int writer;
	pthread_cond_t readers_proceed;
	pthread_cond_t writer_proceed;
	int pending_writers;
	pthread_mutex_t read_write_lock;
} mylib_rwlock_t;

char **theArray;
mylib_rwlock_t *synch_threads;
int countfda;
int threadKill;
//long clientFileDescriptor;

void mylib_rwlock_init (mylib_rwlock_t *l) {
	l -> readers = l -> writer = l -> pending_writers = 0;
	pthread_mutex_init(&(l -> read_write_lock), NULL);
	pthread_cond_init(&(l -> readers_proceed), NULL);
	pthread_cond_init(&(l -> writer_proceed), NULL);
}

void mylib_rwlock_rlock(mylib_rwlock_t *l) {
	/* if there is a write lock or pending writers, perform
	condition wait, else increment count of readers and grant
	read lock */
	pthread_mutex_lock(&(l -> read_write_lock));
	while ((l -> pending_writers > 0) || (l -> writer > 0)) {
		pthread_cond_wait(&(l -> readers_proceed), &(l -> read_write_lock));
	}
	l -> readers ++;
	pthread_mutex_unlock(&(l -> read_write_lock));

}

void mylib_rwlock_wlock(mylib_rwlock_t *l) {
	/* if there are readers or writers, increment pending
	writers count and wait. On being woken, decrement pending
	writers count and increment writer count */
	pthread_mutex_lock(&(l -> read_write_lock));
	while ((l -> writer > 0) || (l -> readers > 0)) {
		l -> pending_writers ++;
		pthread_cond_wait(&(l -> writer_proceed), &(l -> read_write_lock));
		l -> pending_writers --;
	}
	l -> writer ++;
	pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_unlock(mylib_rwlock_t *l) {
	/* if there is a write lock then unlock, else if there
	are read locks, decrement count of read locks. If the count
	is 0 and there is a pending writer, let it through, else if
	there are pending readers, let them all go through */
	pthread_mutex_lock(&(l -> read_write_lock));
	if (l -> writer > 0) {
		l -> writer = 0;
	}
	else if (l -> readers > 0) {
		l -> readers --;
	}
	pthread_mutex_unlock(&(l -> read_write_lock));
	if ((l -> readers == 0) && (l -> pending_writers > 0)) {
		pthread_cond_signal(&(l -> writer_proceed));
	}
	else if (l -> readers > 0) {
		pthread_cond_broadcast(&(l -> readers_proceed));
	}
}

void *clientThreadHandler(void *args)
{
	printf("Starting clientthreadhandler:::\n");
	long private_clientFileDescriptor = (long)args;
	//mylib_rwlock_unlock(synch_threads);	
	message_t draft;
	char str[STR_LEN];

	read(private_clientFileDescriptor, &draft, sizeof(draft));
	printf("--ClientFD = %ld ---- ArrayID = %d ----RW = %d-------\n", private_clientFileDescriptor, draft.arrayID, draft.RW);
	//mylib_rwlock_wlock(synch_threads);
	if (draft.RW == WRITE) {
		mylib_rwlock_wlock(synch_threads);
		snprintf(theArray[draft.arrayID], STR_LEN, "String %i has been modified by a write request\n", draft.arrayID);
		printf("Printing into: %s\n", theArray[draft.arrayID]);
		snprintf(str, STR_LEN, "%s", theArray[draft.arrayID]);
		//mylib_rwlock_unlock(synch_threads);
		//printf("sending to client:%s\n", str);
		//theArray[draft.arrayID] = str;
	}
	else {
		mylib_rwlock_rlock(synch_threads);

		snprintf(str, STR_LEN, "%s", theArray[draft.arrayID]);
		//mylib_rwlock_unlock(synch_threads);
	}
	//printf("sending to client:%s\n", str);
	//mylib_rwlock_unlock(synch_threads);
	write(private_clientFileDescriptor, str, STR_LEN);
	mylib_rwlock_unlock(synch_threads);
	printf("Ending threadhandler...\n");
	// Try to count the number of threads that finish (need mutex lock for countfda)
	// Have problems with deadlock?
	// countfda++;
	close(private_clientFileDescriptor);
	//pthread_exit(&threadKill);
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("%s\n", "Invalid number of arguements, please enter 2 inputs");
		exit(0);
	}

	int array_size = atoi(argv[2]);

	struct sockaddr_in sock_var;
	int serverFileDescriptor = socket(AF_INET,SOCK_STREAM,0);
	//long clientFileDescriptor;
	int i, j;
	pthread_t thread_handles[1000];
	theArray = malloc(array_size * sizeof(char *));
	for (j = 0; j < array_size; j++) {
		theArray[j] = malloc(50 * sizeof(char));
		snprintf(theArray[j], STR_LEN, "String %i: the initial value\n", j);
	}

	for (j = 0; j < array_size; j++) {
		printf("%s\n", theArray[j]);
	}
	//pthread_t *thread_handles;
	//thread_handles = malloc(1000 * sizeof(pthread_t));
	//pthread_t thread_handles[1000];

	sock_var.sin_addr.s_addr = inet_addr("127.0.0.1");
	sock_var.sin_port = atoi(argv[1]);
	sock_var.sin_family = AF_INET;

	synch_threads = malloc(sizeof(mylib_rwlock_t));
	mylib_rwlock_init(synch_threads);

	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
	{
		printf("socket has been created\n");
		listen(serverFileDescriptor,2000);
		while(1)        //loop infinity
		{
			for(i=0; i < thread_count; i++)      //can support 1000 clients at a time
			{
				printf("Loop: %i\n", i);
				//mylib_rwlock_wlock(synch_threads);
				long clientFileDescriptor= accept(serverFileDescriptor,NULL,NULL);
				//mylib_rwlock_unlock(synch_threads);
				
				printf("Connected to client %ld\n",clientFileDescriptor);
				pthread_create((void *) &thread_handles[i], NULL, clientThreadHandler, (void *) clientFileDescriptor);
				
				//mylib_rwlock_unlock(synch_threads);	
			}
			printf("INBETWEEN --------------------------------------------\n");
			int p;
			for (p = 0; p < thread_count; p++) {
				printf("WE CHECKING BEFORE\n");
				pthread_join(thread_handles[p], NULL);
				printf("WE CHECKING AFTER\n");
			}
			//memset(thread_handles, 0, sizeof(thread_handles));
			//close(serverFileDescriptor);
			//listen(serverFileDescriptor, 2000);
			//free(synch_threads);
			//break;
			printf("WE FINISHED-----------------------------------------\n");
		}
		close(serverFileDescriptor);
	}
	else{
		printf("socket creation failed\n");
	}

	//free(thread_handles);
	return 0;
}
