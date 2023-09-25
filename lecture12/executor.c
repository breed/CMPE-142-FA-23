//
// Created by bcr33d on 9/25/23.
//
#include <stdio.h>
#include <unistd.h>

void main(int argc, char **argv) {
    printf("executing %s\n", argv[0]);
    int rc = execvp(argv[1], argv+1);
    if (rc == -1) {
        perror(argv[1]);
    }
    printf("finished execing %s rc = %d\n", argv[1], rc);
}