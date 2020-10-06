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
#define DIE 7
#define LIVE 8

//STRUCTURES
struct msg
{
    int iSender; // message sender (0 .. num)
    int type;    // type of message
    int rowMin;  //first val
    int rowMax;  // second val
    int generation;
};

struct mailbox
{
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
int GRID[1][MAXGRID][MAXGRID];
struct mailbox Mailboxes[MAXTHREAD + 1];
vector<pthread_t> threadIDs;

//PROTOTYPES
void sendMsg(int, struct msg &);
void reciveMsg(int, struct msg &);
void readGrid(const char *);
void printGrid(int); //0 for even, 1 for odd
void initSemaphores(int);
void calcGrid(int, int, int);
void sendType(int, int);
void revive(int);
int neighbors(int, int, int);
void waitforInput(bool);
void ifPrint(bool, int);
int checkExitEarly(int, int, int);

int main(int argc, char **argv)
{
    /*** define runtime parameters ***/
    int threads, generations;
    bool PRINT, INPUT = false;

    readGrid(argv[2]);

    if (atoi(argv[1]) > MAXTHREAD)
    {
        threads = MAXTHREAD;
    }
    else if (atoi(argv[1]) <= 0)
    {
        perror("invalid number of threads");
        exit(1);
    }
    else
    {
        threads = atoi(argv[1]);
    }
    if (threads > row)
    {
        threads = row;
    }

    if (atoi(argv[3]) < 0)
    {
        perror("invalid number of generations");
        exit(1);
    }
    else
    {
        generations = atoi(argv[3]);
    }
    if (argc >= 5)
    {
        if (strcmp(argv[4], "y") == 0)
        {
            PRINT = true;
        }
    }
    if (argc >= 6)
    {
        if (strcmp(argv[5], "y") == 0)
        {
            INPUT = true;
        }
    }
    cout << PRINT << "\n";
    /*** end param block ***/

    /*** start other stuff ***/
    initSemaphores(threads);

    /* create threads & send messages */

    void *thread(void *);
    for (long i = 1; i <= threads; i++)
    {
        pthread_t threadID;
        threadIDs.push_back(threadID);
        if (pthread_create(&threadID, NULL, thread, (void *)i) != 0)
        {
            perror("pthread_create");
            exit(1);
        }
    }

    /* calc and send range messages */
    int rowsPerThread, start = 0;
    rowsPerThread = (int)row / threads;
    for (int i = 1; i <= threads; i++)
    {
        struct msg *message = (struct msg *)malloc(sizeof(struct msg));
        message->rowMin = start;
        message->rowMax = start + (rowsPerThread - 1);
        if (i == threads)
        {
            message->rowMax = row;
        }
        message->generation = generations;
        start += rowsPerThread;
        sendMsg(i, *message);
        free(message);
    }

    vector<int> pendingDeath;
    bool exit = false;
    bool DEAD = false;
    bool STATIC = false;
    int g = 1;
    while (!exit)
    {
        cout.flush();
        cout << "current GEN" << g - 1 << "\n";
        ifPrint(PRINT, g - 1);
        waitforInput(INPUT);

        sendType(threads, GO);

        DEAD = true;
        STATIC = true;
        struct msg *message = (struct msg *)malloc(sizeof(struct msg));
        for (int i = 1; i <= threads; i++)
        {
            reciveMsg(0, *message);
            //add end early logic here
            cout << message->type << "\n";
            bool addThis = false;
            if (DEAD)
            {
                DEAD = message->type == ALLDEAD;
                if(DEAD)addThis = true; //cout << "thread is DEAD: " << i << "\n";
                if(DEAD)cout << "thread is DEAD: " << i << "\n";
            }
            if (STATIC)
            {
                STATIC = message->type == UNCHANGED;
                if(STATIC)addThis = true; //cout << "thread is STATIC: " << i << "\n";
                if(STATIC)cout << "thread is STATIC: " << i << "\n";
            }
            if(message->type == ALLDEAD || message->type == UNCHANGED) addThis = true;
            if (addThis)
            {
                pendingDeath.push_back(i);
            }
        }

        free(message);

        if (g == generations)
        {
            exit = true;
            cout << "last generation at GEN " << g << "\n";
            ifPrint(PRINT, g);
        }
        else if (DEAD)
        {
            exit = true;
            cout << "All cells dead at GEN " << g << "\n";
            ifPrint(PRINT, g);
            sendType(threads, DIE);
        }
        else if (STATIC)
        {
            exit = true;
            cout << "ALL cells are unchanged at GEN " << g << "\n";
            ifPrint(PRINT, g);
            sendType(threads, DIE);
        }
        else
        {
            for (int i = pendingDeath.size(); i > 0; i--)
            {
                revive(pendingDeath.back());
                cout << "thread is revived: " << pendingDeath.back() << "\n";
                pendingDeath.pop_back();
            }
        }
        g++;
    }

    struct msg *message = (struct msg *)malloc(sizeof(struct msg));
    for (int i = 1; i <= threads; i++)
    {
        reciveMsg(0, *message);
    }
    free(message);
    return 0;
}

void *thread(void *arg)
{
    int Name, startRow, endRow, gen, genMax, check;
    Name = (long)arg;
    bool DONE = false;
    gen = 1;

    struct msg *message = (struct msg *)malloc(sizeof(struct msg));
    reciveMsg(Name, *message);
    startRow = message->rowMin;
    endRow = message->rowMax;
    genMax = message->generation;
    while (!DONE)
    {
        reciveMsg(Name, *message);
        calcGrid(startRow, endRow, gen);
        check = checkExitEarly(startRow, endRow, gen);
        cout << "check val" <<check << "\n";
        if (check == 0)
        {
            message->type = GENDONE;
        }
        else if (check == 1)
        {
            message->type = UNCHANGED;
            DONE = true;
        }
        else if (check == 2)
        {
            message->type = ALLDEAD;
            DONE = true;
        }
        message->iSender = Name;
        message->generation = gen;
        sendMsg(0, *message);
        if (DONE)
        {
            reciveMsg(Name, *message);
            DONE = message->type == DIE;
        }
        gen++;
        if (gen > genMax)
            DONE = true;
    }

    message->type = ALLDONE;
    sendMsg(0, *message);
}

int checkExitEarly(int startRow, int endRow, int gen)
{
    bool DEAD, STATIC = true;
    int x = 0;
    if (gen % 2 != 0)
        x = 1;

    for (int i = startRow; i <= endRow; i++)
    {
        for (int j = 0; j <= column; j++)
        {
            if (STATIC)
            {
                STATIC = GRID[0][i][j] == GRID[1][i][j];
            }
            if (DEAD)
            {
                DEAD = GRID[x][i][j] == 0;
            }
        }
    }

    if (STATIC)
    {
        return 1;
    }
    else if (DEAD)
    {
        return 2;
    }
    else
    {
        return 0;
    }
}

void calcGrid(int startRow, int endRow, int gen)
{
    int x = 0;
    int near;
    if (gen % 2 != 0)
        x = 1;
    //printGrid(0);
    //cout << "gird 0^\n" << "calcing" << x << "\n";
    //printGrid(1);
    for (int i = startRow; i <= endRow; i++)
    { // row
        for (int j = 0; j <= column; j++)
        { // column
            near = neighbors(i, j, !x);
            //cout << near << " " << i << " " << j << "\n";
            if (GRID[!x][i][j] == 1)
            {
                if (near != 2 && near != 3)
                {
                    GRID[x][i][j] = 0;
                }
                else
                {
                    GRID[x][i][j] = 1;
                }
            }
            else
            {
                if (near == 3)
                {
                    GRID[x][i][j] = 1;
                }
                else
                {
                    GRID[x][i][j] = 0;
                }
            }
        }
    }
    //cout << "end\n";
    //printGrid(0);
    //cout << "gird 0^\n" << "calcing" << x << "\n";
    //printGrid(1);
}

int neighbors(int rowpos, int columnpos, int gridnum)
{
    int neighbors = 0;
    for (int i = -1; i <= 1; i++)
    {
        if ((i + rowpos) >= 0 || (i + rowpos) <= row)
        {
            for (int j = -1; j <= 1; j++)
            {
                if ((j + columnpos) >= 0 || (j + columnpos) <= column)
                {
                    neighbors += GRID[gridnum][rowpos + i][columnpos + j];
                }
                if (j == 0 && i == 0)
                {
                    neighbors += (-1) * GRID[gridnum][rowpos + i][columnpos + j];
                }
            }
        }
    }
    //cout << "N" << neighbors << "\n";
    return neighbors;
}

void sendType(int threads, int type)
{
    for (int i = 1; i <= threads; i++)
    {
        struct msg *message = (struct msg *)malloc(sizeof(struct msg));
        message->iSender = 0;
        message->type = type;
        sendMsg(i, *message);
        free(message);
    }
}

void revive(int thread)
{
    struct msg *message = (struct msg *)malloc(sizeof(struct msg));
    message->iSender = 0;
    message->type = LIVE;
    sendMsg(thread, *message);
    free(message);
}

void sendMsg(int iTo, struct msg &MSG)
{
    sem_wait(&Mailboxes[iTo].psem);
    Mailboxes[iTo].messages = MSG;
    sem_post(&Mailboxes[iTo].csem);
}

void reciveMsg(int iFrom, struct msg &MSG)
{
    sem_wait(&Mailboxes[iFrom].csem);
    MSG = Mailboxes[iFrom].messages;
    sem_post(&Mailboxes[iFrom].psem);
}

void readGrid(const char *file)
{
    row = 0;
    string line;
    ifstream myfile(file);
    if (myfile)
    {
        while (getline(myfile, line))
        {
            column = 0;
            for (int i = 0; i <= line.length(); i++)
            {
                if (line[i] == '0')
                {
                    GRID[0][row][column] = 0;
                    column += 1;
                }
                else if (line[i] == '1')
                {
                    GRID[0][row][column] = 1;
                    column += 1;
                }
            }
            row += 1;
        }
        myfile.close();
    }
    else
        cout << "GRID READ FAILED\n";
}

void ifPrint(bool enabled, int grid)
{
    if (enabled)
    {
        printGrid(grid);
    }
}

void waitforInput(bool enabled)
{
    if (enabled)
    {
        cout << "press enter to contiue\n";
        cin.get();
    }
}

void printGrid(int grid)
{
    int x;
    if (grid % 2 == 0)
    {
        x = 0;
    }
    else
    {
        x = 1;
    }
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            cout << GRID[x][i][j] << " ";
        }
        cout << "\n";
    }
}

void printGrid2(int grid)
{
    for (int x = 0; x < 2; x++)
    {
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < column; j++)
            {
                cout << GRID[x][i][j] << " ";
            }
            cout << "\n";
        }
    }
}

void initSemaphores(int threads)
{
    /*if (sem_init(&GRIDS.gsem,0,1) != 0){
        perror("gsem_init");
        exit(1);
    }*/
    for (int i = 0; i <= threads; i++)
    {
        if (sem_init(&Mailboxes[i].psem, 0, 1) != 0)
        {
            perror("sem_init");
            exit(1);
        }
        if (sem_init(&Mailboxes[i].csem, 0, 0) != 0)
        {
            perror("sem_init");
            exit(1);
        }
    }
}