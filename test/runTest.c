#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "emu.h"

#define N_BYTES 64
#define MAX_LINE_SIZE 128

typedef struct
{
    uint8_t registerArray[NUM_OF_REGISTERS];
} Params_t;

bool verify(const Params_t* const param_in,
            const Params_t* const param_out);
bool parseTest(int fd,
               const Params_t* const param_in,
               const Params_t* const param_out);

ssize_t readLineFromFd(int fd, char line[MAX_LINE_SIZE],
                       int* totalOffset, bool* isOk);
int main(int argc, char** argv)
{
    Params_t paramsOut;
    Params_t paramsIn;

    if (argc < 2)
    {
        fprintf(stderr, "%s\n", "Provide a filename...");
        fprintf(stdout, "Test FAILED.\n");
        exit(-1);
    }
    if (strlen(argv[0]) == 0)
    {
        fprintf(stderr, "%s\n", "Provide a non-empty filename...");
        fprintf(stdout, "Test FAILED.\n");
        exit(-1);
    }

    const char* fileName = argv[1];
    fprintf(stdout, "%s!\n", fileName);

    int fd = open(fileName, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "%s\n", "Couldn't open the file...");
        fprintf(stdout, "Test FAILED.\n");
        exit(-1);
    }

    bool testValid = parseTest(fd, &paramsIn, &paramsOut);
    if (!testValid)
    {
        fprintf(stderr, "Found error in .yaml %s\n", fileName);
        fprintf(stdout, "Test FAILED.\n");
        exit(-1);
    }
    if (verify(&paramsIn, &paramsOut))
    {
        fprintf(stdout, "Test PASSED.\n");
    } else
    {
        fprintf(stdout, "Test FAILED.\n");
    }
    return 0;
}

bool parseTest(int fd,
               const Params_t* const param_in,
               const Params_t* const param_out)
{
    bool isOk = true;
    char line[MAX_LINE_SIZE];
    line[MAX_LINE_SIZE - 1] = '\0';
    ssize_t bytes_read = 0;
    int totalOffset = 0;
    while ((bytes_read = readLineFromFd(fd, line, &totalOffset, &isOk)) > 0)
    {
        if (strnlen(line, MAX_LINE_SIZE) > 0)
        {
            fprintf(stdout, "Line: %s\n", line);
            memset(line, 0, MAX_LINE_SIZE);
        }
        memset(line, 0, MAX_LINE_SIZE);
    }
    return isOk;
}

bool verify(const Params_t* const param_in,
            const Params_t* const param_out)
{
    bool isOk = true;
    return isOk;
}

ssize_t readLineFromFd(int fd, char line[MAX_LINE_SIZE],
                       int* totalOffset, bool* isOk)
{
    *isOk = true;
    char buff[MAX_LINE_SIZE];
    ssize_t bytes_read = read(fd, buff, MAX_LINE_SIZE - 2);
    if (bytes_read <= 0)
    {
        printf("br%ld\n", bytes_read);
        return bytes_read;
    }

    int i = 0;
    char* c;
    for (c = buff;
         *c != '\n' && *c != 0 && i <= MAX_LINE_SIZE - 2;
         c++, i++);
    if (i >= MAX_LINE_SIZE - 2)
    {
        printf("s:%s\n", buff);
        fprintf(stderr, "Line too long! %d\nMAX: %d\n", i, MAX_LINE_SIZE);
        *isOk = false;
        return 0;
    }
    if (buff[i] == '\n')
    {
        memcpy(line, buff, i);
        line[i] = '\0';
        // printf("totalOffset: %d\n", *totalOffset);
        // printf("i: %d\n", i);
        lseek(fd, *(totalOffset) + i, SEEK_SET);
    }
    *totalOffset += i+1;
    return bytes_read;

    // Now - if we read past the new line, we have to reset
    // the position TO the bytes read

    // Corner cases:
    // 1. We read the entire file on this call
}
