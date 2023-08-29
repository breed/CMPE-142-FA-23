#include <stdio.h>

int main() {
    char *ptr = NULL;
    size_t n = 0;
    getline(&ptr, &n, stdin);
    printf("%s", ptr);
    getline(&ptr, &n, stdin);
    printf("%s", ptr);
}