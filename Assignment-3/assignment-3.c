#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int grid[9][9];
int results[27];

typedef struct {
    int thread_num;
    int index;
} thread_param;

void *check_subgrid(void *arg) {
    thread_param *p = (thread_param *)arg;
    int start_row = (p->index / 3) * 3;
    int start_col = (p->index % 3) * 3;
    int seen[10] = {0};

    for (int i = start_row; i < start_row + 3; i++) {
        for (int j = start_col; j < start_col + 3; j++) {
            int val = grid[i][j];
            if (val < 1 || val > 9 || seen[val]) {
                results[p->thread_num - 1] = 0;
                pthread_exit(NULL);
            }
            seen[val] = 1;
        }
    }
    results[p->thread_num - 1] = 1;
    pthread_exit(NULL);
}

void *check_row(void *arg) {
    thread_param *p = (thread_param *)arg;
    int seen[10] = {0};

    for (int j = 0; j < 9; j++) {
        int val = grid[p->index][j];
        if (val < 1 || val > 9 || seen[val]) {
            results[p->thread_num - 1] = 0;
            pthread_exit(NULL);
        }
        seen[val] = 1;
    }
    results[p->thread_num - 1] = 1;
    pthread_exit(NULL);
}

void *check_column(void *arg) {
    thread_param *p = (thread_param *)arg;
    int seen[10] = {0};

    for (int i = 0; i < 9; i++) {
        int val = grid[i][p->index];
        if (val < 1 || val > 9 || seen[val]) {
            results[p->thread_num - 1] = 0;
            pthread_exit(NULL);
        }
        seen[val] = 1;
    }
    results[p->thread_num - 1] = 1;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open file %s\n", argv[1]);
        return 1;
    }

    // read grid from file
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (fscanf(fp, "%d", &grid[i][j]) != 1) {
                fprintf(stderr, "Error: invalid file format\n");
                fclose(fp);
                return 1;
            }
    fclose(fp);

    pthread_t threads[27];
    thread_param params[27];

    // threads 1-9: subgrids, 10-18: rows, 19-27: columns
    for (int i = 0; i < 9; i++) {
        params[i].thread_num = i + 1;
        params[i].index = i;
        pthread_create(&threads[i], NULL, check_subgrid, &params[i]);
    }
    for (int i = 0; i < 9; i++) {
        params[9 + i].thread_num = 10 + i;
        params[9 + i].index = i;
        pthread_create(&threads[9 + i], NULL, check_row, &params[9 + i]);
    }
    for (int i = 0; i < 9; i++) {
        params[18 + i].thread_num = 19 + i;
        params[18 + i].index = i;
        pthread_create(&threads[18 + i], NULL, check_column, &params[18 + i]);
    }

    for (int i = 0; i < 27; i++)
        pthread_join(threads[i], NULL);

    // print results
    int valid = 1;
    for (int i = 0; i < 9; i++) {
        printf("Thread # %2d (subgrid %d) is %s\n", i + 1, i + 1,
               results[i] ? "valid" : "INVALID");
        if (!results[i]) valid = 0;
    }
    for (int i = 0; i < 9; i++) {
        printf("Thread # %2d (row %d) is %s\n", 10 + i, i + 1,
               results[9 + i] ? "valid" : "INVALID");
        if (!results[9 + i]) valid = 0;
    }
    for (int i = 0; i < 9; i++) {
        printf("Thread # %2d (column %d) is %s\n", 19 + i, i + 1,
               results[18 + i] ? "valid" : "INVALID");
        if (!results[18 + i]) valid = 0;
    }

    printf("%s contains a %s solution\n", argv[1],
           valid ? "valid" : "INVALID");

    return 0;
}
