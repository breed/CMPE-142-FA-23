#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

struct student_s {
    int id;
    char name[];
};
int main(int argc, char **argv) {
    int fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror(argv[1]);
        exit(2);
    }
    struct student_s *students = mmap(NULL, 64*1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (students == MAP_FAILED) {
        perror(argv[1]);
        exit(2);
    }

    while (1) {
        struct student_s *sptr = students;
        while (sptr->id) {
            printf("%d %s\n", sptr->id, sptr->name);
            sptr = (struct student_s *)((char *)sptr + sizeof(*sptr) + strlen(sptr->name) + 1);
        }
        sleep(5);
        printf("------\n");
    }
}
