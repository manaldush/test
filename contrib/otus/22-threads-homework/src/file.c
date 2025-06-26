#include "file.h"
#include <errno.h>
#include <string.h>

FILE* openFile(const char* fileName) {
   FILE* file = fopen(fileName, "r");
   if (file == NULL) {
      printf("File name %s wasn't opened, the error message is: %s\n", fileName, strerror(errno)); 
      return NULL;
   }

   return file;
}

void closeFile(FILE* file) {
    fclose(file);
}

char* readLine(FILE* file, char* line, int size) {
    return fgets(line, size, file);
}