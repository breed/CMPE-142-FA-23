#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

//
// Created by bcr33d on 9/11/23.
//
int main() {
    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe");
    }
    int i = 4;
    int j = 0;
    if (fork() == 0) {
        i += 17;
        printf("child set i to %d\n", i);
        if (write(fds[1], &i, sizeof i) != sizeof i) {
            perror("write");
        }
        exit(0);
    }
    wait(NULL);
    printf("child finished\n");
    /*
    int x = 17;
    if (write(fds[1], &x, sizeof x) != -1) {
        perror("write");
    }
     */
    close(fds[1]);
    if (read(fds[0], &j, sizeof j) != sizeof j) {
        perror("read");
    }
    printf("i = %d\n", i);
    printf("j = %d\n", j);
    printf("doing read a second time\n");
    j = 99;
    if (read(fds[0], &j, sizeof j) != sizeof j) {
        perror("read");
    }
    printf("j = %d\n", j);
}