/*
 * This example is for a producer-consumer problem
 * using two semaphores
 * buf_full semaphore is for buffer full and signals consumer
 * buf_empy semaphore is for buffer empty and signals producer
 */
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#define BUF_LEN	5
#define NITERS	5
#define NCP	5

typedef struct shared_t {
	int buffer[BUF_LEN];
	pthread_mutex_t my_mutex;
	sem_t buf_full;
	sem_t buf_empty;
	int tnum[NCP];
} shared_t;

shared_t shared_data;

void * produce(void *iter) {
	int item;
	int i = 0;

	item = *(int *)iter;
	for(i=0;i<=NITERS;i++) {
		sem_wait(&shared_data.buf_empty);
		pthread_mutex_lock(&shared_data.my_mutex);
		shared_data.buffer[i] = i;
		printf("P[%d] buffer:%d\n", item, shared_data.buffer[i]);
		pthread_mutex_unlock(&shared_data.my_mutex);
		sem_post(&shared_data.buf_full);
	}
	return(NULL);
}

void * consume(void *iter) {
	int item;
	int i;

	item = *(int *)iter;
	for(i=NITERS;i>=0;i--) {
		sem_wait(&shared_data.buf_full);
		pthread_mutex_lock(&shared_data.my_mutex);
		shared_data.buffer[i] = i;
		printf("C[%d] buffer:%d\n", item, shared_data.buffer[i]);
		pthread_mutex_unlock(&shared_data.my_mutex);
		sem_post(&shared_data.buf_empty);
	}
	return(NULL);
}

int main() {
	pthread_t pro_tid, cons_tid;
	int i;
	int j;

	sem_init(&shared_data.buf_full, 0, 1);
	sem_init(&shared_data.buf_empty, 0, BUF_LEN);
	pthread_mutex_init(&shared_data.my_mutex, NULL);

	for(i=0;i<NCP;i++) {
		shared_data.tnum[i]=i;
		if(pthread_create(&pro_tid, NULL, produce, (void *)&(shared_data.tnum[i])) != 0) {
			printf("Failed to create producer thread\n");
			return -1;
		}
	}
	for(j=0;j<NCP;j++) {
		shared_data.tnum[j]=j;
		if(pthread_create(&cons_tid, NULL, consume, (void *)&(shared_data.tnum[j])) != 0) {
			printf("Failed to create consumer thread\n");
			return -1;
		}
	}
	pthread_join(pro_tid, NULL);
	pthread_join(cons_tid, NULL);
	pthread_exit(NULL);	
}
