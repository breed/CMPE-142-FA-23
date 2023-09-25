//
// Created by bcr33d on 9/25/23.
//

#include <stdio.h>

int foo() {
    int j = 27;
    int i;
    printf("i @%p is %d\n", &i, i);
    i = i < 0 || i > 100 ? i = 0 : i + 1;
    return j+i;
}

int goo() {
    int j;
    int i = 17;
    printf("j @%p is %d\n", &j, j);
    j = j < 0 || j > 100 ? j = 0 : j + 1;
    return j+i;
}

int main(int argc, char **argv) {
    foo();
    goo();
    foo();
    foo();
    foo();
    goo();
}