#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>

const char *writeout = "writing to stdout\n";
const char *writeerr = "writing to stderr\n";
const char *write3 = "writing to 3\n";

void perror_write(int fd, const char *str) {
    if (write(fd, str, strlen(str)) != strlen(str)) {
        char *msg;
        asprintf(&msg, "write to %d", fd);
        perror(msg);
    }
}

int main() {
    printf("printing to stdout\n");
    fflush(stdout);
    fprintf(stderr, "printing to stdout\n");
    fflush(stderr);
    // much danger in mixing buffered io and write!!!
    // dup2(1,3);
    perror_write(1, writeout);
    perror_write(2, writeerr);
    perror_write(3, write3);
}