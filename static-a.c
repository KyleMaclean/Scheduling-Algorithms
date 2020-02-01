// gcc static-a.c library/linkedlist.c library/operations.c -std=c99

#include "library/operations.h"
#include "library/linkedlist.h"
#include <stdlib.h>

int main()
{
    struct process *process = NULL;
	struct element *head = NULL; 
	struct element *tail = NULL;
    double totalResponseTime = 0;
    double totalTurnAroundTime = 0;
	
	for (int i = 0; i < NUMBER_OF_JOBS; i++)
	{
		process = generateProcess();
		addLast((void *) process, &head, &tail);
	}
	
	for (int i = 0; i < NUMBER_OF_JOBS; i++)
	{
		process = removeFirst(&head, &tail);
		struct timeval * startTime = (struct timeval *) malloc (sizeof(struct timeval));
        struct timeval * endTime = (struct timeval *) malloc (sizeof(struct timeval));
        
        runNonPreemptiveJob(process, startTime, endTime);
        
        long int preciseTimeCreated = (process->oTimeCreated.tv_sec * 1000) + process->oTimeCreated.tv_usec;
        long int preciseStartTime = (startTime->tv_sec * 1000) + startTime->tv_usec;
        long int preciseEndTime = (endTime->tv_sec * 1000) + endTime->tv_usec;
        long int responseTime = (preciseStartTime - preciseTimeCreated) / 1000; // slides: "... between creating the job and its first execution"
        long int turnAroundTime = (preciseEndTime - preciseTimeCreated) / 1000; // slides: "... between creating the job and finishing it"
        if (turnAroundTime < 0)
            turnAroundTime += 1000;
        if (responseTime < 0)
            responseTime += 1000;
        totalResponseTime += responseTime;
        totalTurnAroundTime += turnAroundTime;
        free(endTime);
        free(startTime);
        
		printf("Process Id = %d, Previous Burst Time = %d, New Burst Time = %d, Response Time = %d, Turn Around Time = %d\n", (int) process->iProcessId, (int) process->iPreviousBurstTime, (int) process->iRemainingBurstTime, responseTime, turnAroundTime);
        free(process);
	}
	
	long double averageResponseTime = totalResponseTime / NUMBER_OF_JOBS;
    long double averageTurnAroundTime = totalTurnAroundTime / NUMBER_OF_JOBS;
    
	printf("Average response time = %Lf\nAverage turn around time = %Lf\n", averageResponseTime, averageTurnAroundTime);
	
	return 0;
}
