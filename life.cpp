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
    bool msgGrid[MAXGRID][MAXGRID];
    int rowMin; //first val
    int rowMax; // second val
};

struct mailbox {
    struct msg messages;
    sem_t psem;
    sem_t csem;
};

struct gridData {
    bool evenGenGrid[MAXGRID][MAXGRID]; // [ROW][colmun]
    bool oddGenGrid[MAXGRID][MAXGRID];
    sem_t gsem;
};

//GLOBAL DATA
int row, column;
struct gridData GRIDS;
struct mailbox Mailboxes[MAXTHREAD + 1];
vector<pthread_t> threadIDs;

//PROTOTYPES
void sendMsg(int, struct msg&);
void reciveMsg(int, struct msg&);
void readGrid(const char*);
void printGrid(int); //0 for even, 1 for odd

int main(int argc, char **argv){
    /*** define runtime parameters ***/
    int threads, generations;
    bool PRINT, INPUT = false;
    if(atoi(argv[1]) > MAXTHREAD){
        threads = MAXTHREAD;
    }else if(atoi(argv[1]) <= 0){
        perror("invalid number of threads");
        exit(1);
    }else{
        threads = atoi(argv[1]);
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

    /*** read in grid from file ***/
    readGrid(argv[2]);

    return 0;
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
                    evenGenGrid[row][column] = false;
                    column += 1;
                }else if(line[i] == '1'){
                    evenGenGrid[row][column] = true;
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
                cout << oddGenGrid[i][j] << " ";
            }
        cout << "\n";
        }
    }else if(grid == 0){
        for(int i = 0; i<row;i++){
            for(int j = 0; j<column;j++){
                cout << evenGenGrid[i][j] << " ";
            }
            cout << "\n";
        }
    }
}

