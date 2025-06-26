#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stack.h"
#include "file.h"
#include "line.h"
#include "referers.h"
#include <time.h>

#define LINE_MAX_SIZE 1024
#define REFERERS_BUCKET_SIZE 200

typedef struct DirInfo
{
    DIR* dir;
    const char* dirName;
} DirInfo;

typedef struct WorkerInfo {
    int threadId;
    struct Referers* referers;
    struct Referers* uris;
} WorkerInfo;

struct LinesStack* linesStack;
pthread_cond_t condition;
pthread_mutex_t mutex;
volatile bool stopSignal = false;

void* runReaderThread(void* arg);
void* runWorkerThread(void* arg);
void destroyWorkersInfo(WorkerInfo** w, int l);
void printCurrentTime();
static struct Referers* merge(struct Referers** referers, int size);

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

    printCurrentTime();

    linesStack = initStack();
    if (linesStack == NULL)
        goto dirCloseLabel;

    if (0 != pthread_cond_init(&condition, NULL)) {
        goto freeStack;
    }

    if (0 != pthread_mutex_init(&mutex, NULL)) {
        goto closeCondition;
    }

    //start worker threads
    pthread_t* workerThreads = malloc(threadNumber * sizeof(pthread_t));
    WorkerInfo** workerInfos = calloc(threadNumber, sizeof(WorkerInfo*));
    int startedWorkerThreadNumber = 0;
    for(;startedWorkerThreadNumber < threadNumber; ) {
        WorkerInfo* workerInfo = calloc(1, sizeof(WorkerInfo));
        workerInfo->threadId = startedWorkerThreadNumber;
        workerInfo->referers = initReferers();
        workerInfo->uris = initReferers();
        workerInfos[startedWorkerThreadNumber] = workerInfo;
        if(pthread_create(&workerThreads[startedWorkerThreadNumber], NULL, runWorkerThread, workerInfo) != 0) {
            fprintf(stderr, "Worker thread can't be created\n");
            status = EXIT_FAILURE;
            goto workersCloseLabel;
        }
        startedWorkerThreadNumber++;
    }

    // start reader thread
    pthread_t readerThread;
    if(pthread_create(&readerThread, NULL, runReaderThread, &dirInfo) != 0) {
        fprintf(stderr, "Reader thread can't be created\n");
        status = EXIT_FAILURE;
        goto workersCloseLabel;
    }

    if (pthread_join(readerThread, NULL) != 0) {
        fprintf(stdout, "Reader thread can't be joined\n");
        pthread_detach(readerThread);
        status = EXIT_FAILURE;
        goto workersCloseLabel;
    }


    workersCloseLabel:
    stopSignal = true;
    while(startedWorkerThreadNumber > 0) {
        WorkerInfo* workerInfo = workerInfos[startedWorkerThreadNumber-1];
        if (pthread_join(workerThreads[workerInfo->threadId], NULL) != 0) {
            fprintf(stderr, "Worker thread can't be joined\n");
            status = EXIT_FAILURE;
        }
        startedWorkerThreadNumber--;
    }

    if (status != EXIT_FAILURE) {
        // referers
        struct Referers** referers = calloc(threadNumber, sizeof(struct Referers*));
        if (referers != NULL) {
            for(int i = 0; i < threadNumber; i++) {
                referers[i] = workerInfos[i]->referers;
            }
            if (referers == NULL) {
                fprintf(stderr, "Statistic for referers can't be merged\n");
                status = EXIT_FAILURE;
            } else {
                struct Referers* referer = merge(referers, threadNumber);
                if (referer != NULL) {
                    printf("Print top 10 referers\n");
                    printTopReferers(referer);
                }
            }
            free(referers);
        }

        //uris
        struct Referers** uris = calloc(threadNumber, sizeof(struct Referers*));
        if (uris != NULL) {
            for(int i = 0; i < threadNumber; i++) {
                uris[i] = workerInfos[i]->uris;
            }
            if (uris == NULL) {
                fprintf(stderr, "Statistic for referers can't be merged\n");
                status = EXIT_FAILURE;
            } else {
                struct Referers* uri = merge(uris, threadNumber);
                if (uri != NULL) {
                    printf("Print top 10 URIs\n");
                    printTopReferers(uri);
                }
            }

            free(uris);
        }
    }

    free(workerThreads);
    destroyWorkersInfo(workerInfos, threadNumber);

    printCurrentTime();

    pthread_mutex_destroy(&mutex);
    closeCondition:
    pthread_cond_destroy(&condition);
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
        pthread_mutex_lock(&mutex);
        if ((isPushed = push(linesStack, line))) {
            pthread_cond_signal(&condition);
        }
        pthread_mutex_unlock(&mutex);
        if (!isPushed) {
            nanosleep((const struct timespec[]){{0, 10000L}}, NULL);
            goto repeatPush;
        }
    }
    
    closeFile(f);
}

