#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stack.h"
#include "file.h"
#include "line.h"
#include "referers.h"

#define LINE_MAX_SIZE 1024

typedef struct DirInfo
{
    DIR* dir;
    const char* dirName;
} DirInfo;

typedef struct WorkerInfo {
    int threadId;
    struct Referers* referers;
} WorkerInfo;

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
    WorkerInfo* workerInfo = calloc(threadNumber, sizeof(threadNumber));
    int startedWorkerThreadNumber = 0;
    for(;startedWorkerThreadNumber < threadNumber; ) {
        workerInfo[startedWorkerThreadNumber] = (WorkerInfo) { .threadId = (startedWorkerThreadNumber), .referers = initReferers()};
        if(thrd_create(&workerThreads[startedWorkerThreadNumber], runWorkerThread, (workerInfo + startedWorkerThreadNumber)) != thrd_success) {
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
        fprintf(stdout, "Reader thread can't be joined\n");
        thrd_detach(readerThread);
        status = EXIT_FAILURE;
        goto workersCloseLabel;
    } else {
        fprintf(stdout, "Reader thread was stopped\n");
    }


    workersCloseLabel:
    stopSignal = true;
    struct Referers* referers = initReferers();
    while(startedWorkerThreadNumber > 0) {
        int id = startedWorkerThreadNumber-1;
        if (thrd_join(workerThreads[id], NULL) != thrd_success) {
            fprintf(stderr, "Worker thread can't be joined\n");
            thrd_detach(workerThreads[startedWorkerThreadNumber-1]);
            status = EXIT_FAILURE;
        } else {
            printf("Worker thread %d has been stopped\n", id);
            mergeReferers(referers, workerInfo[id].referers);
            destroyReferers(workerInfo[id].referers);
        }
        startedWorkerThreadNumber--;
    }
    printTopReferers(referers);
    destroyReferers(referers);
    free(workerThreads);
    free(workerInfo);

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
    while (1)
    {
        char* line = calloc(LINE_MAX_SIZE, sizeof(char));
        if (readLine(f, line, LINE_MAX_SIZE) == NULL) {
            break;
        }
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
    printf("Reader Thread is stopping\n");
    mtx_lock(&mutex);
    stopSignal = true;
    cnd_broadcast(&condition);
    mtx_unlock(&mutex);

    return 0;
}

int runWorkerThread(void* arg) {
    WorkerInfo* workerInfo = ((WorkerInfo*)arg);
    int id = workerInfo->threadId;
    printf("Worker thread %d is started\n", id);
    int linesCounter = 0;
    struct Referers* referers = workerInfo->referers;
    for(;;) {
        struct LineEntry* e;
        mtx_lock(&mutex);
        while ((e = pop(linesStack)) == NULL) {
            if (stopSignal) {
                mtx_unlock(&mutex);
                goto exitLabel;
            } else {
                cnd_wait(&condition, &mutex);
            }
        }
        mtx_unlock(&mutex);
        //printLineEntry(e);
        struct LogLine* logLine =  createLogLine(getLine(e));
        if (logLine != NULL) {
            putReferer(referers, getReferer(logLine), 1);
            destroyLogLine(logLine);
        }
        destroyLine(e);
        linesCounter++;
    }

    exitLabel:
    printf("Worker thread %d is stopping, processed lines = %d\n", id, linesCounter);
    return EXIT_SUCCESS;
}