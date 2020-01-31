// Kyle James Maclean (username = psykjm) (id = 14327105)
// gcc task3.c linkedlist.c coursework.c -lpthread -std=c99

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "coursework.h"
#include "linkedlist.h"

sem_t sSync, sJobs, sDelayProducer;
int sSync_value, sJobs_value, sDelayProducer_value, producer_id, consumer_id, produced, consumed;
struct element *head, *tail;

void visualisation(char *name, int id)
{
    int stars;
    printf("%s Id = %d, Produced = %d, Consumed = %d: ",
           name, id, produced, consumed);
    for (stars = 0; stars < (produced - consumed); stars++)
        printf("*");
    printf("\n");
}

void *producer(void *id)
{
    while (produced < NUMBER_OF_JOBS)
    {
        sem_wait(&sSync);
        addLast((void *) "*", &head, &tail);
        produced++;
        visualisation("Producer", *((int*) id));
        sem_post(&sSync);
        sem_post(&sJobs);
		if ((produced - consumed) == MAX_BUFFER_SIZE)
			sem_wait(&sDelayProducer);
    }
}

void *consumer(void *id)
{
    while (consumed < NUMBER_OF_JOBS)
    {
        sem_wait(&sJobs);
        sem_wait(&sSync);
        removeFirst(&head, &tail);
        consumed++;
		if ((produced - consumed) == MAX_BUFFER_SIZE - 1)
			sem_post(&sDelayProducer);
        visualisation("Consumer", *((int*) id));
        sem_post(&sSync);
    }
}

int main()
{
    pthread_t producer_thread, consumer_thread;
    producer_id = consumer_id = produced = consumed = 0;
    
    sem_init(&sSync,0,1); // binary, synchronise critical section, initialised to 1
	sem_init(&sDelayProducer,0,0); // binary, makes producer go to sleep, initialised to 0
	sem_init(&sJobs,0,0); // counting, representing number of jobs, initialised to 0
    
    if (pthread_create(&producer_thread, NULL, producer, (void *) &producer_id) == -1)
	{
		printf("failed to create 'producer' thread\n");
		exit(-1);
	}
	if (pthread_create(&consumer_thread, NULL, consumer, (void *) &consumer_id) == -1)
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
    if (sem_getvalue(&sDelayProducer, &sDelayProducer_value) == -1)
    {
        printf("failed to get 'sDelayProducer' semaphore value\n");
        exit(-1);
    }
    if (sem_getvalue(&sJobs, &sJobs_value) == -1)
    {
        printf("failed to get 'sJobs' semaphore value\n");
        exit(-1);
    }
    
    printf("sSync = %d, sDelayProducer = %d, sJobs = %d\n", sSync_value, sDelayProducer_value, sJobs_value);
	return 0;
}
