#include "stack.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

struct LineEntry {
    char* line;
    struct LineEntry* next;
};

struct LinesStack {
    struct LineEntry* top;
    int length;
};

bool push(struct LinesStack* q, char* l) {
    if (q->length < 1000) {
        struct LineEntry* nLine = (struct LineEntry*) malloc(sizeof(struct LineEntry));
        nLine->line = l;
        nLine->next = q->top;
        q->top = nLine;
        q->length++;
        return true;
    }
    return false;
}

struct LineEntry* pop(struct LinesStack* q) {
    if(q->length > 0) {
        struct LineEntry* t = q->top;
        q->top = t->next;
        q->length--;
        return t;
    }
    return NULL;
}

struct LinesStack* initStack() {
    struct LinesStack* s = (struct LinesStack*) malloc(sizeof(struct LinesStack));
    if (s == NULL)
        return NULL;
    s->length = 0;
    s->top = NULL;
    return s;
}

void destroyStack(struct LinesStack* stack) {
    struct LineEntry* e;
    while ((e = pop(stack)) != NULL)
    {
        free(e->line);
        free(e);
    }
    free(stack);
    
}

void printLineEntry(struct LineEntry* e) {
    if (e != NULL && e->line != NULL) {
        printf("%s", e->line);
    }
}