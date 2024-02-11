#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "files.h"

int main(int argc, char **argv) {
    
    if (argc < 2) {
       fprintf(stderr, "Input parameters are missed\n");
       return EXIT_FAILURE;
    }

    const char* inputFile = argv[1];
    FILE* f = openFile(inputFile);
    if (f == NULL) {
        return EXIT_FAILURE;
    }

    Table* table = callocTable();
    if (table == NULL) {
        fclose(f);
        return EXIT_FAILURE;
    }

    char word[65];
    ReadResult readResult;
    int res = EXIT_SUCCESS;
    while (((readResult = readNextWord(f, word, 64)) == SUCCESS || readResult == END_OF_FILE)) {
        if (*word != '\0' && !insertTableWord(table, word)) {
            fprintf(stderr, "Word %s can't be inserted in table\n", word);
            res = EXIT_FAILURE;
            break;
        }
        if (readResult == END_OF_FILE) {
            printf("End of file is reached\n");
            break;
        }
    }

    if (readResult == END_OF_FILE && res == EXIT_SUCCESS) {
        printTable(table);
    } else {
        fprintf(stderr, "Table can't be printed\n");
        res = EXIT_FAILURE;
    }

    fclose(f);
    freeTable(table);

    return res;
}