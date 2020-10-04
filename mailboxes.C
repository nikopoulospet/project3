/* mailboxes.C */
/* g++ -o mailboxes mailboxes.C -lpthread */
#include <iostream>
using namespace std;
#include <pthread.h>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXTHREAD 10
#define RANGE 1
#define ALLDONE 2

/* structs */
struct msg {
    int iSender; // message sender (0 .. num)
    int type; // type of message 
    int value1; //first val
    int value2; // second val
};

struct mailbox {
    struct msg message;
    sem_t sem;
};

/* globals */
struct mailbox Mailboxes[MAXTHREAD + 1];
vector<pthread_t> threadIDs;

/* protoypes */
void sendMsg(int, struct msg), reciveMsg(int, struct msg);

int main(int argc, char **argv){
    int threads;
    if (atoi(argv[1]) > MAXTHREAD){
        threads = MAXTHREAD;
    }else{
        threads = atoi(argv[1]);
    }
    /* init semaphores */
    for(int i = 0; i<= threads; i++){
        if (sem_init(&Mailboxes[i].sem,0,1) != 0){
            perror("sem_init");
            exit(1);
        }
    }
    
    /* creation of threads */ 
    void *newThread(void *);
    void *addEm(void *);
    for(int i = 1; i <= threads; i++){
        pthread_t aThread;
        const char *temp = to_string(i).c_str(); // this hurts me but it works part 1
        threadIDs.push_back(aThread);
        if (pthread_create(&aThread, NULL, newThread, (void *)temp) != 0) {
	        perror("pthread_create");
	        exit(1);    
        }
        usleep(500000);
    }   
    
}

void *addEm(void *args){
    
}

void *newThread(void *arg)
{
    int Name = atoi(static_cast<const char*>(arg)); // this hurts me but it works part 2
    cout << "my name is " << Name << "\n";
    cout.flush();
}   

void sendMsg(int iTo, struct msg &Msg){
    while(sem_wait(&Mailboxes[iTo].sem) != 0);
    Mailboxes[iTo].message = Msg;
    if (sem_post(&Mailboxes[iTo].sem) != 0){
        perror("sem_post fail");
        exit(1);
    }
}

void reciveMsg(int iFrom, struct msg &Msg){
    while(sem_wait(&Mailboxes[iFrom].sem) != 0);
    Msg = Mailboxes[iFrom].message;
    if (sem_post(&Mailboxes[iFrom].sem) != 0){
        perror("sem_post fail");
        exit(1);
    }
}