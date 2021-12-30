/*
OS homework02 - Bounded buffer
author: AndyChiang
time: 2021/12/30
line: 187
Don't copy without permission!
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#define MAX_INT 2147483647
#define MIN_INT 0

// four semaphores for max buffer and min buffer
sem_t max_buffer_mutex;
sem_t max_buffer_full;
sem_t min_buffer_mutex;
sem_t min_buffer_full;

// total maximum and minimum
int maximum = MIN_INT;
int minimum = MAX_INT;

// buffers
int big_buffer[1024];
int max_buffer[4];	// shared
int min_buffer[4];	// shared
int max_buffer_index = 0;	// shared
int min_buffer_index = 0;	// shared

void *producer(void*);
void *maximumer(void*);		// consumer for maximum
void *minimumer(void*);		// consumer for minimum

int main(int argc, char* argv[]) {
	// random init big buffer
	srand(time(NULL));
	for(int i=0; i<1024; i++) {
		big_buffer[i] = rand();
	}
	// init max buffer
	for(int i=0; i<4; i++) {
		max_buffer[i] = 0;
	}
	// init min buffer
	for(int i=0; i<4; i++) {
		min_buffer[i] = 0;
	}
	
	// init semaphores
	sem_init(&max_buffer_mutex, 0, 1);
	sem_init(&max_buffer_full, 0, 0);
	sem_init(&min_buffer_mutex, 0, 1);
	sem_init(&min_buffer_full, 0, 0);
	
	// create six threads
	pthread_t th[6];
	int p1 = 0, p2 = 1, p3 = 2, p4 = 3;
	pthread_create(&th[0], NULL, maximumer, NULL);
	pthread_create(&th[2], NULL, producer, (void*)&p1);
	pthread_create(&th[3], NULL, producer, (void*)&p2);
	pthread_create(&th[1], NULL, minimumer, NULL);
	pthread_create(&th[4], NULL, producer, (void*)&p3);
	pthread_create(&th[5], NULL, producer, (void*)&p4);
	
	for(int i=0; i<6; i++) {
		pthread_join(th[i], NULL);
	}
	
	// destory semaphores
	sem_destroy(&max_buffer_mutex);
	sem_destroy(&max_buffer_full);
	sem_destroy(&min_buffer_mutex);
	sem_destroy(&min_buffer_full);
	
	// final maximum and minimum
	printf("Success! maximum=%d and minimum=%d\n", maximum, minimum);
	
	return 0;
}

void *producer(void *arg) {
	int p = *((int*) arg);
	int start, end;
	
	// distribute start and end by argument
	if(p == 0) {
		start = 0;
		end = 255;
	}
	else if(p == 1) {
		start = 256;
		end = 511;
	}
	else if(p == 2) {
		start = 512;
		end = 767;
	}
	else if(p == 3) {
		start = 768;
		end = 1023;
	}
	// calculate max and min
	int temp_max = MIN_INT;
	int temp_min = MAX_INT;
	for(int i=start; i<=end; i++) {
		if(big_buffer[i] > temp_max) {
			temp_max = big_buffer[i];
		}
		if(big_buffer[i] < temp_min) {
			temp_min = big_buffer[i];
		}
	}
	
	printf("Producer: Temporary max=%d and min=%d\n", temp_max, temp_min);
	
	// critical section of max buffer
	sem_wait(&max_buffer_mutex);
	
	// put max into max buffer
	printf("Producer: Put %d into max_buffer at %d\n", temp_max, max_buffer_index);
	max_buffer[max_buffer_index] = temp_max;
	max_buffer_index++;
	
	sem_post(&max_buffer_mutex);
	sem_post(&max_buffer_full);	// full++
	
	// critical section of min buffer
	sem_wait(&min_buffer_mutex);
	
	// put min into min buffer
	printf("Producer: Put %d into min_buffer at %d\n", temp_min, min_buffer_index);
	min_buffer[min_buffer_index] = temp_min;
	min_buffer_index++;
	
	sem_post(&min_buffer_mutex);
	sem_post(&min_buffer_full);	// full++
	
	pthread_exit(NULL);
}

void *maximumer(void *arg) {
	int consumer_index = 0;
	
	// size of max buffer is 4
	while(consumer_index < 4) {
		// critical section of max buffer
		sem_wait(&max_buffer_full);
		sem_wait(&max_buffer_mutex);
		
		// update maximum
		if(max_buffer[consumer_index] > maximum) {
			maximum = max_buffer[consumer_index];
		}
		consumer_index++;
		printf("Consumer: Update! maximum=%d\n", maximum);
		
		sem_post(&max_buffer_mutex);
	}
	
	pthread_exit(NULL);
}

void *minimumer(void *arg) {
	int consumer_index = 0;
	
	// size of min buffer is 4
	while(consumer_index < 4) {
		// critical section of min buffer
		sem_wait(&min_buffer_full);
		sem_wait(&min_buffer_mutex);
		
		// update minimum
		if(min_buffer[consumer_index] < minimum) {
			minimum = min_buffer[consumer_index];
		}
		consumer_index++;
		printf("Consumer: Update! minimum=%d\n", minimum);
		
		sem_post(&min_buffer_mutex);
	}
	
	pthread_exit(NULL);
}
