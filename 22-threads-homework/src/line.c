#include "lines.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
    char* l = *line;
    if (isEndOfStr(l)) {
        return NULL;
    }
    char termChar = ' ';
    if (*l == '-') {
        l++;
        if (!isEndOfStr(l)) {
            // space expected, move ptr forward
            l++;
        }
        return NULL;
    } else if (*l == '"') {
        termChar = '"';
        if (*(l+1) == '-' && *(l+2) == '"') {
            l+=3;
            if (!isEndOfStr(l)) {
                // space expected, move ptr forward
                l++;
            }
            return NULL;
        } else {
            l++;
        }
    } else if (*l == '[') {
        l++;
    }

    int len = 0;
    char* startPtr = l;

    for(;*l != termChar && !isEndOfStr(l);l++, len++);

    if (termChar == '"' || termChar == ']') {
        l++;
        if (!isEndOfStr(l)) {
            // space expected, move ptr forward
            l++;
        }
    } else {
        l++;
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
    getEntity(&l, true);
    char* uri = getEntity(&l, true);
    free(requestLine);

    getEntity(&l, true);

    //get size
    char* size = getEntity(&l, false);
    if (size == NULL) {
        return NULL;
    }
    char* zero = "0";
    int bytesNumber = 0;
    if (strcmp(size, zero) != 0) {
        bytesNumber = atoi(size);
        if (bytesNumber == 0) {
            //parse error
            free(size);
            return NULL;
        }
    }
    free(size);

    //get referer
    char* referer = getEntity(&l, false);

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
