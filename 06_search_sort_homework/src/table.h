#pragma once

#include <stdbool.h>

typedef struct TableEntry {
    const char* word;
    int counter;
    long long hash;
} TableEntry;

typedef struct Table {
    TableEntry** entries;
    int length;
} Table;

void freeTable(Table* table);

Table* callocTable();

bool insertTableWord(Table* table, const char* word);

void printTable(Table* table);