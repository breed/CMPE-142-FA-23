#include <stdio.h>
#include <unistd.h>

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
    if (write(fds[1], &i, sizeof i) != sizeof i) {
        perror("write");
    }
    if (read(fds[0], &j, sizeof j) != sizeof j) {
        perror("read");
    }
    printf("j = %d\n", j);
}