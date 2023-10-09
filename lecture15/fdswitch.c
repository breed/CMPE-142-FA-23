#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
    printf("hello\n");
    int fd = open("out.txt", O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        perror("out.txt");
        exit(2);
    }
    int oldout = dup(1);
    dup2(fd, 1);
    printf("hello2\n");
    dup2(oldout, 1);
    printf("hello3\n");
}
