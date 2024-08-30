#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "crc32.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//4mb
#define PAGE_SIZE 4194304
#define handle_error(msg) \
    do { perror(msg); } while (0)

int main(int argc, char** argv) {

    int result = EXIT_SUCCESS;
    struct stat  sb;
    if (argc < 2) {
       fprintf(stderr, "Input parameter file name is missed\n");
       result = EXIT_FAILURE;
       goto exitPoint;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("File opening");
        goto exitPoint;
    }

    if (fstat(fd, &sb) == -1) {
        fprintf(stderr, "fstat error\n");
        result = EXIT_FAILURE;
        goto closeFilePoint;
    }
    
    off_t pageSize = sysconf(_SC_PAGE_SIZE);
    while(pageSize < PAGE_SIZE) {
        pageSize = pageSize << 1;
    }
    printf("Page size %ld\n", (long)pageSize);
    void* addr = NULL;

    uint32_t crcValue = 0;

    for(off_t counter = 0; counter < sb.st_size; counter+=pageSize) {
        size_t length = pageSize;
        if ((sb.st_size - counter) < pageSize) {
            length = sb.st_size - counter;
        }
        addr = mmap(addr, length, PROT_READ, MAP_PRIVATE, fd, counter);
        if (addr == MAP_FAILED) {
            handle_error("mmap");
            result = EXIT_FAILURE;
            goto closeFilePoint;
        }
        crcValue = crc32c(crcValue, addr, length);
    }

    printf("Result value is %lu\n", (unsigned long)crcValue);


closeFilePoint:
    close(fd);
exitPoint:
    fprintf(stdout, "Execution stopped\n");
    return result;
}