#include <stdio.h>
#include <pthread.h>
#include <threads.h>

static int a_counter = 0;
static int b_counter = 0;

pthread_mutex_t a, b;

void transfer_to_a() {
    b_counter--;
    a_counter++;
}

void transfer_to_b() {
    a_counter--;
    b_counter++;
}

void *mythread(void *arg) {
    printf("%s: begin\n", (char *) arg);
    int i;
    for (i = 0; i < 1e7; i++) {
        if (i & 1) {
            transfer_to_b();
        } else {
            transfer_to_a();
        }
    }
    printf("%s: done\n", (char *) arg);
    return NULL;
}

// main()
//
// Just launches two threads (pthread_create)
// and then waits for them (pthread_join)
//
int main(int argc, char *argv[]) {
    pthread_t p1, p2;
    pthread_mutex_init(&a, NULL);
    pthread_mutex_init(&b, NULL);
    printf("main: begin %d %d\n", a_counter, b_counter);

    pthread_create(&p1, NULL, mythread, "A");
    pthread_create(&p2, NULL, mythread, "B");

    // join waits for the threads to finish
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    printf("main: done with both %d %d\n", a_counter, b_counter);
    return 0;
}
