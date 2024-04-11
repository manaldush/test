#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct DirInfo
{
    DIR* dir;
    const char* dirName;
} DirInfo;


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

    //start worker threads
    thrd_t* workerThreads = malloc(threadNumber * sizeof(thrd_t));
    int startedWorkerThreadNumber = 0;
    for(;startedWorkerThreadNumber < threadNumber; startedWorkerThreadNumber++) {
        if(thrd_create(&workerThreads[startedWorkerThreadNumber], runWorkerThread, &dirInfo) != thrd_success) {
            fprintf(stderr, "Worker thread can't be created\n");
            status = EXIT_FAILURE;
            goto workersCloseLabel;
        }
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
        fprintf(stderr, "Reader thread can't be joined\n");
        status = EXIT_FAILURE;
        goto workersCloseLabel;
    }


    workersCloseLabel:
    while(workerThreads < 0) {
        if (thrd_join(readerThread, &readerThreadResult) != thrd_success) {
            thrd_detach(readerThread);
            fprintf(stderr, "Worker thread can't be joined\n");
            status = EXIT_FAILURE;
            workerThreads--;
        }
    }
    dirCloseLabel:
    closedir(dirInfo.dir);
    exitLabel:
    return status;
}

void readLogFile(const char* fileName) {
    printf("File name: %s\n", fileName);
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

    return 0;
}

int runWorkerThread(void* arg) {
}