void* runReaderThread(void* arg) {
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
    pthread_mutex_lock(&mutex);
    stopSignal = true;
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void* runWorkerThread(void* arg) {
    WorkerInfo* workerInfo = ((WorkerInfo*)arg);
    struct Referers* referers = workerInfo->referers;
    struct Referers* curReferers = NULL;
    struct Referers* uris = workerInfo->uris;
    struct Referers* curUris = NULL;
    for(;;) {
        struct LineEntry* e;
        pthread_mutex_lock(&mutex);
        while ((e = pop(linesStack)) == NULL) {
            if (stopSignal) {
                pthread_mutex_unlock(&mutex);
                goto exitLabel;
            } else {
                pthread_cond_wait(&condition, &mutex);
            }
        }
        pthread_mutex_unlock(&mutex);
        //printLineEntry(e);
        struct LogLine* logLine =  createLogLine(getLine(e));
        if (logLine != NULL) {
            //referers
            if (curReferers == NULL) {
                curReferers = initReferers();
            }
            putReferer(curReferers, getReferer(logLine), 1);
            if (getReferersSize(curReferers) > REFERERS_BUCKET_SIZE) {
                mergeReferers(referers, curReferers);
                destroyReferers(curReferers);
                curReferers = NULL;
            }

            //uris
            if (curUris == NULL) {
                curUris = initReferers();
            }
            putReferer(curUris, getUri(logLine), getSize(logLine));
            if (getReferersSize(curUris) > REFERERS_BUCKET_SIZE) {
                mergeReferers(uris, curUris);
                destroyReferers(curUris);
                curUris = NULL;
            }
            destroyLogLine(logLine);
        }
        destroyLine(e);
    }

    exitLabel:
    if (curReferers != NULL) {
        mergeReferers(referers, curReferers);
        destroyReferers(curReferers);
    }
    return NULL;
}

void destroyWorkersInfo(WorkerInfo** w, int l) {
    if (l > 0) {
        for (int i = 0; i < l; i++)
        {
            WorkerInfo* wRef = *(w + i);
            destroyReferers(wRef->referers);
            free(wRef);
        }
    }
    free(w);
}

void printCurrentTime() {
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "Current local time and date: %s\n", asctime (timeinfo) );
}

void* runMergerThread(void* arg) {
    struct Referers** pair = (struct Referers**) arg;
    mergeReferers(pair[0], pair[1]);
    free(pair);
    return NULL;
}

static struct Referers* merge(struct Referers** referers, int size) {
    if (size == 1) {
        return *referers;
    }
    struct Referers* result = referers[0];
    bool b = ((size % 2) == 0);
    int threadsNumber = b ? (size / 2) : (size - 1) / 2;
    int nSize = b ? (size / 2) : (size - 1) / 2 + 1;
    pthread_t* mergerThreads = calloc(threadsNumber, sizeof(pthread_t));
    if (mergerThreads == NULL) {
        return NULL;
    }
    struct Referers** nReferers = calloc(nSize, sizeof(struct Referers*));
    if (nReferers == NULL) {
        free(mergerThreads);
        return NULL;
    }
    int createdThrNumber = 0;
    for(int thrNumber = 0; thrNumber < threadsNumber; thrNumber++) {
        struct Referers** pair = calloc(2, sizeof(struct Referers*));
        if (pair == NULL) {
            result = NULL;
            break;
        }
        pair[0] = referers[thrNumber * 2];
        pair[1] = referers[thrNumber * 2 + 1];
        nReferers[thrNumber] = pair[0];
        if(pthread_create(&mergerThreads[thrNumber], NULL, runMergerThread, pair) != 0) {
            printf ( "Merge thread start error\n");
            result = NULL;
            break;
        }
        createdThrNumber++;
    }
    if (result != NULL && !b) {
        nReferers[nSize - 1] = referers[size - 1];
    }

    for(int thrNumber = 0; thrNumber < createdThrNumber; thrNumber++) {
        if (pthread_join(mergerThreads[thrNumber], NULL) != 0) {
            printf("Merger thread can't be joined\n");
            result = NULL;
        }
    }

    if (result != NULL) {
        result = merge(nReferers, nSize);
    }

    free(nReferers);
    free(mergerThreads);
    
    return result;
}