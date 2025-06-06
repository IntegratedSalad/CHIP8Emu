#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>

void usage(void);
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stdout, "No arguments provided\n");
        usage();
        return -1;
    }

    const char* fileName = argv[1]; // for now, default to SDL2
    int fd = open(fileName, O_RDWR);

    if (fd < 0)
    {
        fprintf(stderr, "Error while opening file %s: %s\n", fileName, strerror(errno));
        return -1;
    }

    if (read(fd, NULL, 1) == 0)
    {
        fprintf(stderr, "File %s empty!\n", fileName);
        return -1;
    }
    lseek(fd, 0, SEEK_SET);

    fprintf(stdout, "Opened file %s.\n", fileName);


    return 0;
}

void usage(void)
{
    printf("%s", "Usage: ./emulator [chip8 program]\n");
}