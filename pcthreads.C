/* pcthreads.C */
#include <iostream>
using namespace std;
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

/* g++ -o pcthreads pcthreads.C -lpthread */

sem_t psem, csem; /* semaphores */
int n;
main()
{
    pthread_t idprod, idcons; /* ids of threads */
    void *producer(void *);
    void *consumer(void *);
    long loopcnt = 5;

    n = 0;
    if (sem_init(&csem, 0, 0) < 0) {
        perror("sem_init");
        exit(1);
    }
    if (sem_init(&psem, 0, 1) < 0) {
        perror("sem_init");
        exit(1);
    }
    if (pthread_create(&idprod, NULL, producer, (void *)loopcnt) != 0) {
        perror("pthread_create");
        exit(1);
    }
    if (pthread_create(&idcons, NULL, consumer, (void *)loopcnt) != 0) {
        perror("pthread_create");
        exit(1);
    }
    (void)pthread_join(idprod, NULL);
    (void)pthread_join(idcons, NULL);
    (void)sem_destroy(&psem);
    (void)sem_destroy(&csem);
}        

void *producer(void *arg)
{
    int i, loopcnt;
    loopcnt = (long)arg;
    for (i=0; i<loopcnt; i++) {
        sem_wait(&psem);
        n++;  /* increment n by 1 */
        sem_post(&csem);
    }
}       

void *consumer(void *arg)
{
    int i, loopcnt;
    loopcnt = (long)arg;
    for (i=0; i<loopcnt; i++) {
        sem_wait(&csem);
        cout << "n is " << n << "\n"; /* print value of n */
        sem_post(&psem);
    }
}
