#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
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
    }
    struct student_s *students = mmap(NULL, 64*1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    char *ptr = NULL;
    size_t n = 0;
    while (getline(&ptr, &n, stdin) > 0) {
        if (ptr[0] == 'w') {
            struct student_s *sptr = students;
            int rc;
            int id;
            char *name;
            printf("enter student:\n");
            while (scanf("%d %ms\n", &id, &name) > 0) {
                sptr->id = id;
                strcpy(sptr->name, name);
                sptr = (struct student_s *)((char *)sptr + sizeof(*sptr) + strlen(name) + 1);
            }
            printf("done entering students\n");
            sptr->id = 0;
        } else {
            struct student_s *sptr = students;
            while (sptr->id) {
                printf("%d %s\n", sptr->id, sptr->name);
                sptr = (struct student_s *)((char *)sptr + sizeof(*sptr) + strlen(sptr->name) + 1);
            }
        }
    }
}