#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include "types_p2.h"
#include "p2_threads.h"
#include "utils.h"
#include <cstring>

pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct timeval t_global_start;
Restroom restroom;

int main(int argc, char** argv)
{
	// This is to set the global start time
	gettimeofday(&t_global_start, NULL);


	pthread_t       tid[2];
	int             status = 0;
	int             work = 0;
	int num;
	
	while (argc != 2) {
    cout << "[ERROR] Expected 1 argument, but got (" << argc - 1 << ")." << endl;
    cout << "[USAGE] p2_exec <number>" << endl;
    return 0;
	}	

	num = atoi(argv[1]);

	restroom.set_input(num);

	bool threadsCreated = false;
	
	int error;
	if ((error = pthread_create(&tid[0], NULL, createPerson, NULL)) || (error = pthread_create(&tid[1], NULL, enterRR, NULL)) || (error = pthread_create(&tid[2], NULL, leaveRR, NULL))) {
    fprintf(stderr, "Error in making thread: %s\n", strerror(error));
	}

	int x = 0;
	while (x < 3) {
    	if (pthread_join(tid[x], NULL)) {
        	fprintf(stderr, "Failed to join threads\n");
        	break;
    	}
    	x++;
	}	
	return 0;
}


