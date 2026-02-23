#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void section(const char *name) {
    printf("\n###\n# calculating %s\n###\n", name);
}

int send_and_receive(int p2c, int c2p, int v1, int v2, pid_t pid) {
    printf("parent (pid %d): sending %d to child\n", pid, v1);
    printf("parent (pid %d): sending %d to child\n", pid, v2);
    fflush(stdout);
    write(p2c, &v1, sizeof(int));
    write(p2c, &v2, sizeof(int));
    int result;
    read(c2p, &result, sizeof(int));
    printf("parent (pid %d): received %d from child\n", pid, result);
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <4-digit int> <4-digit int>\n", argv[0]);
        return 1;
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[2]);

    if (a < 1000 || a > 9999 || b < 1000 || b > 9999) {
        fprintf(stderr, "error: both arguments must be 4-digit integers (1000-9999).\n");
        return 1;
    }

    printf("your integers are %d %d\n", a, b);
    fflush(stdout);

    int a1 = a / 100, a2 = a % 100;
    int b1 = b / 100, b2 = b % 100;

    int p2c[2], c2p[2];

    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(p2c[1]);
        close(c2p[0]);
        pid_t my_pid = getpid();

        for (int i = 0; i < 4; i++) {
            int v1, v2;
            read(p2c[0], &v1, sizeof(int));
            printf("child (pid %d): received %d from parent\n", my_pid, v1);
            read(p2c[0], &v2, sizeof(int));
            printf("child (pid %d): received %d from parent\n", my_pid, v2);
            int product = v1 * v2;
            printf("child (pid %d): sending %d to parent\n", my_pid, product);
            fflush(stdout);
            write(c2p[1], &product, sizeof(int));
        }

        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    } else {
        close(p2c[0]);
        close(c2p[1]);
        pid_t my_pid = getpid();
        printf("parent (pid %d): created child (pid %d)\n", my_pid, pid);

        section("x");
        int A = send_and_receive(p2c[1], c2p[0], a1, b1, my_pid);
        int X = A * 10000;

        section("y");
        int C = send_and_receive(p2c[1], c2p[0], a1, b2, my_pid);
        int B = send_and_receive(p2c[1], c2p[0], a2, b1, my_pid);
        int Y = (B + C) * 100;

        section("z");
        int D = send_and_receive(p2c[1], c2p[0], a2, b2, my_pid);
        int Z = D;

        wait(NULL);
        close(p2c[1]);
        close(c2p[0]);

        printf("\n%d*%d == %d + %d + %d == %d\n", a, b, X, Y, Z, X + Y + Z);
    }

    return 0;
}
