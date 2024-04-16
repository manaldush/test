#pragma once
#include <stdio.h>

FILE* openFile(const char* fileName);

void closeFile(FILE* file);

char* readLine(FILE* file, const char* line, int size);