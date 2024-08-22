#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include <pthread.h>

typedef struct ThreadContext ThreadContext;
struct ThreadContext {
    Logger* logger;
    char* threadName;
};

static void releaseExecutor(pthread_t *loggerThr, ThreadContext* context) {
    if (loggerThr != NULL) {
        pthread_join(*(loggerThr), NULL);
        free(loggerThr);
    }
    if (context != NULL) {
        free(context);
    }
}

static void *executorThread(void *arg) {
    ThreadContext *context = ((ThreadContext *)arg);
    Logger* logger = context->logger;
    char* threadName = context->threadName;
    for(int i = 0; i <= 10000; i++) {
        int type = i % 5;
        char dest[32];
        switch (type) {
            case 0:
                sprintf(dest, "%s: trace message %d", threadName, i);
                trace(logger, dest);
                break;
            case 1:
                sprintf(dest, "%s: debug message %d", threadName, i);
                debug(logger, dest);
                break;
            case 2:
                sprintf(dest, "%s: info message %d", threadName, i);
                info(logger, dest);
                break;
            case 3:
                sprintf(dest, "%s: warn message %d", threadName, i);
                warn(logger, dest);
                break;
            case 4:
                sprintf(dest, "%s: error message %d", threadName, i);
                error(logger, dest);
                break;
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 3) {
       fprintf(stderr, "Input parameters are missed\n");
       return EXIT_FAILURE;
    }

    int result = EXIT_SUCCESS;
    char* path = argv[1];
    LoggerLevel loggerLevel = str2LoggerLevel(argv[2]);
    pthread_t *loggerThr2 = NULL;
    pthread_t *loggerThr1 = NULL;
    ThreadContext* context1 = NULL;
    ThreadContext* context2 = NULL;
    if (loggerLevel == UNKNOWN) {
        fprintf(stderr, "Input parameter log level has illegal value\n");
        return EXIT_FAILURE;
    }
    Logger* logger = initLogger(path, loggerLevel);
    if (logger == NULL) {
        fprintf(stderr, "Logger wasn't initialized correctly\n");
        return EXIT_FAILURE;
    } else {
        fprintf(stdout, "Logger was initialized correctly\n");
    }

    context1 = malloc(sizeof(ThreadContext));
    if (context1 == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        result = EXIT_FAILURE;
        goto releasePoint;
    } else {
        context1->logger = logger;
        context1->threadName = "thread-1";
    }

    context2 = malloc(sizeof(ThreadContext));
    if (context2 == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        result = EXIT_FAILURE;
        goto releasePoint;
    } else {
        context2->logger = logger;
        context2->threadName = "thread-2";
    }

    loggerThr1 = malloc(sizeof(pthread_t));
    if (loggerThr1 == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        result = EXIT_FAILURE;
        goto releasePoint;
    }

    loggerThr2 = malloc(sizeof(pthread_t));
    if (loggerThr2 == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        result = EXIT_FAILURE;
        goto releasePoint;
    }

    if (pthread_create(loggerThr1, NULL, executorThread, context1) != 0)
    {
        fprintf(stderr, "Executor thread can't be created\n");
        result = EXIT_FAILURE;
        goto releasePoint;
    }

    if (pthread_create(loggerThr2, NULL, executorThread, context2) != 0)
    {
        fprintf(stderr, "Executor thread can't be created\n");
        result = EXIT_FAILURE;
        goto releasePoint;
    }

releasePoint:
    releaseExecutor(loggerThr1, context1);
    releaseExecutor(loggerThr2, context2);

    if (logger != NULL) {
        info(logger, "Stop logger");
        closeLogger(logger);
    }
    fprintf(stdout, "Logger was closed\n");
    return result;
}