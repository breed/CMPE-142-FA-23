#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main() {
    char c;
    printf("Parent pid = %d\n", getpid());
    if (fork() == 0) {
        printf("Starting Taz pid = %d\n", getpid());
        printf("Taz finished\n");
        exit(1);
    }
    printf("Hit enter\n");
    read(0, &c, 1);
    if (fork() == 0) {
        printf("Sleeping child = %d\n", getpid());
        sleep(30);
        printf("Sleeping child one\n");
        exit(10);
    }
    if (fork() == 0) {
        printf("Busy child = %d\n", getpid());
        float prev = 0;
        for (float f = 1.8675309; f != prev; f += 1) prev = f;
        printf("Busy child ended with %f\n", prev);
        exit(20);
    }
    if (fork() == 0) {
        printf("Reading child = %d\n", getpid());
        while (1) read(0, &c, 1);
        printf("Reading child finished\n");
        exit(30);
    }
    printf("Waiting for children\n");
    int status;
    pid_t pid;
    while ((pid = wait(&status)) != -1) {
        if (WIFEXITED(status)) {
            printf("==> Process %d finished with exit code %d\n", pid, WEXITSTATUS(status));
        } else {
            printf("==> Process %d finished with signal %d\n", pid, WTERMSIG(status));
        }
    }
    perror("wait");
}
