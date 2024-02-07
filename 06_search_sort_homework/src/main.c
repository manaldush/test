#include <stdio.h>
#include <math.h>

typedef struct TableEntry {
    const char* word;
    int counter; 
} TableEntry;

typedef struct Table {
    TableEntry* entries;
    int length;
} Table;

void insertWord(Table* table, const char* word);

void rebalanceTable(Table* table);

long long hashFunction(const char* word);

int main(int argc, char **argv) {

    return 0;
}

long long hashFunction(const char* word) {
    int p = 31;
    long long hash = 0;
    for (int i=0; *(word) != '\0'; word++) 
    {
        hash += ((*word) - 'a' + 1) * pow(p, i);
    }

    return hash;
}

char lowerCase(char ch) {
    if (ch >= 65 && ch <= 90) {
        ch+=32;
    }
    return ch;
}

void insertWord(Table* table, const char* word) {
    int k = 1;
    long long hash = hashFunction(word);
    for(int i = 0; i < (*table).length; i++) {
        int index = (hash + i * k) % (*table).length;
        TableEntry e = (*table).entries[index];
        if (e.counter == 0) {
            e.word = word;
            e.counter++;
            return;
        }
    }
    //TODO
    rebalanceTable(table);
}