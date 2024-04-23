#include <stdint.h>

struct RefererEntry;
struct Referers;

struct Urls;
struct UrlEntry;

uint64_t putReferer(struct Referers* referers, const char* referer);
void putUrl(struct Urls* urls, const char* entry);