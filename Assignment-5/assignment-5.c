// assignment-5.c - train station simulation using pthreads and mutexes
// citation: developed with assistance from claude (anthropic llm)

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_STATIONS 5
#define NUM_TRAINS 2

int station_passengers[NUM_STATIONS];
pthread_mutex_t station_mutex[NUM_STATIONS];
pthread_mutex_t stdout_mutex;

int train_capacity[NUM_TRAINS] = {100, 50};
int train_passengers[NUM_TRAINS] = {0, 0};

void *train_thread(void *arg) {
    int id = *(int *)arg;
    int capacity = train_capacity[id];

    while (1) {
        // enter station 0 to pick up passengers
        pthread_mutex_lock(&station_mutex[0]);

        int before_station = station_passengers[0];
        int before_train = train_passengers[id];

        if (before_station > 0) {
            int to_load = before_station;
            if (to_load > capacity - before_train)
                to_load = capacity - before_train;

            sleep(to_load / 10);

            station_passengers[0] -= to_load;
            train_passengers[id] += to_load;

            pthread_mutex_lock(&stdout_mutex);
            printf(" Train %d ENTERS Station 0\n", id);
            printf("    Station 0 has %d passengers left to pick up\n", before_station);
            printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, before_train, capacity);
            printf("        Loading passengers...\n");
            printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, train_passengers[id], capacity);
            printf("    Station 0 has %d passengers left to pick up\n", station_passengers[0]);
            printf(" Train %d LEAVES Station 0\n", id);
            pthread_mutex_unlock(&stdout_mutex);
        } else {
            // no passengers left - print and exit
            pthread_mutex_lock(&stdout_mutex);
            printf(" Train %d ENTERS Station 0\n", id);
            printf("    Station 0 has %d passengers left to arrive\n", before_station);
            printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, before_train, capacity);
            printf("        <Nothing more to do>...\n");
            printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, before_train, capacity);
            printf("    Station 0 has %d passengers left to arrive\n", before_station);
            printf(" Train %d LEAVES Station 0\n", id);
            pthread_mutex_unlock(&stdout_mutex);

            pthread_mutex_unlock(&station_mutex[0]);
            break;
        }

        pthread_mutex_unlock(&station_mutex[0]);
        sleep(1);

        // visit stations 1 through 4 to drop off passengers
        int current_station = 1;
        while (current_station <= 4) {
            pthread_mutex_lock(&station_mutex[current_station]);

            int bs = station_passengers[current_station];
            int bt = train_passengers[id];

            if (bs > 0 && bt > 0) {
                int to_unload = bs;
                if (to_unload > bt)
                    to_unload = bt;

                sleep(to_unload / 10);

                station_passengers[current_station] -= to_unload;
                train_passengers[id] -= to_unload;

                pthread_mutex_lock(&stdout_mutex);
                printf(" Train %d ENTERS Station %d\n", id, current_station);
                printf("    Station %d has %d passengers left to arrive\n", current_station, bs);
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, current_station, bt, capacity);
                printf("        Unloading passengers...\n");
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, current_station, train_passengers[id], capacity);
                printf("    Station %d has %d passengers left to arrive\n", current_station, station_passengers[current_station]);
                printf(" Train %d LEAVES Station %d\n", id, current_station);
                pthread_mutex_unlock(&stdout_mutex);
            } else {
                pthread_mutex_lock(&stdout_mutex);
                printf(" Train %d ENTERS Station %d\n", id, current_station);
                printf("    Station %d has %d passengers left to arrive\n", current_station, bs);
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, current_station, bt, capacity);
                printf("        <Nothing more to do>...\n");
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, current_station, bt, capacity);
                printf("    Station %d has %d passengers left to arrive\n", current_station, bs);
                printf(" Train %d LEAVES Station %d\n", id, current_station);
                pthread_mutex_unlock(&stdout_mutex);
            }

            pthread_mutex_unlock(&station_mutex[current_station]);
            sleep(1);

            // if train is empty, head back to station 0
            if (train_passengers[id] == 0) {
                int back = current_station - 1;
                while (back >= 1) {
                    pthread_mutex_lock(&station_mutex[back]);

                    pthread_mutex_lock(&stdout_mutex);
                    printf(" Train %d ENTERS Station %d\n", id, back);
                    printf("    Station %d has %d passengers left to arrive\n", back, station_passengers[back]);
                    printf("    Train %d is at Station %d and has %d/%d passengers\n", id, back, train_passengers[id], capacity);
                    printf("        <Nothing more to do>...\n");
                    printf("    Train %d is at Station %d and has %d/%d passengers\n", id, back, train_passengers[id], capacity);
                    printf("    Station %d has %d passengers left to arrive\n", back, station_passengers[back]);
                    printf(" Train %d LEAVES Station %d\n", id, back);
                    pthread_mutex_unlock(&stdout_mutex);

                    pthread_mutex_unlock(&station_mutex[back]);
                    sleep(1);
                    back--;
                }
                break;
            }

            current_station++;
        }

        // if train went through station 4 and still not empty, head back
        if (train_passengers[id] > 0 && current_station > 4) {
            int back = 3;
            while (back >= 1) {
                pthread_mutex_lock(&station_mutex[back]);

                pthread_mutex_lock(&stdout_mutex);
                printf(" Train %d ENTERS Station %d\n", id, back);
                printf("    Station %d has %d passengers left to arrive\n", back, station_passengers[back]);
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, back, train_passengers[id], capacity);
                printf("        <Nothing more to do>...\n");
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, back, train_passengers[id], capacity);
                printf("    Station %d has %d passengers left to arrive\n", back, station_passengers[back]);
                printf(" Train %d LEAVES Station %d\n", id, back);
                pthread_mutex_unlock(&stdout_mutex);

                pthread_mutex_unlock(&station_mutex[back]);
                sleep(1);
                back--;
            }
        }
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_TRAINS];
    int train_ids[NUM_TRAINS];

    // initialize station passengers
    station_passengers[0] = 500;
    station_passengers[1] = 50;
    station_passengers[2] = 100;
    station_passengers[3] = 250;
    station_passengers[4] = 100;

    // initialize mutexes
    for (int i = 0; i < NUM_STATIONS; i++) {
        pthread_mutex_init(&station_mutex[i], NULL);
    }
    pthread_mutex_init(&stdout_mutex, NULL);

    // create train threads
    for (int i = 0; i < NUM_TRAINS; i++) {
        train_ids[i] = i;
        pthread_create(&threads[i], NULL, train_thread, &train_ids[i]);
    }

    // wait for both trains to finish
    for (int i = 0; i < NUM_TRAINS; i++) {
        pthread_join(threads[i], NULL);
    }

    // destroy mutexes
    for (int i = 0; i < NUM_STATIONS; i++) {
        pthread_mutex_destroy(&station_mutex[i]);
    }
    pthread_mutex_destroy(&stdout_mutex);

    return 0;
}
