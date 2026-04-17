#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100

typedef struct {
    int id;
    int arrival;
    int burst;
    int burst_left;
    int wait;
    int turnaround;
    int finished;
} Process;

int read_input(const char *filename, Process procs[], int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "error: cannot open file '%s'\n", filename);
        return -1;
    }
    *count = 0;
    int id, burst;
    while (fscanf(fp, "P%d,%d\n", &id, &burst) == 2) {
        procs[*count].id = id;
        procs[*count].arrival = id;
        procs[*count].burst = burst;
        procs[*count].burst_left = burst;
        procs[*count].wait = 0;
        procs[*count].turnaround = 0;
        procs[*count].finished = 0;
        (*count)++;
        if (*count >= MAX_PROCESSES) break;
    }
    fclose(fp);
    if (*count == 0) {
        fprintf(stderr, "error: no valid processes found in file\n");
        return -1;
    }
    return 0;
}

int all_done(Process procs[], int count) {
    for (int i = 0; i < count; i++) {
        if (procs[i].burst_left > 0) return 0;
    }
    return 1;
}

void print_summary(Process procs[], int count) {
    double total_wait = 0, total_turnaround = 0;
    for (int i = 0; i < count; i++) {
        printf("\nP%d\n", procs[i].id);
        printf("\tWaiting time:\t\t%d\n", procs[i].wait);
        printf("\tTurnaround time:\t%d\n", procs[i].turnaround);
        total_wait += procs[i].wait;
        total_turnaround += procs[i].turnaround;
    }
    printf("\nTotal average waiting time:\t%.1f\n", total_wait / count);
    printf("Total average turnaround time:\t%.1f\n", total_turnaround / count);
}

void run_fcfs(Process procs[], int count) {
    printf("First Come First Served\n");
    int time = 0;
    int active = -1;

    while (!all_done(procs, count)) {
        // select process: pick first arrived process that's not done
        if (active == -1 || procs[active].burst_left == 0) {
            active = -1;
            for (int i = 0; i < count; i++) {
                if (procs[i].arrival <= time && procs[i].burst_left > 0) {
                    active = i;
                    break;
                }
            }
        }

        if (active == -1) {
            time++;
            continue;
        }

        // print tick line
        printf("T%d : P%-2d - Burst left %2d, Wait time %d, Turnaround time %d\n",
               time, procs[active].id, procs[active].burst_left,
               procs[active].wait, procs[active].turnaround);

        // update all processes
        for (int i = 0; i < count; i++) {
            if (procs[i].burst_left == 0) continue;
            if (time < procs[i].arrival) continue;

            if (i == active) {
                procs[i].burst_left--;
                procs[i].turnaround++;
            } else {
                procs[i].wait++;
                procs[i].turnaround++;
            }
        }
        time++;
    }
    print_summary(procs, count);
}

void run_sjf(Process procs[], int count) {
    printf("Shortest Job First\n");
    int time = 0;

    while (!all_done(procs, count)) {
        // select process with shortest burst_left (preemptive)
        int active = -1;
        for (int i = 0; i < count; i++) {
            if (procs[i].arrival <= time && procs[i].burst_left > 0) {
                if (active == -1 || procs[i].burst_left < procs[active].burst_left) {
                    active = i;
                }
            }
        }

        if (active == -1) {
            time++;
            continue;
        }

        printf("T%d : P%-2d - Burst left %2d, Wait time %2d, Turnaround time %2d\n",
               time, procs[active].id, procs[active].burst_left,
               procs[active].wait, procs[active].turnaround);

        for (int i = 0; i < count; i++) {
            if (procs[i].burst_left == 0) continue;
            if (time < procs[i].arrival) continue;

            if (i == active) {
                procs[i].burst_left--;
                procs[i].turnaround++;
            } else {
                procs[i].wait++;
                procs[i].turnaround++;
            }
        }
        time++;
    }
    print_summary(procs, count);
}

void run_rr(Process procs[], int count, int quantum) {
    printf("Round Robin with Quantum %d\n", quantum);
    int time = 0;
    int active = -1;
    int q_remaining = 0;
    int last = -1; // last process index that ran, for cyclic scan

    while (!all_done(procs, count)) {
        // if current process finished or quantum expired, find next
        if (active != -1 && procs[active].burst_left == 0) {
            last = active;
            active = -1;
            q_remaining = 0;
        }

        if (active != -1 && q_remaining == 0) {
            last = active;
            active = -1;
        }

        if (active == -1) {
            // cyclic scan: start from last+1, wrap around
            for (int j = 0; j < count; j++) {
                int idx = (last + 1 + j) % count;
                if (procs[idx].arrival <= time && procs[idx].burst_left > 0) {
                    active = idx;
                    q_remaining = quantum;
                    break;
                }
            }
        }

        if (active == -1) {
            time++;
            continue;
        }

        printf("T%d : P%-2d - Burst left %2d, Wait time %2d, Turnaround time %2d\n",
               time, procs[active].id, procs[active].burst_left,
               procs[active].wait, procs[active].turnaround);

        for (int i = 0; i < count; i++) {
            if (procs[i].burst_left == 0) continue;
            if (time < procs[i].arrival) continue;

            if (i == active) {
                procs[i].burst_left--;
                procs[i].turnaround++;
            } else {
                procs[i].wait++;
                procs[i].turnaround++;
            }
        }

        q_remaining--;
        time++;
    }
    print_summary(procs, count);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s -f|-s|-r <quantum> <filename>\n", argv[0]);
        return 1;
    }

    Process procs[MAX_PROCESSES];
    int count = 0;
    char *filename;
    int quantum = 0;

    if (strcmp(argv[1], "-f") == 0) {
        if (argc != 3) {
            fprintf(stderr, "usage: %s -f <filename>\n", argv[0]);
            return 1;
        }
        filename = argv[2];
        if (read_input(filename, procs, &count) != 0) return 1;
        run_fcfs(procs, count);
    } else if (strcmp(argv[1], "-s") == 0) {
        if (argc != 3) {
            fprintf(stderr, "usage: %s -s <filename>\n", argv[0]);
            return 1;
        }
        filename = argv[2];
        if (read_input(filename, procs, &count) != 0) return 1;
        run_sjf(procs, count);
    } else if (strcmp(argv[1], "-r") == 0) {
        if (argc != 4) {
            fprintf(stderr, "usage: %s -r <quantum> <filename>\n", argv[0]);
            return 1;
        }
        quantum = atoi(argv[2]);
        if (quantum <= 0) {
            fprintf(stderr, "error: time quantum must be a positive integer\n");
            return 1;
        }
        filename = argv[3];
        if (read_input(filename, procs, &count) != 0) return 1;
        run_rr(procs, count, quantum);
    } else {
        fprintf(stderr, "error: invalid option '%s'. use -f, -s, or -r\n", argv[1]);
        return 1;
    }

    return 0;
}
