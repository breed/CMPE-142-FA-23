#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct person {
    char *name;
    int duration;
};

char *name[] = {
    "abe",
    "amy",
    "cal",
    "carrie",
    "terry",
    "berry",
    "sherry",
    "ben",
    "jen",
    "ken",
};

pthread_mutex_t door;
pthread_cond_t bell;
volatile int club_count = 0;
int club_limit = 2;

void *enter_club(void *v)
{
    struct person *p = v;
    while (club_count >= club_limit);
    club_count++;
    printf("+ %s entered club\n", p->name);
    sleep(p->duration);
    printf("- %s left club after %d seconds\n", p->name, p->duration);
    club_count--;
}

int main()
{
    pthread_t ph[10];
    struct person people[10];

    pthread_mutex_init(&door, NULL);
    pthread_cond_init(&bell, NULL);
    for (int i = 0; i < 10; i++) {
        people[i].name = name[i];
        people[i].duration = 5 + (i%2);
    }

    for (int i = 0; i < 10; i++) {
        pthread_create(&ph[i], NULL, enter_club, &people[i]);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(ph[i], NULL);
    }
}
