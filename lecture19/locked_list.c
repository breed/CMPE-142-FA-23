#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ___node_t {
    int key;
    struct ___node_t *next;
} node_t;

typedef struct __list_t {
    node_t *head;
    pthread_mutex_t lock;
} list_t;

void List_Init(list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
}

int List_Lookup(list_t *L, int key) {
    for (node_t *curr = L->head; curr; curr = curr->next) {
        if (curr->key == key) return 0;
    }
    return -1;
}

void List_Insert(list_t *L, int key) {
    if (List_Lookup(L, key) == 0) return;
    node_t *new_node = malloc(sizeof *new_node);
    new_node->key = key;
    new_node->next = L->head;
    L->head = new_node;
}

void *test_list(void *v) {
    list_t *L = v;
    for (int i = 0; i < 10000; i++) {
        List_Insert(L, i);
    }
}

#define THREAD_COUNT 30

void main() {
    list_t L;
    List_Init(&L);
    test_list(&L);
    /*
    pthread_t threads[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, test_list, &L);
    }
    printf("Searching for 0: %d\n", List_Lookup(&L, 0));
    printf("Searching for 9000: %d\n", List_Lookup(&L, 900));
    printf("Searching for 90000: %d\n", List_Lookup(&L, 90000));
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    /* */
    int count = 0;
    for (node_t *curr = L.head; curr; curr = curr->next) count++;
    printf("total list size %d\n", count);
    printf("Searching for 0: %d\n", List_Lookup(&L, 0));
    printf("Searching for 9000: %d\n", List_Lookup(&L, 9000));
    printf("Searching for 90000: %d\n", List_Lookup(&L, 90000));
}


#define BUCKETS 101
typedef struct __hash_t {
    list_t buckets[BUCKETS];
} hash_t;

void Hash_init(hash_t *H) {
    for (int i = 0; i< BUCKETS; i++) List_Init(&H->buckets[i]);
}

void Hash_Insert(hash_t *H, int key) {
    List_Insert(&H->buckets[key%BUCKETS], key);
}

int Hash_Lookup(hash_t *H, int key) {
    return List_Lookup(&H->buckets[key%BUCKETS], key);
}

/* DON'T LOOK AT THIS...
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
*/
/* vim: set syntax= : */
