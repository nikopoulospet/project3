/* By Peter Nikopoulos */
/* g++ -o p1 part1.cpp -lpthread */
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
#define DEFAULT 100

/* structs */
struct msg {
    int iSender; // message sender (0 .. num)
    int type; // type of message 
    int value1; //first val
    int value2; // second val
};

struct mailbox {
    vector<struct msg> messages;
    sem_t sem;
};

/* globals */
//vector<struct mailbox> Mailboxes[MAXTHREAD+1];
struct mailbox Mailboxes[MAXTHREAD + 1]; // change this to add a queue for each mailbox using a linked list
vector<pthread_t> threadIDs;

/* protoypes */
void sendMsg(int, struct msg&);
int reciveMsg(int, struct msg&);

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
    /* create ranges */
    /* default range is 1 - 100 */
    int max;
    if(argc < 3){
        max = 100;
    }else{
        max = atoi(argv[2]);
    }
    int inc = (int)max/threads;
    int prev = 0;
    /* send messages of range to mailboxes */
    for(int i = 1; i<= threads; i++){
        struct msg *message = (struct msg*)malloc(sizeof(struct msg));
        message->iSender = 0;
        message->type = RANGE;
        message->value1 = prev + 1;
        message->value2 = prev + inc;
        if(i==threads){
            message->value2 = max;
        }
        prev += inc;
        sendMsg(i,*message);
        //cout << "message sent " << i << " \n";
        //cout.flush();
        free(message);
    }
    
    /* creation of threads */ 
    void *newThread(void *);
    void *addEm(void *);
    for(long i = 1; i <= threads; i++){
        pthread_t aThread;
        threadIDs.push_back(aThread);
        if (pthread_create(&aThread, NULL, addEm, (void *)i) != 0) {
	        perror("pthread_create");
	        exit(1);    
        }

        //usleep(50000);
    }

    int sum = 0;
    for(int i = 1; i <=threads; i++){
        struct msg *message;
        *message = {-1,0,0,0};//(struct msg*)malloc(sizeof(struct msg));
        //cout << i;
        //cout.flush();
        while(reciveMsg(0,*message) != 0);
        sum += message->value1;
        //cout << " " << message->value1 << "\n";
        (void *)pthread_join(threadIDs[i-1],NULL);
        //free(message);
    }
    cout << sum << "\n";
}

void *addEm(void *arg){
    int Name;
    Name = (long)arg;
    //cout << "adding w/ thread num:  " << Name << "\n";
    //cout.flush();

    struct msg *message = (struct msg*)malloc(sizeof(struct msg));
    *message = {-1,0,0,0};
    reciveMsg(Name,*message);
    if (message->iSender != -1){
        //cout << message->value1 << " " << message->value2 << "\n";
    }

    int sum = 0;
    for (int i = message->value1; i <= message->value2; i++){
        sum += i;
        //cout << sum << " " << i <<"\n";
    }

    *message = {Name,ALLDONE,sum,0};
    sendMsg(0,*message);

    free(message);
}

void *newThread(void *arg)
{
    int Name;
    Name = (long)arg;
    cout << "my name is " << Name << "\n";
    cout.flush();
}

void sendMsg(int iTo, struct msg &Msg){
    while(sem_wait(&Mailboxes[iTo].sem) != 0);
    Mailboxes[iTo].messages.push_back(Msg);
    if (sem_post(&Mailboxes[iTo].sem) != 0){
        perror("sem_post fail");
        exit(1);
    }
}

int reciveMsg(int iFrom, struct msg &Msg){
    int val = -1;
    while(sem_wait(&Mailboxes[iFrom].sem) != 0);
    if (Mailboxes[iFrom].messages.size() != 0){
        Msg = Mailboxes[iFrom].messages.back();
        Mailboxes[iFrom].messages.pop_back();
        val = 0;
    }
    if (sem_post(&Mailboxes[iFrom].sem) != 0){
        perror("sem_post fail");
        exit(1);
    }
    return val;
}