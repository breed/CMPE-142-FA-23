#include <stdio.h>
#include <unistd.h>
#include <wait.h>

int main() {
    int a = 4;

    a++;
    switch (fork()) {
    case -1:
        perror("fork");
        break;
    case 0:
        a += 3;
        printf("i'm the child with a = %d @ %p\n", a, &a);
        sleep(5);
        printf("child out!\n");
        break;
    default:
        printf("i'm the parent waiting for the child to finish\n");
        wait(NULL);
        printf("child done. a is %d @ %p\n", a, &a);
    }
}
