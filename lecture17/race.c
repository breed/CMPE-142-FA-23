#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECKPOINTS 5
#define RACERS 5

// yep! there is a race condition here!
volatile int checkpoints = 0;

void *do_race(void *arg)
{
    char *racer = arg;
    printf("racer %s started\n", racer);
    for (int i = 0; i < CHECKPOINTS; i++) {
        sleep(2);
        if (checkpoints++ % RACERS == 0) printf("-----\n");
        printf("%d: %s\n", i, racer);
    }
    sleep(2);
    printf("racer %s finished\n", racer);
    return NULL;
}

int main()
{
    pthread_t threads[RACERS];

    for (int i = 0; i < RACERS; i++) {
        char name[10];
        sprintf(name, "racer%d", i);
        pthread_create(&threads[i], NULL, do_race, strdup(name));
    }
    for (int i = 0; i < RACERS; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
