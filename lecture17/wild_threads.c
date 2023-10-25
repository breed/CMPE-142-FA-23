#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// mythread()
//
// Simply adds 1 to counter repeatedly, in a loop
// No, this is not how you would add 10,000,000 to
// a counter, but it shows the problem nicely.
//
void *mythread(void *arg) {
    printf("begin thread %d\n", *(int*) arg);
    sleep(1);
    printf("end thread %d\n", *(int*) arg);
    return NULL;
}

// main()
//
// Just launches two threads (pthread_create)
// and then waits for them (pthread_join)
//
int main(int argc, char *argv[]) {
    pthread_t ph[50];
    for (int i = 0; i < 50; i++) {
        pthread_create(&ph[i], NULL, mythread, &i);
    }
    for (int i = 0; i < 50; i++) {
        pthread_join(ph[i], NULL);
    }
    return 0;
}
