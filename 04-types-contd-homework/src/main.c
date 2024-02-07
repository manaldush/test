#include <stdio.h>
#include "koi8r.h"
#include "windows1251.h"
#include "iso88595.h"
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>


FILE* getFile(const char* fileName, const char* format);

bool convertToUTF8Wrapper(FILE* inputFile, const char* outputFileName, const uint32_t* mapping);

bool convertToUTF8(FILE* input, FILE* output, const uint32_t* mapping);

bool writeByte(FILE* outputFile, uint8_t b) ;

int main(int argc, char **argv) {

    if (argc == 1) {
       fprintf(stderr, "Input parameters are missed\n");
       return EXIT_FAILURE;
    } else if (argc == 2) {
       fprintf(stderr, "Input parameters are missed: encoding and output file\n");
       return EXIT_FAILURE;
    } else if (argc == 3) {
       fprintf(stderr, "Input parameters are missed: output file\n");
       return EXIT_FAILURE;
    }

    const char* inputFile = argv[1];
    const char* encoding = argv[2];
    const char* outputFile = argv[3];

    const uint32_t* mapping;

    if (strcmp(encoding, "koi8") == 0) {
        mapping = &koi8[0];
    } else if (strcmp(encoding, "windows1251") == 0) {
        mapping = &windows1251[0];
    } else if (strcmp(encoding, "iso8859") == 0) {
        mapping = &iso88595[0];
    } else {    
        return EXIT_FAILURE;
    }

    FILE* inputFilePtr = getFile(inputFile, "r");
    if (inputFilePtr == NULL) {
        exit(EXIT_FAILURE);
    }

    int res = (convertToUTF8Wrapper(inputFilePtr, outputFile, mapping)) ? EXIT_SUCCESS : EXIT_FAILURE;
    fclose(inputFilePtr);

    return res;
}

FILE* getFile(const char* fileName, const char* format) {
   FILE* file = fopen(fileName, format);
   if (file == NULL) {
      printf("File name %s wasn't opened, the error message is: %s\n", fileName, strerror(errno)); 
      return NULL;
   }

   return file;
}

bool convertToUTF8Wrapper(FILE* inputFile, const char* outputFileName, const uint32_t* mapping) {
    FILE* outputFile = getFile(outputFileName, "w");
    if (outputFile == NULL) {
        return false;
    }

    bool res = convertToUTF8(inputFile, outputFile, mapping);
    fclose(outputFile);

    return res;
}

bool writeByte(FILE* outputFile, uint8_t b) {
    if (fwrite(&b, sizeof b, 1, outputFile) != 1) {
        printf("Error writing file\n");
        fclose(outputFile);
        return false;
    };

    return true;
}

bool convertToUTF8(FILE* input, FILE* output, const uint32_t* mapping) {
    uint8_t bVal;
    uint8_t* b = &bVal;
    for (size_t retCode = fread(b, sizeof *b, 1, input); retCode == sizeof *b;retCode = fread(b, sizeof *b, 1, input)) {
        if (*b <= 127) {
            if (!writeByte(output, *b))
                return false;
        } else {
            uint32_t c = *(mapping + (*b - 128));
            uint8_t cb = (c & 0x00ff0000) >> 16;
            if (cb != 0 && !writeByte(output, cb)) {
                return false;
            }
            cb = (c & 0x0000ff00) >> 8;
            if (cb != 0 && !writeByte(output, cb)) {
                return false;
            }
            cb = (c & 0x000000ff);
            if (!writeByte(output, cb)) {
                return false;
            }
        }
    };

    if (feof(input)) {
        return true;
    } else if (ferror(input)) {
        perror("Error reading input file");
        return false;
    }

    return true;
}