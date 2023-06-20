#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include "types_p3.h"
#include "p3_threads.h"
#include "utils.h"
#include <algorithm>
#include <climits>

// pthread conditional variable to start/resume the thread
pthread_cond_t resume[4];

// pthread conditional variable to wait for threads to finish init
pthread_cond_t init[4];

// pthread conditional variable to signal when the thread's task is done
pthread_cond_t a_task_is_done;

// pthread conditional variable to allow the scheduler to wait for a thread to preempt
pthread_cond_t preempt_task;

// tcbs for each thread
ThreadCtrlBlk tcb[4];

// ready queue of threads
std::vector<int> ready_queue;

// number of tasks that did not miss deadline
int num_of_alive_tasks = 4;

// -1 = no thread working, <number> = thread <number> currently working
int running_thread = -1;

// 0 = don't preempt, 1 = preempt current running thread
int preempt = 0;

// mutex used to protect variables defined in this file
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// mutex used for the task done pthread conditional variable
pthread_mutex_t taskDoneMutex = PTHREAD_MUTEX_INITIALIZER;

// marks the "start time"
struct timeval t_global_start;

// used to tell threads when to stop working (after 240 iterations)
int global_work = 0;

void fifo_schedule(void);
void edf_schedule(void);
void rm_schedule(void);

int main(int argc, char** argv)
{
	if(argc !=2 || atoi(argv[1]) < 0 || atoi(argv[1]) > 2)
	{
		std::cout << "[ERROR] Expecting 1 argument, but got " << argc-1 << std::endl;
		std::cout<< "[USAGE] p3_exec <0, 1, or 2>" << std::endl;
		return 0;
	}
	int schedule = atoi(argv[1]);

	// pthreads we are creating
	pthread_t tid[4];

	// This is to set the global start time
	gettimeofday(&t_global_start, NULL);

	// initialize all tcbs
	tcb[0].id = 0;
	tcb[0].task_time = 200;
	tcb[0].period = 1000;
	tcb[0].deadline = 1000;

	tcb[1].id = 1;
	tcb[1].task_time = 500;
	tcb[1].period = 2000;
	tcb[1].deadline = 2000;

	tcb[2].id = 2;
	tcb[2].task_time = 1000;
	tcb[2].period = 4000;
	tcb[2].deadline = 4000;

	tcb[3].id = 3;
	tcb[3].task_time = 1000;
	tcb[3].period = 6000;
	tcb[3].deadline = 6000;

	// initialize all pthread conditional variables
	for (int i = 0; i < 4; i++) {
		pthread_cond_init(&resume[i], NULL);
		pthread_cond_init(&init[i], NULL);
	}
	pthread_cond_init(&a_task_is_done, NULL);
	pthread_cond_init(&preempt_task, NULL);

	// allow all threads to work
	global_work = 1;
	printf("[Main] Create worker threads\n");

	// create pthreads and pass their respective tcb as a parameter
	pthread_mutex_lock(&mutex);
	for (int i = 0; i < 4; i++) {
		if(pthread_create(&tid[i], NULL, threadfunc, &tcb[i])) {
			fprintf(stderr, "Error creating thread\n");
		}
		// Wait until the threads are "ready" and in the ready queue
		pthread_cond_wait(&init[i], &mutex);
	}
	pthread_mutex_unlock(&mutex);

	// Reset the global time and skip the initial wait
	gettimeofday(&t_global_start, NULL);

	for (int i = 0; i < 240; i++) {
		// Select scheduler based on argv[1]
		switch(schedule) {
			case 0:
				fifo_schedule();
				break;
			case 1:
				edf_schedule();
				break;
			case 2:
				rm_schedule();
				break;
		}

		// Wait until the next 100ms interval or until a task is done
		int sleep = 100 - (get_time_stamp() % 100);
		if (num_of_alive_tasks > 0) {
			timed_wait_for_task_complition(sleep);
		} else {
			printf("All the tasks missed the deadline\n");
			break;
		}
	}

	// after 240 iterations, finish off all threads
	printf("[Main] It's time to finish the threads\n");

	printf("[Main] Locks\n");
	pthread_mutex_lock(&mutex);
	global_work = 0;

	// signal all the processes in the ready queue so they finish
	usleep(MSEC(3000));
	while (ready_queue.size() > 0) {
		pthread_cond_signal(&resume[ready_queue[0]]);
		ready_queue.erase(ready_queue.begin());
	}

	printf("[Main] Unlocks\n");
	pthread_mutex_unlock(&mutex);

	/* wait for the threads to finish */
	for (int i = 0; i < 4; i++) {
		if(pthread_join(tid[i], NULL)) {
			fprintf(stderr, "Error joining thread\n");
		}
	}

	return 0;
}

void fifo_schedule()
{
    pthread_mutex_lock(&mutex);

    if (ready_queue.empty()) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    int thread_id = ready_queue.front();
    ready_queue.erase(ready_queue.begin());

    printf("[%6lu ms][Scheduler] Scheduling Thread %d\n", get_time_stamp(), thread_id);

    // signal the scheduled thread to start/resume
    pthread_cond_signal(&resume[thread_id]);

    pthread_mutex_unlock(&mutex);
}


void edf_schedule(void)
{
    pthread_mutex_lock(&mutex);

    if (ready_queue.empty()) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    // find the thread with the earliest deadline
    int earliest_deadline = INT_MAX;
    int scheduled_thread = -1;

    for (int i = 0; i < ready_queue.size(); i++) {
        int thread_id = ready_queue[i];
        ThreadCtrlBlk* thread_tcb = &tcb[thread_id];

        if (thread_tcb->deadline < earliest_deadline) {
            earliest_deadline = thread_tcb->deadline;
            scheduled_thread = thread_id;
        }
    }

    if (scheduled_thread != -1) {
        printf("[%6lu ms][Scheduler] Scheduling Thread %d\n", get_time_stamp(), scheduled_thread);

        // remove the scheduled thread from the ready queue
        ready_queue.erase(std::find(ready_queue.begin(), ready_queue.end(), scheduled_thread));

        // signal the scheduled thread to start/resume
        pthread_cond_signal(&resume[scheduled_thread]);
    }

    pthread_mutex_unlock(&mutex);
}


void rm_schedule()
{
    pthread_mutex_lock(&mutex);

    if (ready_queue.empty()) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    // sort the ready queue based on the period rate of the threads
    std::sort(ready_queue.begin(), ready_queue.end(), [](int thread1, int thread2) {
        return tcb[thread1].period < tcb[thread2].period;
    });

    // schedule the thread with the highest priority 
    int scheduled_thread = ready_queue.front();

    printf("[%6lu ms][Scheduler] Scheduling Thread %d\n", get_time_stamp(), scheduled_thread);

    // remove the scheduled thread from the ready queue
    ready_queue.erase(ready_queue.begin());

    // signal the scheduled thread to start/resume
    pthread_cond_signal(&resume[scheduled_thread]);

    pthread_mutex_unlock(&mutex);
}
