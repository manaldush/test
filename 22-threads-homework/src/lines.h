#pragma once

struct LogLine;

struct LogLine* createLogLine(char* l);

void destroyLogLine(struct LogLine* line);