#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stack.h"
#include "file.h"

#define LINE_MAX_SIZE 2000

typedef struct DirInfo
{
    DIR* dir;
    const char* dirName;
} DirInfo;

struct LinesStack* linesStack;
cnd_t condition;
mtx_t mutex;
volatile bool stopSignal = false;

int runReaderThread(void* arg);
int runWorkerThread(void* arg);

int main(int argc, char** argv) {
    DirInfo dirInfo;
    int status = EXIT_SUCCESS;
    if (argc == 1) {
        fprintf(stderr, "Input parameters missed\n");
        status = EXIT_FAILURE;
        goto exitLabel;
    } else if (argc == 2) {
        fprintf(stderr, "Input parameter number of threads is missed\n");
        status = EXIT_FAILURE;
        goto exitLabel;
    }

    dirInfo.dirName = argv[1];
    printf("Input dir name is: %s\n", dirInfo.dirName);

    // get dir pointer
    dirInfo.dir = opendir(dirInfo.dirName);

    if (dirInfo.dir == NULL) {
        status = EXIT_FAILURE;
        fprintf(stderr, "Log file directory wasn't found\n");
        goto exitLabel;
    }

    const char* threadNumAsStr = argv[2];
    int threadNumber;
    if (strcmp("0", threadNumAsStr) == 0 || (threadNumber = atoi(threadNumAsStr)) <= 0) {
        fprintf(stderr, "Input parameter number of threads has invalid value\n");
        status = EXIT_FAILURE;
        goto dirCloseLabel;
    }

    linesStack = initStack();
    if (linesStack == NULL)
        goto dirCloseLabel;

    if (thrd_success != cnd_init(&condition)) {
        goto freeStack;
    }

    if (thrd_success != mtx_init(&mutex, mtx_plain)) {
        goto closeCondition;
    }

    //start worker threads
    thrd_t* workerThreads = malloc(threadNumber * sizeof(thrd_t));
    int* threadId = malloc(threadNumber * sizeof(int));
    int startedWorkerThreadNumber = 0;
    for(;startedWorkerThreadNumber < threadNumber; ) {
        threadId[startedWorkerThreadNumber] = startedWorkerThreadNumber;
        if(thrd_create(&workerThreads[startedWorkerThreadNumber], runWorkerThread, (threadId + startedWorkerThreadNumber)) != thrd_success) {
            fprintf(stderr, "Worker thread can't be created\n");
            status = EXIT_FAILURE;
            goto workersCloseLabel;
        }
        startedWorkerThreadNumber++;
    }

    // start reader thread
    thrd_t readerThread;
    int readerThreadResult;
    if(thrd_create(&readerThread, runReaderThread, &dirInfo) != thrd_success) {
        fprintf(stderr, "Reader thread can't be created\n");
        status = EXIT_FAILURE;
        goto workersCloseLabel;
    }

    if (thrd_join(readerThread, &readerThreadResult) != thrd_success) {
        thrd_detach(readerThread);
        fprintf(stdout, "Reader thread can't be joined\n");
        status = EXIT_FAILURE;
        goto workersCloseLabel;
    } else {
        fprintf(stdout, "Reader thread was stopped\n");
    }


    workersCloseLabel:
    stopSignal = true;
    while(startedWorkerThreadNumber > 0) {
        if (thrd_join(workerThreads[startedWorkerThreadNumber-1], (threadId + startedWorkerThreadNumber-1)) != thrd_success) {
            thrd_detach(readerThread);
            fprintf(stderr, "Worker thread can't be joined\n");
            status = EXIT_FAILURE;
        }
        startedWorkerThreadNumber--;
    }
    free(workerThreads);
    free(threadId);

    mtx_destroy(&mutex);
    fprintf(stdout, "Mutex destroyed\n");
    closeCondition:
    cnd_destroy(&condition);
    freeStack:
    destroyStack(linesStack);
    dirCloseLabel:
    closedir(dirInfo.dir);
    exitLabel:
    fprintf(stdout, "The end\n");
    return status;
}

void readLogFile(const char* fileName) {
    printf("File name: %s\n", fileName);

    FILE* f = openFile(fileName);
    if (f == NULL) {
        return;
    }
    char* line = calloc(LINE_MAX_SIZE, sizeof(char));
    while (readLine(f, line, LINE_MAX_SIZE) != NULL)
    {
        bool isPushed = false;
        repeatPush:
        mtx_lock(&mutex);
        if ((isPushed = push(linesStack, line))) {
            cnd_signal(&condition);
        }
        mtx_unlock(&mutex);
        if (!isPushed) {
            thrd_sleep(&(struct timespec){.tv_nsec=1000}, NULL);
            goto repeatPush;
        }
    }
    
    closeFile(f);
}

int runReaderThread(void* arg) {
    DirInfo* dir = (DirInfo*) arg;
    struct dirent *de;

    while ((de = readdir(dir->dir)) != NULL) {
        struct stat eStat;
        char* path = calloc(strlen(dir->dirName) + 1 + strlen(de->d_name) + 1, 1);
        strcpy(path, dir->dirName);
        strcat(path, "/");
        strcat(path, de->d_name);
        if(stat(path, &eStat) == 0 && S_ISREG(eStat.st_mode)) {
            readLogFile(path);
        }
        free(path);
    }
    stopSignal = true;

    return 0;
}

int runWorkerThread(void* arg) {
    int id = *((int*)arg);
    printf("Thread %d is started\n", id);
    for(;;) {
        struct LineEntry* e;
        if (mtx_timedlock(&mutex, &(struct timespec){.tv_nsec=1000})) {
            while ((e = pop(linesStack)) == NULL) {
                cnd_wait(&condition, &mutex);
            }
            mtx_unlock(&mutex);
        } else if (stopSignal) {
            break;
        }
        //TODO process entry
        //printLineEntry(e);
    }

    printf("Thread %d is stopping\n", id);

    return EXIT_SUCCESS;
}