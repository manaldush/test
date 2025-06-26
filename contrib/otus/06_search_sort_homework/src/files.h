#pragma once
#include <stdio.h>

typedef enum ReadResult {
    SUCCESS,
    FAILURE,
    END_OF_FILE,
    WORD_LIMIT_REACHED,
} ReadResult;

ReadResult readNextWord(FILE* file, char* word, int maxLength);

FILE* openFile(const char* fileName);