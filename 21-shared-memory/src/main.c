#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>


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
        goto exitPoint;
    }

    if (fstat(fd, &sb) == -1) {
        fprintf(stderr, "fstat error\n");
        result = EXIT_FAILURE;
        goto closeFilePoint;
    }
    
    off_t pageSize = sysconf(_SC_PAGE_SIZE);

//    addr = mmap(NULL, length + offset - pa_offset, PROT_READ, MAP_PRIVATE, fd, pa_offset);
//    if (addr == MAP_FAILED)
//        handle_error("mmap");

closeFilePoint:
    close(fd);
exitPoint:
    fprintf(stdout, "Execution stopped\n");
    return result;
}