#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

const int64_t data[] = {4, 8, 15, 16, 23, 42};
const int dataLength = 6;
const char* emptyStr = "";
const char* format = "%ld ";

struct element {
    int64_t value;
    struct element* next;
};

int64_t p(int64_t v) {
    return v & 1;
}

void print_int(int64_t value) {
    printf(format, value);
}

struct element* addElement(int64_t value, struct element* next) {
    struct element* cursor = malloc(sizeof(struct element));
    if(cursor == NULL) {
        exit(EXIT_FAILURE);
    }
    cursor->value = value;
    cursor->next = next;
    return cursor;
}

void m(struct element* cursor) {
    if (cursor == NULL) 
        return;
    print_int(cursor->value);
    m(cursor->next);
}

struct element* f(struct element* cursor, struct element* next) {
    if (cursor == NULL) 
        return next;
    if (p(cursor->value) != 0) {
        next = addElement(cursor->value, next);
    }
    return f(cursor->next, next);
}

void freeList(struct element* cursor) {
    while (cursor != NULL)
    {
        struct element* t = cursor->next;
        free(cursor);
        cursor = t;
    }
    
}

int main() {
    struct element* next = NULL;
    for(int counter = dataLength - 1; counter >= 0;counter--) {
        next = addElement(data[counter], next);
    }

    struct element* list1 = next;
    m(list1);
    puts(emptyStr);

    struct element* list2 = f(list1, NULL);
    m(list2);
    puts(emptyStr);

    freeList(list1);
    freeList(list2);
}