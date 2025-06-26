#include "logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <execinfo.h>

#define BT_BUF_SIZE 100
#define INIT_LINE_SIZE 64

static const char *loggerLevelsStr[] = {"UNKNOWN", "TRACE", "DEBUG", "INFO", "WARN", "ERROR"};

static __thread void *buffer[BT_BUF_SIZE];

enum LoggerStatus {
    STOPPED,
    STARTED
};
typedef enum LoggerStatus LoggerStatus;

typedef struct MessageEntry MessageEntry;

struct MessageEntry
{
    char *message;
    LoggerLevel level;
    MessageEntry *next;
    MessageEntry *previous;
    char** stackTrace;
    int nptr;
    char* curTime;
};

struct Messages
{
    MessageEntry *top;
    MessageEntry *bottom;
};
typedef struct Messages Messages;

typedef struct LoggerContext
{
    FILE *file;
    Messages *messages;
    LoggerStatus status;
    LoggerLevel loggerLevel;
    pthread_cond_t *condition;
    pthread_mutex_t *mutex;
} LoggerContext;

struct Logger
{
    pthread_t *loggerThr;
    LoggerContext *ctx;
};

char* copyString(char* src)
{
    int size = INIT_LINE_SIZE;
    for(int i = 1; i <= 3; i++) {
        char* dest = malloc(sizeof(char) * size);
        for(int ii = 0; ii < size; ii++) {
            if ((*(dest + ii) = *(src + ii)) == '\0') {
                return dest;
            }
        }
        free(dest);
        size *= 2;
    }


    return NULL;
}

LoggerLevel str2LoggerLevel(char* s) {
    for (int i = TRACE; i <= ERROR; i++) {
        if (strcmp(s, loggerLevelsStr[i]) == 0) {
            return i;
        }
    }

    return UNKNOWN;
}

static char* curTime() {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char* timeStr = malloc(64 * sizeof(char));
    if (timeStr == NULL) {
        fprintf(stderr, "Malloc error during printing message\n");
        return NULL;
    }
    if(strftime(timeStr, 64, "%c", tm) == 0) {
        free(timeStr);
        fprintf(stderr, "Malloc error during printing message\n");
    }

    return timeStr;
}

void *runLoggerThread(void *arg);

Logger *initLogger(char *path, LoggerLevel level)
{
    Logger *logger = NULL;
    FILE *file = fopen(path, "a+");
    if (file == NULL)
    {
        printf("File name %s wasn't opened, the error message is: %s\n", path, strerror(errno));
        goto returnPoint;
    }
    Messages *messages = malloc(sizeof(Messages));
    if (messages == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        goto freeFile;
    }
    pthread_t *loggerThr = malloc(sizeof(pthread_t));
    if (loggerThr == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        goto freeMessages;
    }
    pthread_cond_t *condition = malloc(sizeof(pthread_cond_t));
    if (condition == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        goto freeLoggerThr;
    }
    if (0 != pthread_cond_init(condition, NULL))
    {
        goto freeLoggerCond;
    }
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        goto freeLoggerCond;
    }
    if (0 != pthread_mutex_init(mutex, NULL))
    {
        goto freeLoggerMutex;
    }

    LoggerContext *context = malloc(sizeof(LoggerContext));
    if (context == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        goto freeContext;
    }
    context->file = file;
    context->messages = messages;
    context->messages->bottom = NULL;
    context->messages->top = NULL;
    context->status = STOPPED;
    context->condition = condition;
    context->mutex = mutex;
    context->loggerLevel = level;

    logger = malloc(sizeof(Logger));
    if (logger == NULL)
    {
        fprintf(stderr, "Malloc error during logger initialization\n");
        goto freeLoggerMutex;
    }

    if (pthread_create(loggerThr, NULL, runLoggerThread, context) != 0)
    {
        fprintf(stderr, "Worker thread can't be created\n");
        goto freeLogger;
    }

    pthread_mutex_lock(context->mutex);
    context->status = STARTED;
    pthread_cond_signal(context->condition);
    pthread_mutex_unlock(context->mutex);

    logger->ctx = context;
    logger->loggerThr = loggerThr;
    goto returnPoint;

freeLogger:
    free(logger);
freeContext:
    free(context);
freeLoggerMutex:
    free(mutex);
