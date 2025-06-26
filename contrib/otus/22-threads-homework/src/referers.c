#include <string.h>
#include <stdlib.h>
#include "referers.h"
#include <stdio.h>

struct RefererEntry {
    char* referer;
    uint64_t counter;
    struct RefererEntry* next;
    struct RefererEntry* prev;
};

struct Referers
{
    struct RefererEntry* top;
    struct RefererEntry* bottom;
    int size;
};

struct Referers* initReferers() {
    return calloc(1, sizeof(struct Referers));
}

void putReferer(struct Referers* referers, const char* referer, uint64_t c) {
    if (referer == NULL || c == 0){
        return;
    }
    for(struct RefererEntry* curEntry = referers->top; curEntry != NULL ;curEntry = curEntry->next) {
        if (strcmp(referer, curEntry->referer) == 0) {
            curEntry->counter+=c;
            return;
        }
    }

    struct RefererEntry* newEntry = (struct RefererEntry*) calloc(1, sizeof(struct RefererEntry));
    newEntry->referer = strcpy(calloc(strlen(referer) + 1, sizeof(char)), referer);
    newEntry->counter = c;
    newEntry->next = NULL;
    newEntry->prev = NULL;
    if (referers->bottom != NULL) {
        referers->bottom->next = newEntry;
        newEntry->prev = referers->bottom;
        referers->bottom = newEntry;
    } else {
        referers->bottom = newEntry;
        referers->top = newEntry;
    }
    referers->size++;
    return;
}

void mergeReferers(struct Referers* destination, struct Referers* referers) {
    for(struct RefererEntry* curEntry = referers->top; curEntry != NULL ;curEntry = curEntry->next) {
        putReferer(destination, curEntry->referer, curEntry->counter);
    }
}

void destroyReferers(struct Referers* referers) {
    for(struct RefererEntry* curEntry = referers->top; curEntry != NULL ;) {
        free(curEntry->referer);
        struct RefererEntry* e = curEntry;
        curEntry = curEntry->next;
        free(e);
    }
}

static int compareReferers (const void * elem1, const void * elem2) {
    struct RefererEntry* ref1 = *((struct RefererEntry**) elem1);
    struct RefererEntry* ref2 = *((struct RefererEntry**) elem2);
    if (ref1->counter < ref2->counter) return  1;
    if (ref1->counter > ref2->counter) return  -1;
    return 0;
}

void printTopReferers(struct Referers* referers) {
    struct RefererEntry** entries = calloc(referers->size, sizeof(struct ReferersEntry*));
    int ii = 0;
    for (struct RefererEntry* re = referers->top ; re != NULL; re = re->next)
    {
        entries[ii] = re;
        ii++;
    }
    qsort(entries, referers->size, sizeof(struct RefererEntry*), compareReferers);
    for (int i = 0; i < 10 && i < referers->size; i++)
    {
        struct RefererEntry* e = *(entries + i);
        printf("%d) %s, count = %ld\n", i + 1, e->referer, e->counter);
    }
    free(entries);
}

int getReferersSize(struct Referers* referers) {
    return referers->size;
}
