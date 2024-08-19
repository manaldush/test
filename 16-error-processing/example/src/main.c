#include <stdio.h>
#include <stdlib.h>
#include "logger.h"


int main(int argc, char** argv) {
    if (argc < 3) {
       fprintf(stderr, "Input parameters are missed\n");
       return EXIT_FAILURE;
    }

   char* path = argv[1];
   LoggerLevel loggerLevel = str2LoggerLevel(argv[2]);
   if (loggerLevel == UNKNOWN) {
       fprintf(stderr, "Input parameter log level has illegal value\n");
       return EXIT_FAILURE;
   }
   Logger* logger = initLogger(path, loggerLevel);
   if (logger == NULL) {
       fprintf(stderr, "Logger wasn't initialized correctly\n");
       return EXIT_FAILURE;
    }

    trace(logger, "trace message");

    warn(logger, "warn message");

    closeLogger(logger);
}