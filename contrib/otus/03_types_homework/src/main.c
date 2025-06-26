#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

// EOCDR static part size
const long EoCDR_STATIC_PART_SIZE = 22;

// max EOCDR comment size static part size
const long EoCDR_MAX_COMMENT_LENGTH = 65536;

char SIGNATURE_EoCDR[] = {0x50, 0x4b, 0x05, 0x06};

char SIGNATURE_CDR[] = {0x50, 0x4b, 0x01, 0x02};

FILE* getFile(const char* fileName);

long isRarJpeg(FILE* file);

uint32_t getCDRsize(FILE *file, long eocdrPointer);

uint16_t getTotalEntries(FILE *file, long eocdrPointer);

long printCDR(FILE* file, long position);

uint16_t read16(FILE* file);

int main(int argc, char **argv) {
   if (argc == 1) {
      fprintf(stderr, "Input parameter missed\n");
      return -1;
   }
   const char* fileName = argv[1];
   printf("Input filename is: %s\n", fileName);

   // get file pointer
   FILE* file = getFile(fileName);
   
   long eocdrPosition = isRarJpeg(file);
   if (!eocdrPosition) {
      printf("Input filename %s isn't rar jpeg\n", fileName);
      fclose(file);
      return(0);
   }

   uint32_t cdrSize = getCDRsize(file, eocdrPosition);
   uint16_t totalEntries = getTotalEntries(file, eocdrPosition);

   printf("Input filename %s is rar jpeg, total entries = %i\n", fileName, totalEntries);

   long nextCDRPosition = eocdrPosition - 4 - cdrSize;
   for(int c = 0; c < totalEntries; c++) {
      nextCDRPosition = printCDR(file, nextCDRPosition);
   }

   fclose(file);

   return 0;
}

FILE* getFile(const char* fileName) {
   FILE* file = fopen(fileName, "r");
   if (file == NULL) {
      printf("File name %s wasn't found\n", fileName);
      exit(1);
   }

   printf("File name %s was found\n", fileName);

   return file;
}

long isRarJpeg(FILE* file) {
   // check existence EOCDR block
   fseek(file, 0, SEEK_END);
   long fileLength = ftell(file) - 1;
   if (fileLength - EoCDR_STATIC_PART_SIZE < 0) {
      return 0;
   }
   long int endEoCdrSearchSigPointer = fileLength - EoCDR_STATIC_PART_SIZE + sizeof(SIGNATURE_EoCDR);
   long int startEoCdrSearchSigPointer = fileLength - EoCDR_STATIC_PART_SIZE - EoCDR_MAX_COMMENT_LENGTH + 1;
   startEoCdrSearchSigPointer = (startEoCdrSearchSigPointer < 0) ? 0 : startEoCdrSearchSigPointer;

   fseek(file, startEoCdrSearchSigPointer, SEEK_SET);
   int sigPosition = 0;
   for (long pointer = startEoCdrSearchSigPointer; pointer <= endEoCdrSearchSigPointer; pointer++ )
   {
      char b = fgetc(file);
      if (SIGNATURE_EoCDR[sigPosition] == b) {
         if ((sigPosition + 1) == sizeof(SIGNATURE_EoCDR)) {
            //rar jpeg
            return (pointer+1);
         } else {
            sigPosition++;
         }
      } else {
         sigPosition = 0;
      }
   }

   return 0;
}

uint32_t getCDRsize(FILE *file, long eocdrPointer) {
   long cdrSizeFieldPointer = eocdrPointer + 8;
   fseek(file, cdrSizeFieldPointer, SEEK_SET);

   uint32_t res = 0x0;
   for (int c = 0; c < 4; c++) {
      res = res << 8;
      char b = fgetc(file);
      res = res | b;
   }
   return ntohl(res);
}

uint16_t getTotalEntries(FILE *file, long eocdrPointer) {
   long cdrSizeFieldPointer = eocdrPointer + 6;
   fseek(file, cdrSizeFieldPointer, SEEK_SET);

   return read16(file);
}

long printCDR(FILE* file, long position) {
   fseek(file, position, SEEK_SET);
   for(unsigned c = 0; c < sizeof(SIGNATURE_CDR); c++) {
      char b = fgetc(file);
      if (b != SIGNATURE_CDR[c]) {
         printf("%i\n", b);
         printf("Zip format error: SIGNATURE_CDR wasn't found for position %li", position);
         exit(1);
      }
   }
   fseek(file, position + 28, SEEK_SET);
   uint16_t fileNameLen = read16(file);
   uint16_t extraFieldLen = read16(file);
   uint16_t fileCommentLen = read16(file);
   fseek(file, position + 46, SEEK_SET);
   if (fileNameLen == 0) {
      printf("Zip format error: zero file name field");
      exit(1);
   }

   char fileName[fileNameLen + 1];
   for(int c = 0; c < fileNameLen; c++) {
      fileName[c] = fgetc(file);
   }
   fileName[fileNameLen] = '\0';

   printf("%s\n", fileName);

   return position + 46 + fileNameLen + extraFieldLen + fileCommentLen;
}

uint16_t read16(FILE* file) {
   uint16_t res = 0x0;
   for (int c = 0; c < 2; c++) {
      res = res << 8;
      char b = fgetc(file);
      res = res | b;
   }
   return ntohs(res);
}