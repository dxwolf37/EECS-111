#include "p2_threads.h"
#include "utils.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

extern Restroom restroom;
extern struct timeval t_global_start;
extern pthread_mutex_t mutex;

/*void *threadfunc(void *parm)
{
	int status;
	printf(" [Thread] Start\n");

	printf(" [Thread] Locks\n");
	status = pthread_mutex_lock(&mutex);

    printf(" [Thread] Blocked\n");
    status = pthread_cond_wait(&cond, &mutex);

	printf(" [Thread] Starts again.\n");
	for (int i=0; i<3; i++) {
		printf(" [Thread] Complete thread after (%d) seconds\n", (3-i));
		usleep(MSEC(1000));
	}

	printf(" [Thread] Unlocks\n");
	status = pthread_mutex_unlock(&mutex);
	printf(" [Thread] Complete\n");
}*/

void* createPerson(void* parm) {
    struct timeval cTime;
    srand(time(NULL));
    int sum = 2 * restroom.get_input();
    int numMen = restroom.get_input();
    int numWomen = numMen;
    int a = 0;

    std::vector<int> genderChoices(sum);
    for (int i = 0; i < numMen; ++i) {
        genderChoices[i] = 0; // men
    }
    for (int i = numMen; i < sum; ++i) {
        genderChoices[i] = 1; // women
    }
    for (int i = sum - 1; i > 0; --i) {
    	int j = rand() % (i + 1);
        std::swap(genderChoices[i], genderChoices[j]);
    }

    while (a < sum) {
        int t = 3 + (rand() % 10);
        Person person;

        int genderChoice = genderChoices[a];
        person.set_gender(genderChoice);

        person.set_order(1 + a);

        if (t > 10) {
            int temp = t - 10;
            t -= temp;
        }

        person.set_time(t);

        if (person.get_gender() == 1) {
            numWomen--;
        } else {
            numMen--;
        }

        pthread_mutex_lock(&mutex);
        restroom.q_vector.push_back(person);
        pthread_mutex_unlock(&mutex);

        usleep(MSEC((1 + rand() % 5)));

        a++;
    }

    pthread_exit(0);
}

void *enterRR(void *parameter) {
    struct timeval cTime;
    srand(time(NULL));
    int isMoving = 0;
    int counter = 0;
    int sum = 2 * restroom.get_input();

    while (isMoving != sum) {
        if (!restroom.q_vector.empty()) {
            pthread_mutex_lock(&mutex);
            gettimeofday(&cTime, NULL);

            int gender = restroom.q_vector[0].get_gender();
            int restroomStatus = restroom.get_status();

            if ((gender == 0 && restroomStatus != 1) || (gender == 1 && restroomStatus != 2)) {
                if (gender == 0) {
                    restroom.man_wants_to_enter(restroom.q_vector[0]);
                    cout << "[Restroom] (Man) goes into the restroom, State is ";
                } else {
                    restroom.woman_wants_to_enter(restroom.q_vector[0]);
                    cout << "[Restroom] (Woman) goes into the restroom, State is ";
                }
                counter++;
                restroom.status_switched(restroom.q_vector[0]);
                restroom.print_status();
                restroom.q_vector.erase(restroom.q_vector.begin());
                isMoving++;
            }
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(0);
}

void *leaveRR(void *parameter) {
    struct timeval cTime;
    int sum = 2 * restroom.get_input();

    while (sum != 0) {
        for (int a = 0; a < restroom.rr_vector.size(); a++) {
            if (restroom.rr_vector[a].ready_to_leave()) {
                pthread_mutex_lock(&mutex);
                gettimeofday(&cTime, NULL);
                int gender = restroom.rr_vector[a].get_gender();
                cout << "[" << get_elasped_time(t_global_start, cTime) << "ms]";
                restroom.remove_person(a);
                sum--;
                a--;
                pthread_mutex_unlock(&mutex);
            }
        }
    }

    pthread_exit(0);
}
