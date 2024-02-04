#include <stdio.h>
#include "koi8r.h"
#include "windows1251.h"
#include "iso88595.h"
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

FILE* getFile(const char* fileName, const char* format);

int convertKoi8ToUTF8(FILE* file, const char* outputFile);

int convertWindows1251ToUTF8(FILE* file, const char* outputFile);

int convertISO88595ToUTF8(FILE* file, const char* outputFile);

int convertToUTF8Wrapper(FILE* inputFile, const char* outputFileName, const uint32_t* mapping);

int convertToUTF8(FILE* input, FILE* output, const uint32_t* mapping);

int writeByte(FILE* outputFile, uint8_t b) ;

int main(int argc, char **argv) {

    if (argc == 1) {
       fprintf(stderr, "Input parameters are missed\n");
       return -1;
    } else if (argc == 2) {
       fprintf(stderr, "Input parameters are missed: encoding and output file\n");
       return -1;
    } else if (argc == 3) {
       fprintf(stderr, "Input parameters are missed: output file\n");
       return -1;
    }

    const char* inputFile = argv[1];
    const char* encoding = argv[2];
    const char* outputFile = argv[3];

    FILE* inputFilePtr = getFile(inputFile, "r");
    if (inputFile == NULL) {
        exit(1);
    }

    if (strcmp(encoding, "koi8") == 0) {
        if (convertKoi8ToUTF8(inputFilePtr, outputFile) == 0) {
            fclose(inputFilePtr);
        }
    } else if (strcmp(encoding, "windows1251") == 0) {
        if (convertWindows1251ToUTF8(inputFilePtr, outputFile) == 0) {
            fclose(inputFilePtr);
        }
    } else if (strcmp(encoding, "iso8859") == 0) {
        if (convertISO88595ToUTF8(inputFilePtr, outputFile) == 0) {
            fclose(inputFilePtr);
        }
    } else {
        fclose(inputFilePtr);
    
        return -1;
    }



    return 0;
}

FILE* getFile(const char* fileName, const char* format) {
   FILE* file = fopen(fileName, format);
   if (file == NULL) {
      printf("File name %s wasn't found\n", fileName);
      return NULL;
   }

   return file;
}

int convertToUTF8Wrapper(FILE* inputFile, const char* outputFileName, const uint32_t* mapping) {
    FILE* outputFile = getFile(outputFileName, "w");
    if (outputFile == NULL) {
        return -1;
    }
    if (!convertToUTF8(inputFile, outputFile, mapping)) {
        return 0;
    } else {
        fclose(outputFile);
        return -1;
    }
}

int convertISO88595ToUTF8(FILE* file, const char* outputFile) {
    return convertToUTF8Wrapper(file, outputFile, &iso88595[0]);
}

int convertKoi8ToUTF8(FILE* inputFile, const char* outputFileName) {
    return convertToUTF8Wrapper(inputFile, outputFileName, &koi8[0]);
}

int convertWindows1251ToUTF8(FILE* inputFile, const char* outputFileName) {
    return convertToUTF8Wrapper(inputFile, outputFileName, &windows1251[0]);
}

int writeByte(FILE* outputFile, uint8_t b) {
    if (fwrite(&b, sizeof b, 1, outputFile) != 1) {
        printf("Error writing file\n");
        fclose(outputFile);
        return -1;
    };

    return 0;
}

int convertToUTF8(FILE* input, FILE* output, const uint32_t* mapping) {
    uint8_t bVal;
    uint8_t* b = &bVal;
    for (size_t retCode = fread(b, sizeof *b, 1, input); retCode == sizeof *b;retCode = fread(b, sizeof *b, 1, input)) {
        if (*b <= 127) {
            if (writeByte(output, *b) != 0)
                return -1;
        } else {
            uint32_t c = *(mapping + (*b - 128));
            uint8_t cb = (c & 0x00ff0000) >> 16;
            if (cb != 0 && writeByte(output, cb) != 0) {
                return -1;
            }
            cb = (c & 0x0000ff00) >> 8;
            if (cb != 0 && writeByte(output, cb) != 0) {
                return -1;
            }
            cb = (c & 0x000000ff);
            if (writeByte(output, cb) != 0) {
                return -1;
            }
        }
    };

    if (feof(input)) {
        return 0;
    } else if (ferror(input)) {
        perror("Error reading input file");
        return -1;
    }

    return 0;
}