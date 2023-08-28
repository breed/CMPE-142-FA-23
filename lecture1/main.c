#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        char *endptr;
        printf("Hello, World! %d %s %ld\t", i, argv[i], strtol(argv[i], &endptr, 0));
        printf("::: %p %p %c\n", argv[i], endptr, *endptr);
    }
    return 0;
}
