#include <string.h>
#include "table.h"
#include <stdlib.h>
#include <stdio.h>

static int DEFAULT_K = 1;
static const int INIT_TABLE_LENGTH = 16;

typedef enum InsertResult {
  SUCCESS,
  MALLOC_ERROR,
  REBALANCE_NEED
} InsertResult;

TableEntry** getPtr(Table* table, int i) {
    return &((*table).entries[i]);
}

long long hashFunction(const char* word) {
    long long hash = 0;
    for (int p = 1; *(word) != '\0'; word++, p*=31) {
        hash += ((*word) - 'a' + 1) * p;
    }

    return hash;
}

int nextIndex(long long hash, int i, int tableLen) {
    int r = (hash + i * DEFAULT_K) % tableLen;
    return r < 0 ? r * (-1) : r;
}

void freeTableEntry(TableEntry* entry) {
    free((TableEntry*) entry->word);
    free((Table*)entry);
}

void freeTable(Table* table) {
    for(int i = 0; i < table->length; i++) {
        TableEntry** e = getPtr(table, i);
        if (*e == NULL) {
            //skip
            continue;
        }
        freeTableEntry(*e);
    }
    free(table);
}

TableEntry* callocTableEntry(const char* _word) {
    TableEntry* e = (TableEntry*) calloc(1, sizeof(TableEntry));
    if (e == NULL) {
        return NULL;
    }
    int len = strlen(_word);
    char* word = malloc(sizeof(char) * (len + 1));
    if (word == NULL) {
        free(e);
        return NULL;
    }
    word = strcpy(word, _word);
    e->word = word;
    return e;
}

Table* callocTable() {
    Table* table = calloc(1, sizeof(Table));
    if (table == NULL) {
        return NULL;
    }

    table->entries = (TableEntry**)calloc(INIT_TABLE_LENGTH, sizeof(TableEntry*));
    if (table->entries == NULL) {
        free(table);
        return NULL;
    }

    table->length = INIT_TABLE_LENGTH;
    return table;
}

bool rebalanceTable(Table* table) {
    int nLength = table->length * 2;
    TableEntry** entries = calloc(nLength, sizeof(TableEntry*));
    if(entries == NULL) {
        return false;
    }
    int tableLen = (*table).length;
    for(int i = 0; i < tableLen; i++) {
        TableEntry** e = getPtr(table, i);
        if (*e == NULL) {
            //skip
            continue;
        }
        bool inserted = false;
        for(int ii = 0; ii < nLength && !inserted; ii++) {
            int index = nextIndex((*e)->hash, ii, nLength);
            TableEntry** ee = &(entries[index]);
            if (*ee == NULL) {
                *ee = *e;
                inserted = true;
            }
        }

        if (!inserted) {
            free(entries);
            return false;
        }
    }

    table->entries = entries;
    table->length = nLength;
    return true;
}

InsertResult insertTableEntry(Table* table, const char* word, long long hash) {
    int tableLen = (*table).length;
    for(int i = 0; i < tableLen * 0.75; i++) {
        int index = nextIndex(hash, i, tableLen);
        TableEntry** e = getPtr(table, index);
        if (*e == NULL) {
            *e = callocTableEntry(word);
            if (e == NULL) {
                return MALLOC_ERROR;
            }
            (*e)->counter++;
            (*e)->hash = hash;
            return SUCCESS;
        } else if (strcmp(word, (*e)->word) == 0) {
            (*e)->counter++;
            return SUCCESS;
        }
    }
    return REBALANCE_NEED;
}

bool insertTableWord(Table* table, const char* word) {
    long long hash = hashFunction(word);
    switch (insertTableEntry(table, word, hash)) {
        case SUCCESS:
            return true;
        case REBALANCE_NEED:
            return (rebalanceTable(table) && insertTableEntry(table, word, hash) == SUCCESS);
        case MALLOC_ERROR:
            return false;
        default:
            return false;
    }
}

void printTable(Table* table) {
    int tableLen = (*table).length;
    printf("Words statistics in file\n");
    for(int i = 0; i < tableLen; i++) {
        TableEntry** e = getPtr(table, i);
        if (*e != NULL) {
            printf("Word = %s, count = %d\n", (*e)->word, (*e)->counter);
        }
    }
}