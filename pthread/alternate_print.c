#include <stdio.h>
#include <pthread.h>

pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
int global_count = 0;

static void * odd_thread() {
	while (1) {
		pthread_mutex_lock(&count_lock);
		while ((global_count) % 2 != 0) {
			pthread_cond_wait(&cond_var, &count_lock);
			printf("thread id: %ld value:%d\n", pthread_self(), global_count);
		}
		global_count++;
		pthread_cond_signal(&cond_var);
		if(global_count > 100) {
			pthread_mutex_unlock(&count_lock);
			return(NULL);
		}
		pthread_mutex_unlock(&count_lock);
	}
}

static void * even_thread() {
	while (1) {
		pthread_mutex_lock(&count_lock);
		while (global_count % 2 == 0) {
			pthread_cond_wait(&cond_var, &count_lock);
			printf("thread id: %ld value:%d\n", pthread_self(), global_count);
		}
		global_count++;
		pthread_cond_signal(&cond_var);
		if(global_count > 100) {
			pthread_mutex_unlock(&count_lock);
			return(NULL);
		}
		pthread_mutex_unlock(&count_lock);
	}
}

int main() {
	pthread_t odd_tid;
	pthread_t even_tid;

	if(pthread_create(&odd_tid, NULL, &odd_thread, NULL) != 0) {
		printf("Failed to create odd thread\n");
	}
	if(pthread_create(&even_tid, NULL, &even_thread, NULL) != 0) {
		printf("Failed to create even thread\n");
	}
	pthread_join(odd_tid, NULL);
	pthread_join(even_tid, NULL);
	pthread_mutex_destroy(&count_lock);
	pthread_cond_destroy(&cond_var);
	return 0;
}
