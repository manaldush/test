#pragma once

typedef enum LoggerLevel {
    UNKNOWN = 0,
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR
} LoggerLevel;

struct Logger;

typedef struct Logger Logger;

void trace(Logger* logger, char* s);

void debug(Logger* logger, char* s);

void info(Logger* logger, char* s);

void warn(Logger* logger, char* s);

void error(Logger* logger, char* s);

Logger* initLogger(char* path, LoggerLevel level);

void closeLogger(Logger* logger);

LoggerLevel str2LoggerLevel(char* s);