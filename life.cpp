/* By Peter Nikopoulos */
/* g++ -o life life.cpp -lpthread */
#include <iostream>
using namespace std;
#include <pthread.h>
#include <cstdlib>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>

// PARAMETERS
#define MAXGRID 40
#define MAXTHREAD 10
#define RANGE 1
#define ALLDONE 2
#define GO 3
#define GENDONE 4
#define UNCHANGED 5
#define ALLDEAD 6

//STRUCTURES
struct msg {
    int iSender; // message sender (0 .. num)
    int type; // type of message 
    int rowMin; //first val
    int rowMax; // second val
    int generation;
};

struct mailbox {
    struct msg messages;
    sem_t psem;
    sem_t csem;
};
/*
struct gridData {
    bool evenGenGrid[MAXGRID][MAXGRID]; // [ROW][colmun]
    bool oddGenGrid[MAXGRID][MAXGRID];
};*/

//GLOBAL DATA
int row, column;
//struct gridData GRIDS;
bool GRID[1][MAXGRID][MAXGRID];
struct mailbox Mailboxes[MAXTHREAD + 1];
vector<pthread_t> threadIDs;

//PROTOTYPES
void sendMsg(int, struct msg&);
void reciveMsg(int, struct msg&);
void readGrid(const char*);
void printGrid(int); //0 for even, 1 for odd
void initSemaphores(int);
void calcGrid(int,int,int);

int main(int argc, char **argv){
    /*** define runtime parameters ***/
    int threads, generations;
    bool PRINT, INPUT = false;
    
    readGrid(argv[2]);

    if(atoi(argv[1]) > MAXTHREAD){
        threads = MAXTHREAD;
    }else if(atoi(argv[1]) <= 0){
        perror("invalid number of threads");
        exit(1);
    }else{
        threads = atoi(argv[1]);
    }
    if(threads > row){
        threads = row;
    }

    if (atoi(argv[3]) < 0){
        perror("invalid number of generations");
        exit(1);
    }else{
        generations = atoi(argv[3]);
    }
    if(argc >= 5){
        if(argv[4] == "y"){
            PRINT = true;
        } 
    }
    if(argc >= 6){
        if(argv[4] == "y"){
            INPUT = true;
        } 
    }
    /*** end param block ***/

    /*** start other stuff ***/
    initSemaphores(threads);

    /* create threads & send messages */
    
    void *thread(void *);
    for (long i = 1; i <= threads; i++){
        cout << "create :" << i << "\n";
        pthread_t threadID;
        threadIDs.push_back(threadID);
        if (pthread_create(&threadID, NULL, thread, (void *)i) != 0) {
	        perror("pthread_create");
	        exit(1);    
        }
    }

    /* calc and send range messages */
    int rowsPerThread, start = 0;
    rowsPerThread = (int)row/threads;
    for(int i = 1; i <= threads; i++){
        struct msg *message = (struct msg*)malloc(sizeof(struct msg));
        message->rowMin = start;
        message->rowMax = start + (rowsPerThread-1);
        if(i == threads){
            message->rowMax = row;
        }
        message->generation = generations;
        start += rowsPerThread;
        sendMsg(i,*message);
        free(message);
        usleep(50000);
    }

    for(int i = 0; i<1; i++){
        //pthread_join(threadIDs[i-1],NULL);
    }
    return 0;
}

void *thread(void *arg){
    int Name, startRow, endRow, gen, genMax;
    Name = (long)arg;

    struct msg *message = (struct msg*)malloc(sizeof(struct msg));
    reciveMsg(Name, *message);
    startRow = message->rowMin;
    endRow = message->rowMax;
    genMax = message->generation;
    cout << startRow << " " << endRow << "\n";
    for(int gen = 1; gen <= genMax; gen++){
        reciveMsg(Name, *message);
        calcGrid(startRow,endRow,gen);
        message->iSender = Name;
        message->type = GENDONE;
        message->generation = gen;
        sendMsg(0,*message);
    }

}
void calcGrid(int start, int end, int gen){

}

void sendMsg(int iTo, struct msg &MSG){
    sem_wait(&Mailboxes[iTo].psem);
    Mailboxes[iTo].messages = MSG;
    sem_post(&Mailboxes[iTo].csem);
}

void reciveMsg(int iFrom, struct msg &MSG){
    sem_wait(&Mailboxes[iFrom].csem);
    MSG = Mailboxes[iFrom].messages;
    sem_post(&Mailboxes[iFrom].psem);
}

void readGrid(const char* file){
    row = 0;
    string line;
    ifstream myfile(file);
    if (myfile){
        while (getline( myfile, line)){
            column = 0;
            for(int i = 0; i <= line.length(); i++){
                if(line[i] == '0'){
                    GRIDS.evenGenGrid[row][column] = false;
                    column += 1;
                }else if(line[i] == '1'){
                    GRIDS.evenGenGrid[row][column] = true;
                    column += 1;
                }
            }
            row += 1;
        }
    myfile.close();
    }
  else cout << "GRID READ FAILED\n";
}

void printGrid(int grid){
    if(grid == 1){
        for(int i = 0; i<row;i++){
            for(int j = 0; j<column;j++){
                cout << GRIDS.oddGenGrid[i][j] << " ";
            }
        cout << "\n";
        }
    }else if(grid == 0){
        for(int i = 0; i<row;i++){
            for(int j = 0; j<column;j++){
                cout << GRIDS.evenGenGrid[i][j] << " ";
            }
            cout << "\n";
        }
    }
}

void initSemaphores(int threads){
    /*if (sem_init(&GRIDS.gsem,0,1) != 0){
        perror("gsem_init");
        exit(1);
    }*/
    for(int i = 0; i<= threads; i++){
        if (sem_init(&Mailboxes[i].psem,0,1) != 0){
            perror("sem_init");
            exit(1);
        }
        if (sem_init(&Mailboxes[i].csem,0,0) != 0){
            perror("sem_init");
            exit(1);
        }
    }
}