freeLoggerCond:
    free(condition);
freeLoggerThr:
    free(loggerThr);
freeMessages:
    free(messages);
freeFile:
    fclose(file);

returnPoint:
    return logger;
}

void *runLoggerThread(void *arg)
{
    LoggerContext *context = ((LoggerContext *)arg);
    pthread_mutex_lock(context->mutex);
initializationStep:
    if (context->status != STARTED) {
        pthread_cond_wait(context->condition, context->mutex);
        goto initializationStep;
    }
    MessageEntry *entry = NULL;
nextIteration:
    entry = context->messages->top;
    if (entry == NULL)
    {
        if (context->status != STARTED)
        {
            goto exitLabel;
        }
        pthread_cond_wait(context->condition, context->mutex);
        goto nextIteration;
    }
    else
    {
        context->messages->top = entry->next;
        if (context->messages->top == context->messages->bottom && context->messages->top != NULL)
        {
            context->messages->bottom = NULL;
        }
        pthread_mutex_unlock(context->mutex);
        fprintf(context->file, "%s : %s - %s\n", entry->curTime, *(loggerLevelsStr + entry->level), entry->message);
        if (entry->nptr > 0) {
            for (int j = 0; j < entry->nptr; j++) {
                fprintf(context->file, "    %s\n", entry->stackTrace[j]);
            }
        }
        free(entry->message);
        if (entry->stackTrace != NULL) {
            free(entry->stackTrace);
        }
        free(entry->curTime);
        free(entry);
        goto nextIteration;
    }

exitLabel:
    return NULL;
}

void closeLogger(Logger *logger)
{
    pthread_mutex_lock(logger->ctx->mutex);
    logger->ctx->status = STOPPED;
    pthread_cond_signal(logger->ctx->condition);
    pthread_mutex_unlock(logger->ctx->mutex);
    pthread_join(*(logger->loggerThr), NULL);

    free(logger->loggerThr);
    fclose(logger->ctx->file);
    free(logger->ctx->mutex);
    free(logger->ctx->condition);
    free(logger->ctx->messages);
    free(logger->ctx);
    free(logger);
}

static void logMessage(Logger *logger, char *s, LoggerLevel level)
{
    if (logger->ctx->loggerLevel <= level)
    {
        MessageEntry *entry = malloc(sizeof(MessageEntry));
        if (entry == NULL)
        {
            fprintf(stderr, "Malloc error during printing message\n");
            return;
        }
        entry->message = copyString(s);
        if (entry->message == NULL) {
            free(entry);
            fprintf(stderr, "Malloc error during copy log string\n");
            return;
        }
        entry->stackTrace = NULL;
        entry->nptr = 0;
        if (level == ERROR) {
            char** stackTrace = malloc(sizeof(char*) * BT_BUF_SIZE);
            if (stackTrace == NULL) {
                fprintf(stderr, "Malloc error during printing message\n");
                free(entry->message);
                free(entry);
                return;
            }
            int nptr = backtrace(buffer, BT_BUF_SIZE);
            stackTrace = backtrace_symbols(buffer, nptr);
            entry->stackTrace = stackTrace;
            entry->nptr = nptr;
        }
        entry->level = level;
        entry->next = NULL;
        entry->curTime = curTime();
        pthread_mutex_lock(logger->ctx->mutex);
        if (logger->ctx->messages->bottom != NULL)
        {
            logger->ctx->messages->bottom->next = entry;
            logger->ctx->messages->bottom = entry;
        }
        else
        {
            if (logger->ctx->messages->top == NULL)
            {
                logger->ctx->messages->top = entry;
            }
            else
            {
                logger->ctx->messages->bottom = entry;
                logger->ctx->messages->top->next = entry;
            }
        }

        pthread_mutex_unlock(logger->ctx->mutex);
    }
}

void trace(Logger *logger, char *s)
{
    logMessage(logger, s, TRACE);
}

void debug(Logger *logger, char *s)
{
    logMessage(logger, s, DEBUG);
}

void info(Logger *logger, char *s)
{
    logMessage(logger, s, INFO);
}

void warn(Logger *logger, char *s)
{
    logMessage(logger, s, WARN);
}

void error(Logger *logger, char *s)
{
    logMessage(logger, s, ERROR);
}