// Kyle James Maclean (username = psykjm) (id = 14327105)
// gcc task2.c -lpthread -std=c99

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "coursework.h"
#include "linkedlist.h"

sem_t sSync, sDelayConsumer;
int sSync_value, sDelayConsumer_value, counter, produced, consumed;

void visualisation(char *name)
{
	int stars;
    printf("%s, Produced = %d, Consumed = %d: ", name, produced, consumed);
	for (stars = 0; stars < counter; stars++)
		printf("*");
	printf("\n");
}

void *producer()
{
	while (produced < NUMBER_OF_JOBS)
	{
		sem_wait(&sSync);
		counter++;
		produced++;
		visualisation("Producer");
        if (counter == 1)
            sem_post(&sDelayConsumer); // wake up consumer after buffer increases to singleton
		sem_post(&sSync);
	}
	pthread_exit(NULL);
}

void *consumer()
{
	int temp;
    sem_wait(&sDelayConsumer);
	while (1)
    {
		sem_wait(&sSync);
		counter--;
		consumed++;
		visualisation("Consumer");
        temp = counter;
        sem_post(&sSync);
		if (consumed == NUMBER_OF_JOBS)
			pthread_exit(NULL);
		if (temp == 0)
			sem_wait(&sDelayConsumer);
	}
}

int main()
{
	pthread_t producer_thread, consumer_thread;
	counter = produced = consumed = 0;
    sem_init(&sSync,0,1); // slides: "synchronises acccess to the buffer (counter), initialised to 1
    sem_init(&sDelayConsumer,0,0); // slides: "ensures that the consumer goes to sleep when there are no items available, initialised to 0"
	
	if (pthread_create(&producer_thread, NULL, producer, (void *) NULL) == -1)
	{
		printf("failed to create 'producer' thread\n");
		exit(-1);
	}
	if (pthread_create(&consumer_thread, NULL, consumer, NULL) == -1)
	{
		printf("failed to create 'consumer' thread\n");
		exit(-1);
	}
	
	pthread_join(producer_thread, NULL);
	pthread_join(consumer_thread, NULL);
    
    if (sem_getvalue(&sSync, &sSync_value) == -1)
    {
        printf("failed to get 'sSync' semaphore value\n");
        exit(-1);
    }
    if (sem_getvalue(&sDelayConsumer, &sDelayConsumer_value) == -1)
    {
        printf("failed to get 'sDelayConsumer' semaphore value\n");
        exit(-1);
    }
    
    printf("sSync = %d, sDelayConsumer = %d\n", sSync_value, sDelayConsumer_value);
	return 0;
}

