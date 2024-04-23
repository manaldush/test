#include <string.h>
#include "statistic.h"

struct RefererEntry {
    const char* referer;
    uint64_t counter;
    struct RefererEntry* next;
};

struct Referers
{
    struct RefererEntry* top;
};

uint64_t putReferer(struct Referers* referers, const char* referer) {
    for(struct RefererEntry* curEntry = referers->top; curEntry != NULL ;curEntry = curEntry->next) {
        if (strcmp(referer, curEntry->referer) == 0) {
            curEntry->counter++;
            return curEntry->counter;
        }
    }

    struct RefererEntry* newEntry = (struct RefererEntry*) malloc(sizeof(struct RefererEntry));
    newEntry->next = referers->top;
    newEntry->referer = referer;
    newEntry->counter = 1;

    referers->top = newEntry;
    return newEntry->counter;
}

void putUrl(struct Urls* urls, const char* entry) {
    
}