#pragma once

struct LogLine;

struct LogLine* createLogLine(char* l);

void destroyLogLine(struct LogLine* line);

void printLogLine(struct LogLine* line);

char* getReferer(struct LogLine* line);