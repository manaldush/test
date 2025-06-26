#pragma once
#include <stdint.h>

struct RefererEntry;
struct Referers;

struct Referers* initReferers();
void putReferer(struct Referers* referers, const char* referer, uint64_t c);
void mergeReferers(struct Referers* destination, struct Referers* referers);
void destroyReferers(struct Referers* referers);
void printTopReferers(struct Referers* referers);
int getReferersSize(struct Referers* referers);