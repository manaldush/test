#include "line.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

struct LogLine
{
    int size;
    char* referer;
    char* uri;
};

static bool isEndOfStr(char* l) {
    bool res = (*l == '\n' || *l == '\0');
    return res;
}

static char* getEntity(char** line, bool skip) {

    if (isEndOfStr(*line)) {
        return NULL;
    }
    char termChar = ' ';
    if (**line == '-') {
        (*line)++;
        if (!isEndOfStr(*line)) {
            // space expected, move ptr forward
            (*line)++;
        }
        return NULL;
    } else if (**line == '"') {
        termChar = '"';
        if (*((*line)+1) == '-' && *((*line)+2) == '"') {
            (*line)+=3;
            if (!isEndOfStr(*line)) {
                // space expected, move ptr forward
                (*line)++;
            }
            return NULL;
        } else {
            (*line)++;
        }
    } else if (**line == '[') {
        (*line)++;
        termChar = ']';
    }

    int len = 0;
    char* startPtr = *line;

    for(;(**line) != termChar && !isEndOfStr(*line);(*line)++, len++);

    if (termChar == '"' || termChar == ']') {
        (*line)++;
        if (!isEndOfStr(*line)) {
            // space expected, move ptr forward
            (*line)++;
        }
    } else {
        (*line)++;
    }

    if (skip) return NULL;

    if (len > 0) {
        char* result = calloc(len + 1, sizeof(char));
        for(int c = 0; c < len; c++) {
            result[c] = startPtr[c];
        }
        return result;
    } else {
        return NULL;
    }
}

struct LogLine* createLogLine(char* l) {
    getEntity(&l, true);
    getEntity(&l, true);
    getEntity(&l, true);
    getEntity(&l, true);

    //get uri
    char* requestLine = getEntity(&l, false);
    if (requestLine == NULL) {
        return NULL;
    }
    char* requestLineBuffer = requestLine;
    getEntity(&requestLine, true);
    char* uri = getEntity(&requestLine, false);
    free(requestLineBuffer);

    getEntity(&l, true);

    //get size
    int bytesNumber = 0;
    char* size = getEntity(&l, false);
    if (size != NULL) {
        char* zero = "0";
        if (strcmp(size, zero) != 0) {
            bytesNumber = atoi(size);
        }
        free(size);       
    }

    //get referer
    char* referer = getEntity(&l, false);

    if (referer == NULL && uri == NULL) {
        return NULL;
    }
    struct LogLine* logLine = malloc(sizeof(struct LogLine));

    logLine->referer = referer;
    logLine->size = bytesNumber;
    logLine->uri = uri;
    
    return logLine;
}

void destroyLogLine(struct LogLine* line) {
    if (line->referer != NULL) {
        free(line->referer);
    }
    if (line->uri != NULL) {
        free(line->uri);
    }
    free(line);
}

void printLogLine(struct LogLine* line) {
    if (line != NULL) {
        printf("Referer = %s, size = %d,  URI = %s\n", 
        line->referer == NULL ? "NULL" : line->referer,
        line->size,
        line->uri == NULL ? "NULL" : line->uri);
    }
}

char* getReferer(struct LogLine* line) {
    return line->referer;
}