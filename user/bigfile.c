#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define BSIZE 1024  // block size

int buf[BSIZE];

int
main(int argc, char *argv[])
{
    int file_size;
    char* name;
    if (argc == 3) {
        file_size = atoi(argv[1]);
        name = argv[2];
    } else if (argc == 4 && strcmp(argv[1], "-B") == 0) {
        file_size = atoi(argv[2]) * BSIZE;
        name = argv[3];
    } else {
        printf("error: Usage: bigfile [-B] <size> <name>\n");
        printf("\t<size> is size of the file in bytes (or in disk blocks if -B set)\n");
        printf("\t<name> is name of the file\n");
        exit(1);
    }

    int fd = open(name, O_CREATE | O_WRONLY);
    int actual_size = 0;
    for (; actual_size + BSIZE < file_size; actual_size += BSIZE) {
        write(fd, buf, BSIZE);
    }
    write(fd, buf, file_size - actual_size);

    return 0;
}
