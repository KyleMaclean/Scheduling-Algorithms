// gcc windows-pb.c library/linkedlist.c library/operations.c -lpthread -std=c99

#define _BSD_SOURCE // required to not have an implicit declaration of usleep
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "library/operations.h"
#include "library/linkedlist.h"
double dAverageResponseTime, dAverageTurnAroundTime;
#include "library/printFunction.c"

sem_t sSync, sFull, sEmpty;
int producer_id, consumer_id, produced, consumed;

struct timeval *startTime, *endTime;
struct process *newProcess, *lowestPriorityProcess, *consumingProcess, *boostableProcess;
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

void lookForLowerPriorityProcessThan(struct process * pProcess)
{
	for (int i = MAX_PRIORITY/2; i > pProcess->iPriority; i--) // check all FCFS priorities lower than pProcess's
	{
		if (heads[i] != NULL && tails[i] != NULL) // found a lower priority process
		{
			// lower priority process is removed (to get a reference to it), preempted, and added back to its queue
			lowestPriorityProcess = removeFirst(&heads[i], &tails[i]);
			preemptJob(lowestPriorityProcess);
			addFirst((void *) lowestPriorityProcess, &heads[lowestPriorityProcess->iPriority], &tails[lowestPriorityProcess->iPriority]);
			
			printf("Pre-empted job: Pre-empted Process Id = %d, Pre-empted Priority %d, New Process Id %d, New priority %d\n",
			lowestPriorityProcess->iProcessId, lowestPriorityProcess->iPriority, pProcess->iProcessId, pProcess->iPriority);
			
			usleep((MAX_BURST_TIME / 2) * 1000); // sleep once for every preempted process
			break; // stop searching for lower priority jobs	
		}
	}
}

void *booster()
{
	while (consumed != NUMBER_OF_JOBS)
	{
		for (int i = MAX_PRIORITY/2; i < MAX_PRIORITY; i++)
		{
			sem_wait(&sSync);
			if (heads[i] != NULL && tails[i] != NULL)
			{
				boostableProcess = removeFirst(&heads[i], &tails[i]);
				struct timeval oCurrent;
				gettimeofday(&oCurrent, NULL);
				long int difference = getDifferenceInMilliSeconds(boostableProcess->oMostRecentTime, oCurrent);	
				
				if (difference > BOOST_INTERVAL)
				{
					// put process at the head of the highest priority RR queue
					addFirst((void *) boostableProcess, &heads[MAX_PRIORITY/2], &tails[MAX_PRIORITY/2]);
					printf("Boost priority: Process Id = %d, Priority = %d, New Priority = %d\n",
					boostableProcess->iProcessId, i, MAX_PRIORITY/2);
				}
				else
				{
					// put process back at the head of its original queue 
					addFirst((void *) boostableProcess, &heads[i], &tails[i]);
				}
			}
			sem_post(&sSync);
		}
	}
	pthread_exit(0);
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
    			consumingProcess = removeFirst(&heads[i], &tails[i]);
				runJob(consumingProcess, startTime, endTime);
				if (consumingProcess->iRemainingBurstTime == 0)
				{
					consumed++;
					sem_post(&sEmpty);
				}
				else
				{
					addLast((void *) consumingProcess, &heads[consumingProcess->iPriority], &tails[consumingProcess->iPriority]);
				}
				processJob(consumer_id, consumingProcess, *startTime, *endTime);
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

void *producer(void *id)
{
    while (produced < NUMBER_OF_JOBS)
    {
		sem_wait(&sEmpty);
        sem_wait(&sSync);
        newProcess = generateProcess();
		if (newProcess->iPriority <= MAX_PRIORITY/2) // if FCFS
		{
			lookForLowerPriorityProcessThan(newProcess);
		}		
		addLast((void *) newProcess, &heads[newProcess->iPriority], &tails[newProcess->iPriority]);
		produced++;
		sem_post(&sFull);
		printProducedProcess(*((int*) id), newProcess);
		sem_post(&sSync);
    }
}

int main()
{
    pthread_t producer_thread, consumer_thread, booster_thread;
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
	
	if (pthread_create(&booster_thread, NULL, booster, NULL) == -1)
	{
		printf("failed to create 'booster' thread\n");
		exit(-1);
	}
    
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);
	pthread_join(booster_thread, NULL);
    
    printf("Average Response Time = %f \nAverage Turn Around Time = %f \n",
	dAverageResponseTime/NUMBER_OF_JOBS, dAverageTurnAroundTime/NUMBER_OF_JOBS);
	return 0;
}
