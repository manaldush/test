#pragma once

#include <stdbool.h>

struct LineEntry;
struct LinesStack;

bool push(struct LinesStack* q, char* l);

struct LineEntry* pop(struct LinesStack* q);

struct LinesStack* initStack();

void destroyStack(struct LinesStack* stack);

void destroyLine(struct LineEntry* e);

void printLineEntry(struct LineEntry* e);

char* getLine(struct LineEntry* entry);