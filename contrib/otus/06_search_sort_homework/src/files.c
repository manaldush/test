#include <stdio.h>
#include <errno.h>
#include "files.h"
#include <stdbool.h>
#include <string.h>

static char LOWER_CASE_LOW_BORDER = 97;
static char LOWER_CASE_HIGH_BORDER = 122;
static char UPPER_CASE_LOW_BORDER = 65;
static char UPPER_CASE_HIGH_BORDER = 90;
static char SHIFT = 32;

bool isLowerCaseChar(const char* cptr) {
    char c = *cptr;
    return (c >= LOWER_CASE_LOW_BORDER && c <= LOWER_CASE_HIGH_BORDER) ? true : false;
}

bool isUpperCaseChar(const char* cptr) {
    char c = *cptr;
    return (c >= UPPER_CASE_LOW_BORDER && c <= UPPER_CASE_HIGH_BORDER) ? true : false;
}

bool isChar(const char* cptr) {
    return isLowerCaseChar(cptr) || isUpperCaseChar(cptr) ? true : false;
}

FILE* openFile(const char* fileName) {
   FILE* file = fopen(fileName, "r");
   if (file == NULL) {
      printf("File name %s wasn't opened, the error message is: %s\n", fileName, strerror(errno)); 
      return NULL;
   }

   return file;
}

char lowerCase(char ch) {
    if (ch >= UPPER_CASE_LOW_BORDER && ch <= UPPER_CASE_HIGH_BORDER) {
        ch+=SHIFT;
    }
    return ch;
}

ReadResult readNextWord(FILE* file, char* word, int maxLength) {
    for(int wi = 0; wi < maxLength;) {
        char ch = fgetc (file);
        if (ch == EOF && !feof(file)) {
            printf("Read file error occurred: %s\n", strerror(errno)); 
            return FAILURE;
        } else if (ch == EOF) {
            *(word + wi) = '\0';
            return END_OF_FILE;
        }
        if (isChar(&ch)) {
            *(word + wi++) = lowerCase(ch);
        } else if (wi > 0) {
            *(word + wi) = '\0';
            return SUCCESS;
        } else {
            //word not found yet - continue
        }
    }
    *(word + maxLength) = '\0';
    return WORD_LIMIT_REACHED;
}