#pragma once
#include <stdio.h>

FILE* openFile(const char* fileName);

void closeFile(FILE* file);

char* readLine(FILE* file, char* line, int size);