/*
 * This program solves the following problem:
 * - There is one consumer thread and multiple producer threads
 * All producer threads start but wait at a point for all the
 * threads to meet. At this point the last thread signals the 
 * consumer thread which would be waiting on a conditional variable
 *
 * Sample output:
 *
 * IN PRODUCER THREAD:[139796390364928] count:1
 * IN PRODUCER THREAD:[139796390364928], waiting on cond variable
 * IN PRODUCER THREAD:[139796398757632] count:2
 * IN PRODUCER THREAD:[139796398757632], waiting on cond variable
 * IN PRODUCER THREAD:[139796407150336] count:3
 * IN PRODUCER THREAD:[139796407150336], waiting on cond variable
 * IN PRODUCER THREAD:[139796415543040] count:4
 * IN PRODUCER THREAD:[139796415543040], waiting on cond variable
 * IN PRODUCER THREAD:[139796423935744] count:5
 * IN PRODUCER THREAD:[139796423935744], waiting on cond variable
 * IN PRODUCER THREAD:[139796432328448] count:6
 * Signalling consumer
 * IN PRODUCER THREAD:[139796432328448], waiting on cond variable
 * 
 * IN CONSUMER THREAD
 * Reinit count
 * IN CONSUMER THREAD : Signalling cond variable
 * IN PRODUCER THREAD:[139796390364928] count:1
 * IN PRODUCER THREAD:[139796390364928], waiting on cond variable
 * ...
 *
 */
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

sem_t c_sem;

int count;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

void *producer_thread() {
	while(1) {
		if(0 != pthread_mutex_lock( &mutex1)) {
			printf("Failed to lock, producer thread:[%ld]\n", pthread_self());
		}
		count++;

		printf("IN PRODUCER THREAD:[%ld] count:%d\n", pthread_self(), count);
		if(count == 6) {
			printf("Signalling consumer\n");
			if(-1 == sem_post(&c_sem)) {
				printf("Signalling consumer failed\n");
			}
		}
		printf("IN PRODUCER THREAD:[%ld], waiting on cond variable\n",pthread_self());
		pthread_cond_wait( &condition_var, &mutex1 );
		pthread_mutex_unlock( &mutex1);
	}
}

void *consumer_thread() {
	while(1) {
		if(-1 == sem_wait(&c_sem)) {
			printf("Wait on c_sem failed\n");
			return(NULL);
		}
		printf("\nIN CONSUMER THREAD\n");
		pthread_mutex_lock( &mutex1);
		printf("Reinit count\n");
		count = 0;
		printf("IN CONSUMER THREAD : Signalling cond variable\n"); 
		pthread_cond_broadcast( &condition_var );
		pthread_mutex_unlock( &mutex1);

	}
}

int main() {
	int ret_val = 0;
	int i = 0;
	pthread_t prod_thread[5];
	pthread_t cons_thread;

	if(-1 == sem_init(&c_sem, 0, 0)) {
		printf("init failed\n");
		ret_val = -1;
	}
	for(i = 0; i <= 5; i++) {
		if(-1 == pthread_create(&prod_thread[i], NULL, 
					producer_thread, NULL)) {
			printf("Failed to create producer threads\n");
			ret_val = -1;
		}
	}
	if(-1 == pthread_create(&cons_thread, NULL, 
			consumer_thread, NULL)) {
		printf("Failed to create consumer thread\n");
		ret_val = -1;
	}
	if(-1 == pthread_join(cons_thread, NULL)) {
		printf("Join failed\n");
		ret_val = -1;
	}
	for(i = 0; i <= 5; i++) {
		if(-1 == pthread_join(prod_thread[i], NULL)) {
			printf("Join failed\n");
			ret_val = -1;
		}
	}
	return ret_val;
}
