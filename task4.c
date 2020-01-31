// Kyle James Maclean (username = psykjm) (id = 14327105)
// gcc task4.c linkedlist.c coursework.c -lpthread -std=c99

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "coursework.h"
#include "linkedlist.h"
double dAverageResponseTime, dAverageTurnAroundTime;
#include "printFunction.c"

sem_t sSync, sFull, sEmpty;
int producer_id, consumer_id, produced, consumed;

struct timeval *startTime, *endTime;
struct process *process;
struct element *tails[MAX_PRIORITY], *heads[MAX_PRIORITY];

void printProducedProcess(int producer_id, struct process * pProcess)
{
	
	if (pProcess->iPriority <= MAX_PRIORITY/2)
		printf("Producer %d, Process Id = %d (FCFS), Priority = %d, Initial Burst Time %d\n",
		producer_id, pProcess->iProcessId, pProcess->iPriority, pProcess->iInitialBurstTime);
	if (pProcess->iPriority > MAX_PRIORITY/2)
		printf("Producer %d, Process Id = %d (RR), Priority = %d, Initial Burst Time %d\n",
		producer_id, pProcess->iProcessId, pProcess->iPriority, pProcess->iInitialBurstTime);
}

void *producer(void *id)
{
    while (produced < NUMBER_OF_JOBS)
    {
		sem_wait(&sEmpty);
        sem_wait(&sSync);
        process = generateProcess();
        addLast((void *) process, &heads[process->iPriority], &tails[process->iPriority]);
        produced++;
		sem_post(&sFull);
        printProducedProcess(*((int*) id), process);
        sem_post(&sSync);
    }
}

void *consumer(void *id)
{
	int consumer_id = *(int*) id - 1;
    while (1)
    {
		sem_wait(&sFull);
    	sem_wait(&sSync);
    	for (int i = 0; i < MAX_PRIORITY; i++)
    	{
    		if (heads[i] != NULL && tails[i] != NULL)
    		{
    			process = removeFirst(&heads[i], &tails[i]);
				runJob(process, startTime, endTime);
				if (process->iRemainingBurstTime == 0)
				{
					consumed++;
					sem_post(&sEmpty);
				}
				else
				{
					addLast((void *) process, &heads[process->iPriority], &tails[process->iPriority]);
				}
				processJob(consumer_id, process, *startTime, *endTime);
				i = 0;
    		}
    	}
    	sem_post(&sSync);
		if (consumed == NUMBER_OF_JOBS)
        {
			sem_post(&sFull); // wake up other consumers that are waiting for jobs that will never come
        	pthread_exit(0); // "end gracefully/in a correct manner"
        }
    }
	
}

int main()
{
    pthread_t producer_thread, consumer_thread;
    producer_id = consumer_id = produced = consumed = 0;
    dAverageResponseTime = dAverageTurnAroundTime = 0;
    
    startTime = (struct timeval *) malloc (sizeof(struct timeval));
    endTime = (struct timeval *) malloc (sizeof(struct timeval));

    sem_init(&sSync,0,1); // binary, synchronise critical section, initialised to 1
	sem_init(&sFull,0,0); // counting, representing number of waiting jobs, initialised to 0
	sem_init(&sEmpty,0,MAX_BUFFER_SIZE); // counting, representing number of spaces for jobs, initialised to MAX_BUFFER_SIZE
    
	for (int i = 0; i < MAX_PRIORITY; i++)
	{
		heads[i] = NULL;
		tails[i] = NULL;
	}

    if (pthread_create(&producer_thread, NULL, producer, (void *) &producer_id) == -1)
	{
		printf("failed to create 'producer' thread\n");
		exit(-1);
	}

	for (; consumer_id < NUMBER_OF_CONSUMERS; consumer_id++)
	{
		if (pthread_create(&consumer_thread, NULL, consumer, (void *) &consumer_id) == -1)
		{
			printf("failed to create 'consumer' (id = %d) thread\n", consumer_id);
			exit(-1);
		}
	}
    
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);
    
    printf("Average Response Time = %f \nAverage Turn Around Time = %f \n",
	dAverageResponseTime/NUMBER_OF_JOBS, dAverageTurnAroundTime/NUMBER_OF_JOBS);
	return 0;
}
