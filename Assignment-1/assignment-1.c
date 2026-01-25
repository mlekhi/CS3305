#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static bool parse_int(const char *s, int *out) {
    char *end = NULL;
    long val;

    if (s == NULL || *s == '\0') {
        return false;
    }

    errno = 0;
    val = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        return false;
    }
    if (val < INT_MIN || val > INT_MAX) {
        return false;
    }

    *out = (int)val;
    return true;
}

static bool is_prime(int n) {
    int i;

    if (n < 2) {
        return false;
    }
    if (n == 2) {
        return true;
    }
    if (n % 2 == 0) {
        return false;
    }

    for (i = 3; i <= n / i; i += 2) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

static void count_primes_in_range(int start, int end, int *count, long long *sum) {
    int i;
    int local_count = 0;
    long long local_sum = 0;

    for (i = start; i <= end; i++) {
        if (is_prime(i)) {
            local_count++;
            local_sum += i;
        }
    }

    *count = local_count;
    *sum = local_sum;
}

static void print_result(int start, int end, int count, long long sum) {
    pid_t pid = getpid();
    pid_t ppid = getppid();

    printf("pid: %d, ppid %d - count and sum of primes between %d and %d are %d and %lld\n",
           (int)pid, (int)ppid, start, end, count, sum);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    int mode;
    int min;
    int max;
    int range;
    int q1;
    int q2;
    int q3;
    int starts[4];
    int ends[4];
    int i;

    if (argc < 4) {
        fprintf(stderr, "error: expected 3 params: mode min max\n");
        return 1;
    }

    if (!parse_int(argv[1], &mode) || !parse_int(argv[2], &min) || !parse_int(argv[3], &max)) {
        fprintf(stderr, "error: params must be integers\n");
        return 1;
    }

    if (max <= min) {
        fprintf(stderr, "error: max must be > min\n");
        return 1;
    }

    range = max - min;
    q1 = min + (range / 4);
    q2 = min + (range / 2);
    q3 = min + ((range * 3) / 4);

    starts[0] = min;
    ends[0] = q1;
    starts[1] = q1;
    ends[1] = q2;
    starts[2] = q2;
    ends[2] = q3;
    starts[3] = q3;
    ends[3] = max;

    printf("process id: %d\n", (int)getpid());
    fflush(stdout);

    if (mode == 0) {
        for (i = 0; i < 4; i++) {
            int count = 0;
            long long sum = 0;

            count_primes_in_range(starts[i], ends[i], &count, &sum);
            print_result(starts[i], ends[i], count, sum);
        }
        return 0;
    }

    for (i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            int count = 0;
            long long sum = 0;

            count_primes_in_range(starts[i], ends[i], &count, &sum);
            print_result(starts[i], ends[i], count, sum);
            return 0;
        }
    }

    for (i = 0; i < 4; i++) {
        (void)wait(NULL);
    }

    return 0;
}

