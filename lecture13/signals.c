#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

// THIS CODE DOES NOT WORK!!!

void alarmed(int signum) {
    printf("got an alarm!!!");
}

int main(int argc, char **argv) {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("sleep", "sleep", "1000", NULL);
        perror("sleep");
        exit(2);
    }

    //signal(SIGALRM, alarmed);
    //alarm(2);
    int status;
    int rc = wait(&status);
    if (rc == -1) {
        perror("wait");
    }
    printf("%x\n", status);
}
