#include <stdio.h>
#include <pthread.h>

/*
 * Problem with this code is that there is no synch between
 * the threads.
 * So func1 might run continuously and print line 18 multiple
 * times before func2 gets scheduled
 * So the output might end up looking like:
 * <thread-id of func1>: Unlocking mutex cnt:<num>
 *		the <num> might not change for various cycles
 */
pthread_mutex_t cnt_m = PTHREAD_MUTEX_INITIALIZER;
int cnt_g = 0;

void * func1() {
	pthread_mutex_lock(&cnt_m);
	do {
		if (cnt_g % 2 == 0) {
			printf("thread id:%ld val:%d\n", pthread_self(), cnt_g);
			cnt_g++;
		} else {
			printf("%ld: Unlocking mutex cnt:%d\n", pthread_self(), cnt_g);
			pthread_mutex_unlock(&cnt_m);
		}
	} while (cnt_g < 100);
}

void * func2() {
	pthread_mutex_lock(&cnt_m);
	do {
		if (cnt_g % 2 != 0) {
			printf("thread id:%ld val:%d\n", pthread_self(), cnt_g);
			cnt_g++;
		} else {
			printf("%ld: Unlocking mutex cnt:%d\n", pthread_self(), cnt_g);
			pthread_mutex_unlock(&cnt_m);
		}
	} while (cnt_g < 100);
}

int main() {
	pthread_t t1;
	pthread_t t2;

	if (pthread_create(&t1, NULL, &func1, NULL) != 0) {
		printf("Failed to create thread 1\n");
	}
	if (pthread_create(&t2, NULL, &func2, NULL) != 0) {
		printf("Failed to create thread 2\n");
	}
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
}
