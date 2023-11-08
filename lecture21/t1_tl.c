#include <stdio.h>
#include <pthread.h>
#include <threads.h>

static thread_local int counter = 0;

// mythread()
//
// Simply adds 1 to counter repeatedly, in a loop
// No, this is not how you would add 10,000,000 to
// a counter, but it shows the problem nicely.
//
void *mythread(void *arg) {
    printf("%s: begin\n", (char *) arg);
    int i;
    for (i = 0; i < 1e7; i++) {
        counter++;
    }
    printf("%s: done\n", (char *) arg);
    return &counter;
}

// main()
//
// Just launches two threads (pthread_create)
// and then waits for them (pthread_join)
//
int main(int argc, char *argv[]) {
    pthread_t p1, p2;
    printf("main: begin (counter = %d)\n", counter);

    pthread_create(&p1, NULL, mythread, "A");
    pthread_create(&p2, NULL, mythread, "B");

    int *r1, *r2;
    // join waits for the threads to finish
    pthread_join(p1, (void**)&r1);
    pthread_join(p2, (void**)&r2);

    printf("r1 = %p, r2 = %p, counter = %p\n", r1, r2, &counter);

    counter = *r1 + *r2;
    printf("main: done with both (counter = %d)\n", counter);
    return 0;
}
