#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

/*
 * This is a simple program testing producer consumer 
 * concept using mutex and condition variables
 * 5 producer threads are created and wait on cond
 * variable, the consumer thread broadcasts the cond
 * variable and then the producer threads increment 
 * and print the count
 */
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t my_cond_var = PTHREAD_COND_INITIALIZER;

sem_t my_sem;
int count = 0;

void * func_thread() {
	while (1) {
		pthread_mutex_lock(&my_mutex);
		pthread_cond_wait(&my_cond_var, &my_mutex);
		count++;
		printf("From thread[%ld]: count: %d\n", pthread_self(), count);
		pthread_mutex_unlock(&my_mutex);
	}
}

void * cons_thread() {
	while(1) {
		pthread_mutex_lock(&my_mutex);
		printf("From cons thread[%ld]: count:%d\n", pthread_self(), count);
		pthread_cond_broadcast(&my_cond_var);
		pthread_mutex_unlock(&my_mutex);
		//sem_post(&my_sem);
	}
}

int main() {
	pthread_t tid[5], cons_tid;
	int i;

	//sem_init(&my_sem, 0, 5);
	pthread_cond_init(&my_cond_var, NULL);
	for(i=0;i<5;i++) {
		if (pthread_create(&tid[i], NULL, func_thread, NULL) != 0) {	
			printf("Failed to create thread[%d]\n", i);
		}
	}
	pthread_create(&cons_tid, NULL, cons_thread, NULL);
	for(i=0;i<5;i++) {
		pthread_join(tid[i], NULL);
	}
}
