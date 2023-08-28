#include <stdio.h>

int main() {
    char ptr[10];
    size_t n = 10;
    getline(&ptr, &n, stdin);
    printf("%s\n", ptr);
    getline(&ptr, &n, stdin);
    printf("%s\n", ptr);
}